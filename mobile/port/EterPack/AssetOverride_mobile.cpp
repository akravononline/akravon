// Cynex: mobil dosya katmani - "mobile/" klasoru pack'ten once okunur
// Cynex: once acik dosyalar, sonra mobile/*.zip arsivleri (ad sirasiyla, ilk bulunan kazanir)
#include "EterPack/StdAfx.h"
#include "EterBase/MappedFile.h"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>
#include <algorithm>
#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace
{
	const char* c_szRoot = "mobile";

	struct SArchive
	{
		std::string path;
		int fd = -1;
	};

	struct SEntry
	{
		int archive = -1;			// Cynex: -1 = acik dosya
		uint16_t method = 0;		// 0 stored, 8 deflate
		uint32_t compSize = 0;
		uint32_t size = 0;
		uint32_t localOffset = 0;
		std::atomic<int64_t> dataOffset{ -1 };

		SEntry() {}
		SEntry(const SEntry& o) : archive(o.archive), method(o.method), compSize(o.compSize), size(o.size), localOffset(o.localOffset), dataOffset(o.dataOffset.load()) {}
	};

	std::vector<SArchive> g_archives;
	std::unordered_map<std::string, SEntry> g_files;
	std::once_flag g_scanOnce;

	std::string Key(const char* name)
	{
		std::string k = name ? name : "";
		for (char& c : k)
		{
			if (c == '\\')
				c = '/';
			c = (char)tolower((unsigned char)c);
		}
		if (k.size() > 2 && k[1] == ':' && k[2] == '/')
			k.erase(0, 3);	// Cynex: d:/ymir work -> ymir work
		while (!k.empty() && k[0] == '/')
			k.erase(0, 1);
		return k;
	}

	uint16_t U16(const unsigned char* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
	uint32_t U32(const unsigned char* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }

	bool ReadAt(int fd, void* dst, size_t len, int64_t offset)
	{
		unsigned char* p = (unsigned char*)dst;
		while (len)
		{
			const ssize_t n = pread(fd, p, len, (off_t)offset);
			if (n <= 0)
				return false;
			p += n;
			len -= (size_t)n;
			offset += n;
		}
		return true;
	}

	void ScanLoose(const std::string& dir, const std::string& rel, std::vector<std::string>& zips)
	{
		DIR* d = opendir(dir.c_str());
		if (!d)
			return;
		while (dirent* e = readdir(d))
		{
			const std::string n = e->d_name;
			if (n == "." || n == "..")
				continue;
			const std::string full = dir + "/" + n;
			const std::string r = rel.empty() ? n : rel + "/" + n;
			struct stat st;
			if (stat(full.c_str(), &st) != 0)
				continue;
			if (S_ISDIR(st.st_mode))
				ScanLoose(full, r, zips);
			else if (rel.empty() && n.size() > 4 && Key(n.c_str()).compare(n.size() - 4, 4, ".zip") == 0)
				zips.push_back(full);
			else
				g_files[Key(r.c_str())];	// Cynex: varsayilan SEntry = acik dosya
		}
		closedir(d);
	}

	// Cynex: paketleyici korece/turkce adlari ham bayt -> latin1 -> utf-8 yaziyor, geri cevir
	std::string Utf8ToLatin1(const std::string& in)
	{
		std::string out;
		for (size_t i = 0; i < in.size(); ++i)
		{
			const unsigned char c = (unsigned char)in[i];
			if (c < 0x80)
				out += (char)c;
			else if ((c & 0xE0) == 0xC0 && i + 1 < in.size())
			{
				out += (char)(((c & 0x1F) << 6) | ((unsigned char)in[i + 1] & 0x3F));
				++i;
			}
			else
				out += (char)c;
		}
		return out;
	}

	// Cynex: zip merkez dizini (zip64 yok - paketleyici arsivleri 4GB/65535 altinda boler)
	void ScanZip(const std::string& path)
	{
		const int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
		if (fd < 0)
			return;
		struct stat st;
		if (fstat(fd, &st) != 0 || st.st_size < 22)
		{
			close(fd);
			return;
		}
		const int64_t fileSize = st.st_size;
		const int64_t tailLen = std::min<int64_t>(fileSize, 65557);
		std::vector<unsigned char> tail((size_t)tailLen);
		if (!ReadAt(fd, tail.data(), tail.size(), fileSize - tailLen))
		{
			close(fd);
			return;
		}
		int64_t eocd = -1;
		for (int64_t i = tailLen - 22; i >= 0; --i)
			if (U32(&tail[(size_t)i]) == 0x06054b50)
			{
				eocd = i;
				break;
			}
		if (eocd < 0)
		{
			close(fd);
			return;
		}
		const unsigned char* e = &tail[(size_t)eocd];
		const uint16_t count = U16(e + 10);
		const uint32_t cdSize = U32(e + 12);
		const uint32_t cdOffset = U32(e + 16);
		std::vector<unsigned char> cd(cdSize);
		if (!ReadAt(fd, cd.data(), cd.size(), cdOffset))
		{
			close(fd);
			return;
		}

		const int archive = (int)g_archives.size();
		g_archives.push_back({ path, fd });

		size_t pos = 0;
		for (uint16_t i = 0; i < count && pos + 46 <= cd.size(); ++i)
		{
			const unsigned char* h = &cd[pos];
			if (U32(h) != 0x02014b50)
				break;
			const uint16_t nameLen = U16(h + 28), extraLen = U16(h + 30), commentLen = U16(h + 32);
			if (pos + 46 + nameLen > cd.size())
				break;
			std::string raw((const char*)h + 46, nameLen);
			if (U16(h + 8) & 0x800)
				raw = Utf8ToLatin1(raw);
			const std::string name = Key(raw.c_str());
			if (!name.empty() && name.back() != '/' && g_files.find(name) == g_files.end())
			{
				SEntry& en = g_files[name];
				en.archive = archive;
				en.method = U16(h + 10);
				en.compSize = U32(h + 20);
				en.size = U32(h + 24);
				en.localOffset = U32(h + 42);
			}
			pos += 46 + nameLen + extraLen + commentLen;
		}
	}

	void ScanAll()
	{
		std::vector<std::string> zips;
		ScanLoose(c_szRoot, "", zips);
		std::sort(zips.begin(), zips.end());
		for (const std::string& z : zips)
			ScanZip(z);
		printf("Cynex: mobil dosya sayisi %u (%u arsiv)\n", (unsigned)g_files.size(), (unsigned)g_archives.size());
	}

	SEntry* Find(const char* c_szFileName)
	{
		std::call_once(g_scanOnce, ScanAll);
		if (g_files.empty())
			return nullptr;
		auto it = g_files.find(Key(c_szFileName));
		return it == g_files.end() ? nullptr : &it->second;
	}

	int64_t DataOffset(SEntry& en)
	{
		int64_t off = en.dataOffset.load(std::memory_order_relaxed);
		if (off >= 0)
			return off;
		unsigned char lh[30];
		if (!ReadAt(g_archives[en.archive].fd, lh, sizeof(lh), en.localOffset) || U32(lh) != 0x04034b50)
			return -1;
		off = (int64_t)en.localOffset + 30 + U16(lh + 26) + U16(lh + 28);
		en.dataOffset.store(off, std::memory_order_relaxed);
		return off;
	}
}

bool CynexAssetOverride_Get(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
{
	SEntry* en = Find(c_szFileName);
	if (!en)
		return false;

	if (en->archive < 0)
	{
		const std::string path = std::string(c_szRoot) + "/" + Key(c_szFileName);
		return rMappedFile.Create(path.c_str(), pData, 0, 0) ? true : false;
	}

	const int64_t off = DataOffset(*en);
	if (off < 0)
		return false;

	if (en->method == 0)
		return rMappedFile.Create(g_archives[en->archive].path.c_str(), pData, (int)off, (int)en->size) ? true : false;

	if (en->method != 8)
		return false;

	// Cynex: deflate - ham zlib akisi
	std::vector<unsigned char> comp(en->compSize);
	if (!ReadAt(g_archives[en->archive].fd, comp.data(), comp.size(), off))
		return false;
	std::vector<unsigned char> out(en->size ? en->size : 1);
	z_stream zs = {};
	if (inflateInit2(&zs, -MAX_WBITS) != Z_OK)
		return false;
	zs.next_in = comp.data();
	zs.avail_in = (uInt)comp.size();
	zs.next_out = out.data();
	zs.avail_out = (uInt)en->size;
	const int r = inflate(&zs, Z_FINISH);
	inflateEnd(&zs);
	if (r != Z_STREAM_END || zs.total_out != en->size)
	{
		printf("Cynex: arsiv dosyasi acilamadi %s\n", c_szFileName);
		return false;
	}
	const BYTE* p = rMappedFile.AppendDataBlock(out.data(), en->size);
	if (pData)
		*pData = p;
	return true;
}

// Cynex: pack.Exist / resim varligi da mobile/ katmanini gorsun
bool CynexAssetOverride_Exist(const char* c_szFileName)
{
	return Find(c_szFileName) != nullptr;
}

// Cynex: on ek ile baslayan tum dosyalar (property/ gibi pack listesi isteyenler icin)
void CynexAssetOverride_List(const char* c_szPrefix, void (*fn)(const char*, void*), void* pv)
{
	Find("");
	const std::string prefix = Key(c_szPrefix);
	std::vector<std::string> names;
	for (const auto& it : g_files)
		if (it.first.compare(0, prefix.size(), prefix) == 0)
			names.push_back(it.first);
	std::sort(names.begin(), names.end());
	for (const std::string& n : names)
		fn(n.c_str(), pv);
}
