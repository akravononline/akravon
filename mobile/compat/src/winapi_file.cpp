// Cynex: kernel32 dosya fonksiyonlari - posix uzerinde, windows yollari harf duyarsiz cozulur
#include "cynex_handle.h"
#include <io.h>
#include <fcntl.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mutex>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>

namespace cynex
{
	struct SFile : SHandle
	{
		SFile() : SHandle(HANDLE_FILE) {}
		~SFile() override { if (fd >= 0) close(fd); }
		int fd = -1;
		std::string path;
	};

	struct SMapping : SHandle
	{
		SMapping() : SHandle(HANDLE_MAPPING) {}
		~SMapping() override { if (fd >= 0) close(fd); }
		int fd = -1;
		bool writable = false;
		uint64_t size = 0;
	};

	struct SFind : SHandle
	{
		SFind() : SHandle(HANDLE_FIND) {}
		~SFind() override { if (dir) closedir(dir); }
		DIR* dir = nullptr;
		std::string dirPath;
		std::string pattern;
	};

	std::string NormalizeSlashes(const char* path)
	{
		std::string s = path ? path : "";
		std::replace(s.begin(), s.end(), '\\', '/');
		// Cynex: "C:/..." gibi surucu harfi mobilde anlamsiz - goreli kabul et
		if (s.size() >= 2 && s[1] == ':' && isalpha((unsigned char)s[0]))
			s = s.substr(s.size() >= 3 && s[2] == '/' ? 3 : 2);
		std::string out;
		out.reserve(s.size());
		for (char c : s)
		{
			if (c == '/' && !out.empty() && out.back() == '/')
				continue;
			out += c;
		}
		return out;
	}

	namespace
	{
		std::mutex s_pathMutex;
		std::unordered_map<std::string, std::string> s_pathCache;

		bool Exists(const std::string& p)
		{
			struct stat st;
			return stat(p.c_str(), &st) == 0;
		}

		// Cynex: tek bilesen icin klasorde harf duyarsiz eslesme
		bool FindCaseInsensitive(const std::string& dir, const std::string& name, std::string& found)
		{
			DIR* d = opendir(dir.empty() ? "." : dir.c_str());
			if (!d)
				return false;
			bool ok = false;
			while (dirent* e = readdir(d))
			{
				if (strcasecmp(e->d_name, name.c_str()) == 0)
				{
					found = e->d_name;
					ok = true;
					break;
				}
			}
			closedir(d);
			return ok;
		}
	}

	std::string ResolvePath(const char* path, bool forCreate)
	{
		std::string p = NormalizeSlashes(path);
		if (p.empty() || Exists(p))
			return p;

		std::string key = p;
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		{
			std::lock_guard<std::mutex> lock(s_pathMutex);
			auto it = s_pathCache.find(key);
			if (it != s_pathCache.end() && Exists(it->second))
				return it->second;
		}

		std::string cur = (p[0] == '/') ? "/" : "";
		size_t pos = (p[0] == '/') ? 1 : 0;
		bool ok = true;
		while (pos <= p.size())
		{
			size_t next = p.find('/', pos);
			if (next == std::string::npos)
				next = p.size();
			std::string comp = p.substr(pos, next - pos);
			pos = next + 1;
			if (comp.empty() || comp == ".")
				continue;
			std::string cand = cur.empty() ? comp : (cur.back() == '/' ? cur + comp : cur + "/" + comp);
			if (comp != ".." && !Exists(cand))
			{
				std::string found;
				if (FindCaseInsensitive(cur, comp, found))
					cand = cur.empty() ? found : (cur.back() == '/' ? cur + found : cur + "/" + found);
				else
				{
					ok = false;
					// Cynex: olusturulacak dosya - kalan bilesenler oldugu gibi
					cur = cand;
					if (pos <= p.size())
						cur += "/" + p.substr(pos);
					break;
				}
			}
			cur = cand;
		}

		if (ok)
		{
			std::lock_guard<std::mutex> lock(s_pathMutex);
			s_pathCache[key] = cur;
		}
		return (ok || forCreate) ? cur : p;
	}

	void SetLastErrorFromErrno(int e)
	{
		switch (e)
		{
			case ENOENT: SetLastError(ERROR_FILE_NOT_FOUND); break;
			case ENOTDIR: SetLastError(ERROR_PATH_NOT_FOUND); break;
			case EACCES:
			case EPERM: SetLastError(ERROR_ACCESS_DENIED); break;
			case EEXIST: SetLastError(ERROR_ALREADY_EXISTS); break;
			case ENOMEM: SetLastError(ERROR_NOT_ENOUGH_MEMORY); break;
			default: SetLastError((DWORD)e); break;
		}
	}
}

using namespace cynex;

namespace
{
	std::mutex s_viewMutex;
	std::map<uintptr_t, std::pair<void*, size_t>> s_views;	// kullaniciya verilen -> (mmap tabani, uzunluk)

	FILETIME ToFileTime(time_t t)
	{
		// Cynex: 1601'den beri 100ns
		uint64_t v = (uint64_t(t) + 11644473600ULL) * 10000000ULL;
		FILETIME ft;
		ft.dwLowDateTime = (DWORD)v;
		ft.dwHighDateTime = (DWORD)(v >> 32);
		return ft;
	}

	void FillFindData(const std::string& dir, const char* name, LPWIN32_FIND_DATA fd)
	{
		memset(fd, 0, sizeof(*fd));
		strncpy(fd->cFileName, name, MAX_PATH - 1);
		std::string full = dir.empty() ? name : dir + "/" + name;
		struct stat st;
		if (stat(full.c_str(), &st) == 0)
		{
			fd->dwFileAttributes = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
			fd->nFileSizeLow = (DWORD)st.st_size;
			fd->nFileSizeHigh = (DWORD)((uint64_t)st.st_size >> 32);
			fd->ftCreationTime = fd->ftLastAccessTime = fd->ftLastWriteTime = ToFileTime(st.st_mtime);
		}
	}

	bool NextMatch(SFind* f, LPWIN32_FIND_DATA fd)
	{
		while (dirent* e = readdir(f->dir))
		{
			if (fnmatch(f->pattern.c_str(), e->d_name, FNM_CASEFOLD) == 0)
			{
				FillFindData(f->dirPath, e->d_name, fd);
				return true;
			}
		}
		return false;
	}
}

extern "C" {

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES, DWORD dwCreationDisposition, DWORD, HANDLE)
{
	const bool write = (dwDesiredAccess & (GENERIC_WRITE | GENERIC_ALL)) != 0;
	const bool read = (dwDesiredAccess & (GENERIC_READ | GENERIC_ALL)) != 0 || !write;
	int flags = (read && write) ? O_RDWR : (write ? O_WRONLY : O_RDONLY);
	switch (dwCreationDisposition)
	{
		case CREATE_NEW: flags |= O_CREAT | O_EXCL; break;
		case CREATE_ALWAYS: flags |= O_CREAT | O_TRUNC; break;
		case OPEN_ALWAYS: flags |= O_CREAT; break;
		case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
		default: break;
	}

	const std::string path = ResolvePath(lpFileName, (flags & O_CREAT) != 0);
	const int fd = open(path.c_str(), flags | O_CLOEXEC, 0644);
	if (fd < 0)
	{
		SetLastErrorFromErrno(errno);
		return INVALID_HANDLE_VALUE;
	}
	SFile* f = new SFile;
	f->fd = fd;
	f->path = path;
	RegisterHandle(f);
	SetLastError(0);
	return f;
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD n, LPDWORD lpRead, LPOVERLAPPED)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	if (lpRead)
		*lpRead = 0;
	if (!f)
		return FALSE;
	size_t total = 0;
	while (total < n)
	{
		ssize_t r = read(f->fd, (char*)lpBuffer + total, n - total);
		if (r < 0)
		{
			if (errno == EINTR)
				continue;
			SetLastErrorFromErrno(errno);
			return FALSE;
		}
		if (r == 0)
			break;
		total += (size_t)r;
	}
	if (lpRead)
		*lpRead = (DWORD)total;
	return TRUE;
}

BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD n, LPDWORD lpWritten, LPOVERLAPPED)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	if (lpWritten)
		*lpWritten = 0;
	if (!f)
		return FALSE;
	size_t total = 0;
	while (total < n)
	{
		ssize_t r = write(f->fd, (const char*)lpBuffer + total, n - total);
		if (r < 0)
		{
			if (errno == EINTR)
				continue;
			SetLastErrorFromErrno(errno);
			return FALSE;
		}
		total += (size_t)r;
	}
	if (lpWritten)
		*lpWritten = (DWORD)total;
	return TRUE;
}

BOOL FlushFileBuffers(HANDLE hFile)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	return f && fsync(f->fd) == 0;
}

DWORD GetFileSize(HANDLE hFile, LPDWORD lpHigh)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	struct stat st;
	if (!f || fstat(f->fd, &st) != 0)
	{
		if (lpHigh)
			*lpHigh = 0;
		return INVALID_FILE_SIZE;
	}
	if (lpHigh)
		*lpHigh = (DWORD)((uint64_t)st.st_size >> 32);
	return (DWORD)st.st_size;
}

BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize)
{
	DWORD hi = 0;
	DWORD lo = GetFileSize(hFile, &hi);
	if (lo == INVALID_FILE_SIZE && GetLastError() != 0)
		return FALSE;
	lpFileSize->QuadPart = ((LONGLONG)hi << 32) | lo;
	return TRUE;
}

DWORD SetFilePointer(HANDLE hFile, LONG lDistance, PLONG lpHigh, DWORD dwMoveMethod)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	if (!f)
		return INVALID_SET_FILE_POINTER;
	int64_t off = lpHigh ? (((int64_t)*lpHigh << 32) | (uint32_t)lDistance) : (int64_t)lDistance;
	const int whence = dwMoveMethod == FILE_CURRENT ? SEEK_CUR : (dwMoveMethod == FILE_END ? SEEK_END : SEEK_SET);
	off_t r = lseek(f->fd, (off_t)off, whence);
	if (r < 0)
	{
		SetLastErrorFromErrno(errno);
		return INVALID_SET_FILE_POINTER;
	}
	if (lpHigh)
		*lpHigh = (LONG)((uint64_t)r >> 32);
	return (DWORD)r;
}

BOOL SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistance, PLARGE_INTEGER lpNew, DWORD dwMoveMethod)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	if (!f)
		return FALSE;
	const int whence = dwMoveMethod == FILE_CURRENT ? SEEK_CUR : (dwMoveMethod == FILE_END ? SEEK_END : SEEK_SET);
	off_t r = lseek(f->fd, (off_t)liDistance.QuadPart, whence);
	if (r < 0)
		return FALSE;
	if (lpNew)
		lpNew->QuadPart = r;
	return TRUE;
}

BOOL SetEndOfFile(HANDLE hFile)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	if (!f)
		return FALSE;
	off_t cur = lseek(f->fd, 0, SEEK_CUR);
	return ftruncate(f->fd, cur) == 0;
}

BOOL GetFileTime(HANDLE hFile, LPFILETIME c, LPFILETIME a, LPFILETIME w)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	struct stat st;
	if (!f || fstat(f->fd, &st) != 0)
		return FALSE;
	const FILETIME ft = ToFileTime(st.st_mtime);
	if (c) *c = ft;
	if (a) *a = ToFileTime(st.st_atime);
	if (w) *w = ft;
	return TRUE;
}

HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES, DWORD flProtect, DWORD dwMaxHigh, DWORD dwMaxLow, LPCSTR)
{
	SFile* f = (SFile*)GetHandle(hFile, HANDLE_FILE);
	if (!f)
		return NULL;
	SMapping* m = new SMapping;
	m->fd = dup(f->fd);
	m->writable = (flProtect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)) != 0;
	uint64_t size = ((uint64_t)dwMaxHigh << 32) | dwMaxLow;
	if (size == 0)
	{
		struct stat st;
		fstat(f->fd, &st);
		size = (uint64_t)st.st_size;
	}
	m->size = size;
	RegisterHandle(m);
	return m;
}

LPVOID MapViewOfFile(HANDLE hMapping, DWORD dwDesiredAccess, DWORD dwOffsetHigh, DWORD dwOffsetLow, SIZE_T dwBytes)
{
	SMapping* m = (SMapping*)GetHandle(hMapping, HANDLE_MAPPING);
	if (!m)
		return NULL;
	const uint64_t offset = ((uint64_t)dwOffsetHigh << 32) | dwOffsetLow;
	if (offset >= m->size && m->size > 0)
		return NULL;
	size_t len = dwBytes ? dwBytes : (size_t)(m->size - offset);
	if (len == 0)
		len = 1;
	// Cynex: mmap sayfa hizali ofset ister
	const uint64_t page = (uint64_t)sysconf(_SC_PAGESIZE);
	const uint64_t aligned = offset & ~(page - 1);
	const size_t delta = (size_t)(offset - aligned);
	const bool write = (dwDesiredAccess & (FILE_MAP_WRITE | FILE_MAP_ALL_ACCESS)) != 0 && m->writable;
	void* base = mmap(nullptr, len + delta, write ? (PROT_READ | PROT_WRITE) : PROT_READ, write ? MAP_SHARED : MAP_PRIVATE, m->fd, (off_t)aligned);
	if (base == MAP_FAILED)
	{
		SetLastErrorFromErrno(errno);
		return NULL;
	}
	void* user = (char*)base + delta;
	std::lock_guard<std::mutex> lock(s_viewMutex);
	s_views[(uintptr_t)user] = std::make_pair(base, len + delta);
	return user;
}

BOOL UnmapViewOfFile(LPCVOID lpBaseAddress)
{
	std::lock_guard<std::mutex> lock(s_viewMutex);
	auto it = s_views.find((uintptr_t)lpBaseAddress);
	if (it == s_views.end())
		return FALSE;
	munmap(it->second.first, it->second.second);
	s_views.erase(it);
	return TRUE;
}

BOOL DeleteFileA(LPCSTR lpFileName)
{
	const std::string p = ResolvePath(lpFileName);
	if (unlink(p.c_str()) != 0)
	{
		SetLastErrorFromErrno(errno);
		return FALSE;
	}
	return TRUE;
}

BOOL MoveFileExA(LPCSTR lpExisting, LPCSTR lpNew, DWORD dwFlags)
{
	const std::string a = ResolvePath(lpExisting);
	const std::string b = ResolvePath(lpNew, true);
	if (!(dwFlags & MOVEFILE_REPLACE_EXISTING) && access(b.c_str(), F_OK) == 0)
	{
		SetLastError(ERROR_ALREADY_EXISTS);
		return FALSE;
	}
	if (rename(a.c_str(), b.c_str()) != 0)
	{
		SetLastErrorFromErrno(errno);
		return FALSE;
	}
	return TRUE;
}

BOOL MoveFileA(LPCSTR lpExisting, LPCSTR lpNew)
{
	return MoveFileExA(lpExisting, lpNew, 0);
}

BOOL CopyFileA(LPCSTR lpExisting, LPCSTR lpNew, BOOL bFailIfExists)
{
	const std::string a = ResolvePath(lpExisting);
	const std::string b = ResolvePath(lpNew, true);
	if (bFailIfExists && access(b.c_str(), F_OK) == 0)
	{
		SetLastError(ERROR_ALREADY_EXISTS);
		return FALSE;
	}
	int in = open(a.c_str(), O_RDONLY | O_CLOEXEC);
	if (in < 0)
	{
		SetLastErrorFromErrno(errno);
		return FALSE;
	}
	int out = open(b.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (out < 0)
	{
		SetLastErrorFromErrno(errno);
		close(in);
		return FALSE;
	}
	char buf[65536];
	ssize_t r;
	bool ok = true;
	while ((r = read(in, buf, sizeof(buf))) > 0)
	{
		if (write(out, buf, (size_t)r) != r)
		{
			ok = false;
			break;
		}
	}
	close(in);
	close(out);
	return ok && r >= 0;
}

BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES)
{
	const std::string p = ResolvePath(lpPathName, true);
	if (mkdir(p.c_str(), 0755) != 0)
	{
		SetLastErrorFromErrno(errno);
		return FALSE;
	}
	return TRUE;
}

BOOL RemoveDirectoryA(LPCSTR lpPathName)
{
	const std::string p = ResolvePath(lpPathName);
	return rmdir(p.c_str()) == 0;
}

DWORD GetFileAttributesA(LPCSTR lpFileName)
{
	const std::string p = ResolvePath(lpFileName);
	struct stat st;
	if (stat(p.c_str(), &st) != 0)
	{
		SetLastErrorFromErrno(errno);
		return INVALID_FILE_ATTRIBUTES;
	}
	DWORD a = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	if (!(st.st_mode & S_IWUSR))
		a |= FILE_ATTRIBUTE_READONLY;
	return a;
}

BOOL SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes)
{
	const std::string p = ResolvePath(lpFileName);
	struct stat st;
	if (stat(p.c_str(), &st) != 0)
		return FALSE;
	mode_t mode = st.st_mode & 07777;
	mode = (dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? (mode & ~0222) : (mode | S_IWUSR);
	return chmod(p.c_str(), mode) == 0;
}

HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
	std::string spec = NormalizeSlashes(lpFileName);
	size_t slash = spec.rfind('/');
	std::string dir = slash == std::string::npos ? "" : spec.substr(0, slash);
	std::string pat = slash == std::string::npos ? spec : spec.substr(slash + 1);
	if (pat == "*.*")
		pat = "*";
	const std::string realDir = dir.empty() ? "" : ResolvePath(dir.c_str());

	DIR* d = opendir(realDir.empty() ? "." : realDir.c_str());
	if (!d)
	{
		SetLastError(ERROR_PATH_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}
	SFind* f = new SFind;
	f->dir = d;
	f->dirPath = realDir;
	f->pattern = pat;
	RegisterHandle(f);
	if (!NextMatch(f, lpFindFileData))
	{
		ReleaseHandle(f);
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}
	return f;
}

BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData)
{
	SFind* f = (SFind*)GetHandle(hFindFile, HANDLE_FIND);
	if (!f || !NextMatch(f, lpFindFileData))
	{
		SetLastError(ERROR_NO_MORE_FILES);
		return FALSE;
	}
	return TRUE;
}

BOOL FindClose(HANDLE hFindFile)
{
	SFind* f = (SFind*)GetHandle(hFindFile, HANDLE_FIND);
	if (!f)
		return FALSE;
	ReleaseHandle(f);
	return TRUE;
}

DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer)
{
	char buf[1024];
	if (!getcwd(buf, sizeof(buf)))
		return 0;
	const DWORD len = (DWORD)strlen(buf);
	if (!lpBuffer || nBufferLength <= len)
		return len + 1;
	memcpy(lpBuffer, buf, len + 1);
	return len;
}

BOOL SetCurrentDirectoryA(LPCSTR lpPathName)
{
	const std::string p = ResolvePath(lpPathName);
	return chdir(p.c_str()) == 0;
}

DWORD GetModuleFileNameA(HMODULE, LPSTR lpFilename, DWORD nSize)
{
	// Cynex: pc'de exe yolu - mobilde oyun klasoru + metin2client.bin
	char cwd[1024];
	if (!getcwd(cwd, sizeof(cwd)))
		strcpy(cwd, ".");
	std::string p = std::string(cwd) + "/metin2client.bin";
	if (!lpFilename || nSize == 0)
		return 0;
	strncpy(lpFilename, p.c_str(), nSize - 1);
	lpFilename[nSize - 1] = 0;
	return (DWORD)strlen(lpFilename);
}

HMODULE GetModuleHandleA(LPCSTR lpModuleName)
{
	static int s_self;
	return lpModuleName ? NULL : (HMODULE)&s_self;
}

DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer)
{
	char cwd[1024];
	if (!getcwd(cwd, sizeof(cwd)))
		strcpy(cwd, ".");
	std::string p = std::string(cwd) + "/temp/";
	mkdir(p.c_str(), 0755);
	if (!lpBuffer || nBufferLength <= p.size())
		return (DWORD)p.size() + 1;
	strcpy(lpBuffer, p.c_str());
	return (DWORD)p.size();
}

UINT GetTempFileNameA(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName)
{
	static std::atomic<UINT> s_counter{ 1 };
	const UINT n = uUnique ? uUnique : s_counter++;
	snprintf(lpTempFileName, MAX_PATH, "%s/%.3s%04X.tmp", NormalizeSlashes(lpPathName).c_str(), lpPrefixString ? lpPrefixString : "", n & 0xffff);
	if (!uUnique)
	{
		int fd = open(lpTempFileName, O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	}
	return n;
}

DWORD GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart)
{
	std::string p = NormalizeSlashes(lpFileName);
	if (p.empty() || p[0] != '/')
	{
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)))
			p = std::string(cwd) + "/" + p;
	}
	if (!lpBuffer || nBufferLength <= p.size())
		return (DWORD)p.size() + 1;
	strcpy(lpBuffer, p.c_str());
	if (lpFilePart)
	{
		char* s = strrchr(lpBuffer, '/');
		*lpFilePart = s ? s + 1 : lpBuffer;
	}
	return (DWORD)p.size();
}

int _access(const char* path, int mode)
{
	const std::string p = ResolvePath(path);
	return access(p.c_str(), mode & 06);
}

int _mkdir(const char* path)
{
	const std::string p = ResolvePath(path, true);
	return mkdir(p.c_str(), 0755);
}

long _filelength(int fd)
{
	struct stat st;
	return fstat(fd, &st) == 0 ? (long)st.st_size : -1;
}

}	// extern "C"

// io.h _findfirst ailesi
namespace
{
	struct SFindIo
	{
		DIR* dir = nullptr;
		std::string dirPath;
		std::string pattern;
	};

	bool FindIoNext(SFindIo* f, struct _finddata_t* fi)
	{
		while (dirent* e = readdir(f->dir))
		{
			if (fnmatch(f->pattern.c_str(), e->d_name, FNM_CASEFOLD) != 0)
				continue;
			memset(fi, 0, sizeof(*fi));
			strncpy(fi->name, e->d_name, sizeof(fi->name) - 1);
			std::string full = f->dirPath.empty() ? e->d_name : f->dirPath + "/" + e->d_name;
			struct stat st;
			if (stat(full.c_str(), &st) == 0)
			{
				fi->attrib = S_ISDIR(st.st_mode) ? _A_SUBDIR : _A_NORMAL;
				fi->size = (uint32_t)st.st_size;
				fi->time_write = fi->time_create = fi->time_access = st.st_mtime;
			}
			return true;
		}
		return false;
	}
}

extern "C" intptr_t _findfirst(const char* filespec, struct _finddata_t* fileinfo)
{
	std::string spec = NormalizeSlashes(filespec);
	size_t slash = spec.rfind('/');
	SFindIo* f = new SFindIo;
	f->dirPath = slash == std::string::npos ? "" : ResolvePath(spec.substr(0, slash).c_str());
	f->pattern = slash == std::string::npos ? spec : spec.substr(slash + 1);
	if (f->pattern == "*.*")
		f->pattern = "*";
	f->dir = opendir(f->dirPath.empty() ? "." : f->dirPath.c_str());
	if (!f->dir || !FindIoNext(f, fileinfo))
	{
		if (f->dir)
			closedir(f->dir);
		delete f;
		return -1;
	}
	return (intptr_t)f;
}

extern "C" int _findnext(intptr_t handle, struct _finddata_t* fileinfo)
{
	SFindIo* f = (SFindIo*)handle;
	return (f && FindIoNext(f, fileinfo)) ? 0 : -1;
}

extern "C" int _findclose(intptr_t handle)
{
	SFindIo* f = (SFindIo*)handle;
	if (!f)
		return -1;
	if (f->dir)
		closedir(f->dir);
	delete f;
	return 0;
}
