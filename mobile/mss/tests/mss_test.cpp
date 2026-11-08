// Cynex: cynex_mss windows testi - MilesLib'in cagri sirasiyla gercek oyun dosyalari uzerinde
// Cynex: MSS32 kehanet degerleri C:\CynexMobilBuild\mss\oracle (oracle.c / play3d*.c, x86 + loopback kayit)
#include <mss.h>
#include "mss_testing.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

namespace
{
	int g_pass = 0;
	int g_fail = 0;
	bool g_verbose = false;

#define CHECK(cond, ...) do { if (cond) { ++g_pass; } else { ++g_fail; printf("  FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

	const char * SES_ROOT = u8"C:/test/sound/";
	const char * YENI_ROOT = u8"C:/CynexMobilBuild/mobile_ui_kaynak/sound/";
	const char * CLIENT_ROOT = u8"C:/Barbaros2/";
	const char * ASSETS = "C:/CynexMobilBuild/mss/assets/";

	U32 g_rate = 48000;

	// Cynex: oyunun pack katmani (CSoundData + CMappedFile birebir)

	struct SGameFile
	{
		std::vector<uint8_t>	data;
		uint32_t				seek = 0;
	};

	const int SOUND_FILE_MAX_NUM = 5;
	bool		g_isSoundFile[SOUND_FILE_MAX_NUM];
	SGameFile	g_soundFile[SOUND_FILE_MAX_NUM];

	std::vector<std::string>						g_roots;
	std::map<std::string, std::vector<uint8_t>>		g_memFiles;
	int g_opens = 0, g_closes = 0, g_openNow = 0, g_openMax = 0, g_badReads = 0, g_readCalls = 0;

	bool LoadDisk(const std::string & utf8, std::vector<uint8_t> & out)
	{
		std::ifstream f(fs::u8path(utf8), std::ios::binary);
		if (!f)
			return false;
		out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
		return true;
	}

	U32 AILCALLBACK open_callback(char const * filename, U32 * file_handle)
	{
		int index = -1;
		for (int i = 0; i < SOUND_FILE_MAX_NUM; ++i)
		{
			if (!g_isSoundFile[i])
			{
				index = i;
				break;
			}
		}
		if (index == -1)
			return 0;

		SGameFile & file = g_soundFile[index];
		file.seek = 0;
		auto mem = g_memFiles.find(filename);
		bool ok = false;
		if (mem != g_memFiles.end())
		{
			file.data = mem->second;
			ok = true;
		}
		else
		{
			for (const std::string & root : g_roots)
			{
				if (LoadDisk(root + filename, file.data))
				{
					ok = true;
					break;
				}
			}
		}
		if (!ok)
			return 0;

		g_isSoundFile[index] = true;
		*file_handle = index;
		++g_opens;
		++g_openNow;
		if (g_openNow > g_openMax)
			g_openMax = g_openNow;
		return 1;
	}

	void AILCALLBACK close_callback(U32 file_handle)
	{
		if (file_handle >= SOUND_FILE_MAX_NUM || !g_isSoundFile[file_handle])
			return;
		std::vector<uint8_t>().swap(g_soundFile[file_handle].data);
		g_isSoundFile[file_handle] = false;
		++g_closes;
		--g_openNow;
	}

	S32 AILCALLBACK seek_callback(U32 file_handle, S32 offset, U32 type)
	{
		if (file_handle >= SOUND_FILE_MAX_NUM || !g_isSoundFile[file_handle])
			return 0;

		SGameFile & f = g_soundFile[file_handle];
		const uint32_t size = (uint32_t) f.data.size();
		const uint32_t off = (uint32_t) offset;
		switch (type)
		{
			case 0:
				f.seek = off > size ? size : off;
				break;
			case 1:
				f.seek = f.seek + off < size ? f.seek + off : size;
				break;
			case 2:
				f.seek = size - off;
				break;
		}
		return (S32) f.seek;
	}

	U32 AILCALLBACK read_callback(U32 file_handle, void * buffer, U32 bytes)
	{
		if (file_handle >= SOUND_FILE_MAX_NUM || !g_isSoundFile[file_handle])
			return 0;

		++g_readCalls;
		SGameFile & f = g_soundFile[file_handle];
		const uint32_t size = (uint32_t) f.data.size();
		// Cynex: oyundaki hata aynen: min(Size(), bytes) doner, asan istekte hic kopyalamaz
		const uint32_t real = size < bytes ? size : bytes;
		if (f.seek + real > size)
		{
			++g_badReads;
			return real;
		}
		memcpy(buffer, f.data.data() + f.seek, real);
		f.seek += real;
		return real;
	}

	// Cynex: yardimcilar

	std::vector<float> Render(uint32_t frames, bool service = true, uint32_t block = 480)
	{
		std::vector<float> out(frames * 2);
		uint32_t done = 0;
		while (done < frames)
		{
			const uint32_t n = frames - done < block ? frames - done : block;
			if (service)
				CynexMssTest_ServiceStreams();
			CynexMssTest_Render(&out[done * 2], n);
			done += n;
		}
		return out;
	}

	struct SLevels
	{
		double l, r;
	};

	SLevels Rms(const std::vector<float> & buf, size_t from, size_t to)
	{
		double l = 0, r = 0;
		for (size_t i = from; i < to; ++i)
		{
			l += (double) buf[i * 2] * buf[i * 2];
			r += (double) buf[i * 2 + 1] * buf[i * 2 + 1];
		}
		const double n = (double) (to - from);
		SLevels s = { sqrt(l / n), sqrt(r / n) };
		return s;
	}

	bool AllFinite(const std::vector<float> & buf)
	{
		for (float v : buf)
		{
			if (!std::isfinite(v))
				return false;
		}
		return true;
	}

	double Peak(const std::vector<float> & buf)
	{
		double p = 0;
		for (float v : buf)
			p = fabs(v) > p ? fabs(v) : p;
		return p;
	}

	struct SWav
	{
		uint16_t format = 0, channels = 0, bits = 0;
		uint32_t rate = 0;
		std::vector<int16_t> pcm;
	};

	bool ParseTestWav(const uint8_t * p, size_t size, SWav & w)
	{
		if (size < 12 || memcmp(p, "RIFF", 4) || memcmp(p + 8, "WAVE", 4))
			return false;
		size_t pos = 12;
		while (pos + 8 <= size)
		{
			uint32_t len;
			memcpy(&len, p + pos + 4, 4);
			if (!memcmp(p + pos, "fmt ", 4))
			{
				memcpy(&w.format, p + pos + 8, 2);
				memcpy(&w.channels, p + pos + 10, 2);
				memcpy(&w.rate, p + pos + 12, 4);
				memcpy(&w.bits, p + pos + 22, 2);
			}
			else if (!memcmp(p + pos, "data", 4))
			{
				if (len > size - pos - 8)
					len = (uint32_t) (size - pos - 8);
				if (w.bits == 16)
				{
					w.pcm.resize(len / 2);
					memcpy(w.pcm.data(), p + pos + 8, w.pcm.size() * 2);
				}
				return true;
			}
			pos += 8 + len + (len & 1);
		}
		return false;
	}

	// Cynex: iki dizinin en iyi kaydirmadaki normalize korelasyonu (mono, ilk n ornek)
	double BestCorrelation(const int16_t * a, size_t na, const int16_t * b, size_t nb, int maxLag, int * bestLag)
	{
		double best = -2;
		*bestLag = 0;
		const size_t n = (na < nb ? na : nb) - (size_t) maxLag;
		for (int lag = -maxLag; lag <= maxLag; ++lag)
		{
			double sab = 0, saa = 0, sbb = 0;
			for (size_t i = (size_t) maxLag; i < n; i += 3)
			{
				const double x = a[i];
				const double y = b[(size_t) ((int) i + lag)];
				sab += x * y;
				saa += x * x;
				sbb += y * y;
			}
			const double c = (saa > 0 && sbb > 0) ? sab / sqrt(saa * sbb) : 0;
			if (c > best)
			{
				best = c;
				*bestLag = lag;
			}
		}
		return best;
	}

	// Cynex: CSoundData::ReadFromDisk birebir; donen tampon AIL_mem_free_lock ile birakilir
	struct SSoundData
	{
		void *	data = nullptr;
		U32		size = 0;
		bool	withSize = false;

		void * Get() const
		{
			return withSize ? (void *) ((S32 *) data + 1) : data;
		}

		bool Read(const char * filename, S32 * typeOut = nullptr)
		{
			U32 * s = (U32 *) AIL_file_read(filename, FILE_READ_WITH_SIZE);
			if (!s)
				return false;

			const S32 type = AIL_file_type(s + 1, s[0]);
			if (typeOut)
				*typeOut = type;

			AILSOUNDINFO info;
			switch (type)
			{
				case AILFILETYPE_PCM_WAV:
					data = s;
					size = *((S32 *) s);
					withSize = true;
					break;

				case AILFILETYPE_ADPCM_WAV:
					AIL_WAV_info(s + 1, &info);
					AIL_decompress_ADPCM(&info, &data, &size);
					AIL_mem_free_lock(s);
					break;

				case AILFILETYPE_MPEG_L3_AUDIO:
					AIL_decompress_ASI(s + 1, *((S32 *) s), filename, &data, &size, 0);
					AIL_mem_free_lock(s);
					break;

				default:
					AIL_mem_free_lock(s);
					return false;
			}
			return data != nullptr;
		}

		void Free()
		{
			AIL_mem_free_lock(data);
			data = nullptr;
		}
	};

	uint32_t WavFrames(const void * image, uint32_t * rate = nullptr)
	{
		AILSOUNDINFO info;
		if (!AIL_WAV_info(image, &info))
			return 0;
		if (rate)
			*rate = info.rate;
		return info.samples;
	}

	// Cynex: sample DONE olana kadar kac kare caldigi
	uint32_t FramesUntilDone2D(HSAMPLE s, uint32_t limit)
	{
		uint32_t frames = 0;
		std::vector<float> tmp(64 * 2);
		while (AIL_sample_status(s) == SMP_PLAYING && frames < limit)
		{
			CynexMssTest_Render(tmp.data(), 64);
			frames += 64;
		}
		return frames;
	}

	uint32_t FramesUntilDone3D(H3DSAMPLE s, uint32_t limit)
	{
		uint32_t frames = 0;
		std::vector<float> tmp(64 * 2);
		while (AIL_3D_sample_status(s) == SMP_PLAYING && frames < limit)
		{
			CynexMssTest_Render(tmp.data(), 64);
			frames += 64;
		}
		return frames;
	}

	void ResetGameFs()
	{
		for (int i = 0; i < SOUND_FILE_MAX_NUM; ++i)
			g_isSoundFile[i] = false;
	}

	// Cynex: testler

	void TestFiles()
	{
		printf("[dosya] geri cagirimlar, file_read, file_type, WAV_info\n");

		const int opens0 = g_opens;
		U32 * s = (U32 *) AIL_file_read("sound/common/etc/m_7015_01.wav", FILE_READ_WITH_SIZE);
		CHECK(s && s[0] == 74028 && !memcmp(s + 1, "RIFF", 4), "file_read with size");
		CHECK(g_opens == opens0 + 1 && g_openNow == 0, "file_read geri cagirimdan gecmeli ve kapatmali");
		CHECK(AIL_file_type(s + 1, s[0]) == AILFILETYPE_PCM_WAV, "pcm type");

		AILSOUNDINFO info;
		CHECK(AIL_WAV_info(s + 1, &info) == 1, "WAV_info");
		// Cynex: MSS32: fmt=1 len=73984 rate=44100 bits=16 ch=1 samples=36992 block=2 dataoff=44
		CHECK(info.format == 1 && info.data_len == 73984 && info.rate == 44100 && info.bits == 16 && info.channels == 1 && info.samples == 36992 && info.block_size == 2, "WAV_info alanlari");
		CHECK((const char *) info.data_ptr - (const char *) (s + 1) == 44 && info.initial_ptr == info.data_ptr, "WAV_info data_ptr");
		AIL_mem_free_lock(s);

		void * plain = AIL_file_read("sound/common/etc/m_7015_01.wav", NULL);
		CHECK(plain && !memcmp(plain, "RIFF", 4), "file_read NULL dest");
		AIL_mem_free_lock(plain);

		CHECK(AIL_file_read("sound/yok_boyle_bir_dosya.wav", FILE_READ_WITH_SIZE) == NULL, "missing file");
		CHECK(!strcmp(AIL_last_error(), "Unable to open file."), "missing file error '%s'", AIL_last_error());

		U32 * m = (U32 *) AIL_file_read("sound/ambience/warp_test.mp3", FILE_READ_WITH_SIZE);
		CHECK(m && AIL_file_type(m + 1, m[0]) == AILFILETYPE_MPEG_L3_AUDIO, "mp3 type");
		AIL_mem_free_lock(m);
		U32 * b = (U32 *) AIL_file_read("bgm/login_window.mp3", FILE_READ_WITH_SIZE);
		CHECK(b && AIL_file_type(b + 1, b[0]) == AILFILETYPE_MPEG_L3_AUDIO, "bgm mp3 type");
		AIL_mem_free_lock(b);

		const char garbage[] = "garbage-garbage-garbage-garbage-garbage";
		CHECK(AIL_file_type(garbage, sizeof(garbage)) == AILFILETYPE_UNKNOWN, "garbage type");
		CHECK(AIL_WAV_info(garbage, &info) == 0, "garbage WAV_info");

		std::vector<uint8_t> noise(65536);
		uint32_t x = 12345;
		for (auto & v : noise)
		{
			x = x * 1103515245 + 12345;
			v = (uint8_t) (x >> 16);
		}
		CHECK(AIL_file_type(noise.data(), (U32) noise.size()) == AILFILETYPE_UNKNOWN, "rastgele veri mp3 sanilmamali");

		// Cynex: varsayilan stdio yolu (geri cagirim yokken)
		AIL_set_file_callbacks(NULL, NULL, NULL, NULL);
		U32 * d = (U32 *) AIL_file_read((std::string(ASSETS) + "tone.wav").c_str(), FILE_READ_WITH_SIZE);
		CHECK(d && d[0] == 35324, "stdio file_read");
		AIL_mem_free_lock(d);
		AIL_set_file_callbacks(open_callback, close_callback, seek_callback, read_callback);

		CHECK(g_openNow == 0, "acik slot kalmamali");
	}

	// Cynex: kesik/bozuk goruntuler tampon disina tasmamali (ASan derlemesinde anlamli)
	void TestCorrupt(HSAMPLE s2, H3DSAMPLE s3)
	{
		printf("[bozuk] kesik ve rastgele bozulmus wav/mp3 goruntuleri\n");

		std::vector<uint8_t> wav, mp3;
		LoadDisk(std::string(SES_ROOT) + "sound/common/etc/m_7015_01.wav", wav);
		LoadDisk(std::string(SES_ROOT) + "sound/ambience/warp_test.mp3", mp3);

		int accepted = 0;
		for (uint32_t cut = 0; cut < 200; ++cut)
		{
			std::vector<uint8_t> part(wav.begin(), wav.begin() + cut);
			part.shrink_to_fit();
			AIL_file_type(part.data(), (U32) part.size());
			const S32 ok = AIL_set_sample_file(s2, part.data(), (S32) part.size());
			if (cut < 46)
				accepted += ok;
			else if (!ok)
				--accepted;
		}
		CHECK(accepted == 0, "baslik kesikse ret, en az bir ornek varsa kabul (%d)", accepted);

		uint32_t x = 777;
		for (int i = 0; i < 3000; ++i)
		{
			std::vector<uint8_t> img(wav.begin(), wav.begin() + 4096);
			const uint32_t riff = (uint32_t) img.size() - 8;
			for (int k = 0; k < 4; ++k)
			{
				x = x * 1103515245 + 12345;
				const uint32_t at = 12 + (x >> 8) % 60;
				img[at] = (uint8_t) (x >> 20);
			}
			memcpy(&img[4], &riff, 4);
			AIL_file_type(img.data(), (U32) img.size());
			AILSOUNDINFO info;
			AIL_WAV_info(img.data(), &info);
			if (AIL_set_sample_file(s2, img.data(), (S32) img.size()))
			{
				AIL_start_sample(s2);
				Render(256, false);
				AIL_end_sample(s2);
			}
			if (AIL_set_3D_sample_file(s3, img.data()))
			{
				AIL_start_3D_sample(s3);
				Render(256, false);
				AIL_end_3D_sample(s3);
			}
		}
		AIL_init_sample(s2);

		for (uint32_t cut = 0; cut < 3000; cut += 7)
		{
			std::vector<uint8_t> part(mp3.begin(), mp3.begin() + cut);
			part.shrink_to_fit();
			AIL_file_type(part.data(), (U32) part.size());
			void * out = NULL;
			U32 size = 0;
			if (AIL_decompress_ASI(part.data(), (U32) part.size(), "x.mp3", &out, &size, 0))
				AIL_mem_free_lock(out);
		}
		CHECK(true, "bozuk girdiler");
	}

	void TestAllGameFiles(HSAMPLE s2, H3DSAMPLE s3)
	{
		printf("[toplu] iki sound klasorundeki tum wav/mp3 - ReadFromDisk + set_sample_file + set_3D_sample_file\n");

		int files = 0, ok = 0, mp3 = 0, pcm = 0, failed2D = 0, failed3D = 0, lenBad = 0;
		double seconds = 0;
		const auto t0 = std::chrono::steady_clock::now();

		const char * roots[] = { SES_ROOT, YENI_ROOT };
		for (const char * root : roots)
		{
			g_roots.assign(1, root);
			const fs::path base = fs::u8path(root);
			for (const auto & entry : fs::recursive_directory_iterator(base))
			{
				if (!entry.is_regular_file())
					continue;
				std::string ext = entry.path().extension().u8string();
				for (auto & c : ext)
					c = (char) tolower(c);
				if (ext != ".wav" && ext != ".mp3")
					continue;

				std::string rel = fs::relative(entry.path(), base).generic_u8string();
				if (rel.compare(0, 4, "bgm/") == 0)
					continue;

				++files;
				SSoundData data;
				S32 type = 0;
				if (!data.Read(rel.c_str(), &type))
				{
					printf("  okunamadi: %s (tip %d)\n", rel.c_str(), type);
					continue;
				}
				++ok;
				if (type == AILFILETYPE_MPEG_L3_AUDIO)
					++mp3;
				else
					++pcm;

				AIL_init_sample(s2);
				if (!AIL_set_sample_file(s2, data.Get(), (S32) data.size))
					++failed2D;
				if (!AIL_set_3D_sample_file(s3, data.Get()))
					++failed3D;

				uint32_t rate = 0;
				const uint32_t frames = WavFrames(data.Get(), &rate);
				if (!frames || !rate)
					++lenBad;
				else
					seconds += (double) frames / rate;

				data.Free();
			}
		}

		const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
		printf("  %d dosya, %d okundu (pcm %d, mp3 %d), toplam %.1f sn ses, %.0f ms\n", files, ok, pcm, mp3, seconds, ms);
		CHECK(files > 2000 && ok == files, "tum dosyalar okunmali (%d/%d)", ok, files);
		CHECK(failed2D == 0 && failed3D == 0 && lenBad == 0, "set_sample_file hatasi 2D=%d 3D=%d len=%d", failed2D, failed3D, lenBad);
		CHECK(g_openNow == 0 && g_badReads == 0, "slot sizintisi %d / hatali okuma %d", g_openNow, g_badReads);

		g_roots = { SES_ROOT, CLIENT_ROOT };
	}

	void TestDecompress()
	{
		printf("[acma] ASI (mp3) MSS32 ciktisiyla, ADPCM ffmpeg ile kiyas\n");

		struct SCase
		{
			const char * file;
			const char * oracle;
			uint32_t oracleFrames;
		};
		const SCase cases[] =
		{
			{ "sound/ambience/warp_test.mp3", "asi_out.wav", 375552 },
			{ "bgm/login_window.mp3", "asi_bgm.wav", 0 },
		};

		for (const SCase & c : cases)
		{
			SSoundData data;
			S32 type = 0;
			CHECK(data.Read(c.file, &type) && type == AILFILETYPE_MPEG_L3_AUDIO, "ASI read %s", c.file);
			if (!data.data)
				continue;

			SWav mine, ref;
			ParseTestWav((const uint8_t *) data.data, data.size, mine);
			std::vector<uint8_t> refBytes;
			LoadDisk(std::string(ASSETS) + c.oracle, refBytes);
			ParseTestWav(refBytes.data(), refBytes.size(), ref);

			const size_t mf = mine.pcm.size() / mine.channels;
			const size_t rf = ref.pcm.size() / (ref.channels ? ref.channels : 1);
			CHECK(mine.rate == ref.rate && mine.channels == ref.channels, "ASI bicim %u/%u vs %u/%u", mine.rate, mine.channels, ref.rate, ref.channels);
			if (c.oracleFrames)
				CHECK(rf == c.oracleFrames, "oracle kare sayisi");

			std::vector<int16_t> ma, mb;
			for (size_t i = 0; i < mf && i < 441000; ++i)
				ma.push_back(mine.pcm[i * mine.channels]);
			for (size_t i = 0; i < rf && i < 441000; ++i)
				mb.push_back(ref.pcm[i * ref.channels]);
			int lag = 0;
			const double corr = BestCorrelation(ma.data(), ma.size(), mb.data(), mb.size(), 2400, &lag);
			printf("  %s: kare %zu (MSS32 %zu, fark %+lld), korelasyon %.5f, kayma %d\n", c.file, mf, rf, (long long) mf - (long long) rf, corr, lag);
			CHECK(corr > 0.99, "ASI korelasyon %.4f", corr);
			CHECK(llabs((long long) mf - (long long) rf) <= 2400, "ASI uzunluk farki");
			data.Free();
		}

		struct SAdpcm
		{
			const char * file;
			const char * ref;
			uint16_t format;
		};
		const SAdpcm adpcm[] =
		{
			{ "test/ima.wav", "ima_ref.wav", 0x11 },
			{ "test/ima_st.wav", "ima_st_ref.wav", 0x11 },
			{ "test/ms.wav", "ms_ref.wav", 0x02 },
		};

		for (const SAdpcm & c : adpcm)
		{
			U32 * s = (U32 *) AIL_file_read(c.file, FILE_READ_WITH_SIZE);
			CHECK(s && AIL_file_type(s + 1, s[0]) == AILFILETYPE_ADPCM_WAV, "ADPCM type %s", c.file);
			if (!s)
				continue;
			AILSOUNDINFO info;
			CHECK(AIL_WAV_info(s + 1, &info) && info.format == c.format && info.bits == 4 && info.block_size == 1024, "ADPCM WAV_info");
			AIL_mem_free_lock(s);

			SSoundData data;
			CHECK(data.Read(c.file) && data.data, "ADPCM decompress %s", c.file);
			if (!data.data)
				continue;

			SWav mine, ref;
			ParseTestWav((const uint8_t *) data.data, data.size, mine);
			std::vector<uint8_t> refBytes;
			LoadDisk(std::string(ASSETS) + c.ref, refBytes);
			ParseTestWav(refBytes.data(), refBytes.size(), ref);

			const size_t n = mine.pcm.size() < ref.pcm.size() ? mine.pcm.size() : ref.pcm.size();
			int maxDiff = 0;
			for (size_t i = 0; i < n; ++i)
			{
				const int d = abs(mine.pcm[i] - ref.pcm[i]);
				maxDiff = d > maxDiff ? d : maxDiff;
			}
			printf("  %s: %zu ornek (ffmpeg %zu), en buyuk fark %d\n", c.file, mine.pcm.size(), ref.pcm.size(), maxDiff);
			CHECK(mine.channels == ref.channels && mine.rate == ref.rate, "ADPCM bicim");
			CHECK(n > 30000 && maxDiff <= 2, "ADPCM ffmpeg ile ayni olmali (fark %d)", maxDiff);

			data.Free();
		}
	}

	void Test2D(HDIGDRIVER dig)
	{
		printf("[2D] durumlar, dongu, hacim/pan (MSS32 olcumu), sure\n");

		HSAMPLE s = AIL_allocate_sample_handle(dig);
		CHECK(s && AIL_sample_status(s) == SMP_DONE, "alloc DONE");
		AIL_init_sample(s);
		CHECK(AIL_sample_status(s) == SMP_DONE, "init DONE");
		F32 v = -1, p = -1;
		AIL_sample_volume_pan(s, &v, &p);
		CHECK(v == 1.0f && p == 0.5f, "varsayilan hacim/pan");
		AIL_sample_volume_pan(s, &v, NULL);

		SSoundData tone;
		CHECK(tone.Read("test/tone.wav"), "tone");
		CHECK(AIL_set_sample_file(s, tone.Get(), (S32) tone.size) == 1, "set_sample_file");
		CHECK(AIL_sample_status(s) == SMP_DONE, "set file DONE");

		const uint32_t toneFrames = (uint32_t) (0.4 * 44100);
		const uint32_t expect = (uint32_t) ceil(toneFrames * (double) g_rate / 44100.0);
		const double ref = 0.5 / sqrt(2.0);

		struct SPan
		{
			float vol, pan, l, r;
		};
		// Cynex: MSS32 AIL_sample_volume_levels / loopback olcumu
		const SPan pans[] =
		{
			{ 1.0f, 0.5f, 0.812252f, 0.812252f },
			{ 1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.5f, 0.5f, 0.255843f, 0.255843f },
			{ 0.5f, 0.25f, 0.288936f, 0.207809f },
			{ 0.5f, 0.75f, 0.207809f, 0.288936f },
			{ 1.0f, 0.1f, 0.968886f, 0.501187f },
			{ 0.1f, 0.0f, 0.021544f, 0.0f },
		};
		for (const SPan & c : pans)
		{
			AIL_set_sample_volume_pan(s, c.vol, c.pan);
			AIL_set_sample_loop_count(s, 1);
			AIL_start_sample(s);
			CHECK(AIL_sample_status(s) == SMP_PLAYING, "start PLAYING");
			std::vector<float> out = Render(expect + 960, false);
			const SLevels lv = Rms(out, 2048, expect - 2048);
			CHECK(fabs(lv.l / ref - c.l) < 0.01 + c.l * 0.01 && fabs(lv.r / ref - c.r) < 0.01 + c.r * 0.01,
				"vol %.2f pan %.2f -> %.4f %.4f (MSS32 %.4f %.4f)", c.vol, c.pan, lv.l / ref, lv.r / ref, c.l, c.r);
			CHECK(AIL_sample_status(s) == SMP_DONE, "bitince DONE");
		}

		AIL_set_sample_volume_pan(s, 1.0f, 0.5f);

		// Cynex: sure ve dongu
		AIL_set_sample_loop_count(s, 1);
		AIL_start_sample(s);
		uint32_t played = FramesUntilDone2D(s, g_rate * 10);
		CHECK(played >= expect && played <= expect + 64, "1 dongu %u kare (beklenen %u)", played, expect);

		AIL_set_sample_loop_count(s, 3);
		AIL_start_sample(s);
		played = FramesUntilDone2D(s, g_rate * 10);
		CHECK(played >= expect * 3 - 2 && played <= expect * 3 + 64, "3 dongu %u kare (beklenen %u)", played, expect * 3);

		AIL_set_sample_loop_count(s, 0);
		AIL_start_sample(s);
		played = FramesUntilDone2D(s, expect * 6);
		CHECK(AIL_sample_status(s) == SMP_PLAYING && played >= expect * 6, "0 = sonsuz dongu");
		AIL_end_sample(s);
		CHECK(AIL_sample_status(s) == SMP_DONE, "end DONE");
		std::vector<float> silent = Render(4800, false);
		CHECK(Peak(silent) == 0.0, "end sonrasi sessiz");

		// Cynex: stop / resume (MSS32: resume bitmis sample'i baslatir, stop bitmiste etkisiz)
		AIL_set_sample_loop_count(s, 1);
		AIL_resume_sample(s);
		CHECK(AIL_sample_status(s) == SMP_PLAYING, "DONE'da resume baslatir");
		Render(4800, false);
		AIL_stop_sample(s);
		CHECK(AIL_sample_status(s) == SMP_STOPPED, "stop STOPPED");
		silent = Render(48000, false);
		CHECK(Peak(silent) == 0.0 && AIL_sample_status(s) == SMP_STOPPED, "durakken sessiz");
		AIL_resume_sample(s);
		CHECK(AIL_sample_status(s) == SMP_PLAYING, "resume PLAYING");
		played = FramesUntilDone2D(s, g_rate * 10);
		CHECK(played + 4800 >= expect && played + 4800 <= expect + 64, "durdur/devam toplam sure %u", played + 4800);
		AIL_stop_sample(s);
		CHECK(AIL_sample_status(s) == SMP_DONE, "bitmiste stop DONE kalir");
		AIL_start_sample(s);
		AIL_end_sample(s);
		CHECK(AIL_sample_status(s) == SMP_DONE, "end DONE");

		CHECK(AIL_sample_status(NULL) == 0, "NULL status 0");
		AIL_start_sample(NULL);
		AIL_set_sample_volume_pan(NULL, 1, 1);

		const char garbage[] = "garbage-garbage-garbage-garbage-garbage";
		CHECK(AIL_set_sample_file(s, garbage, sizeof(garbage)) == 0, "garbage set file");
		CHECK(!strcmp(AIL_last_error(), "Unknown file type."), "garbage error '%s'", AIL_last_error());
		CHECK(AIL_sample_status(s) == SMP_DONE, "garbage sonrasi DONE");
		AIL_start_sample(s);
		CHECK(AIL_sample_status(s) == SMP_DONE, "verisiz start etkisiz");

		// Cynex: gercek oyun dosyalari: 44.1k / 22.05k / 32k mono, 44.1k stereo, 8-bit
		const char * files[] =
		{
			"sound/common/etc/m_7015_01.wav",
			"sound/ambience/waterfall.wav",
			"sound/pc2/assassin/bow/attack.wav",
			"sound/monster2/fennec_fox/dead_1.wav",
			"test/u8.wav",
			"sound/common/etc/salute.mp3",
		};
		for (const char * f : files)
		{
			SSoundData data;
			if (!data.Read(f))
			{
				CHECK(false, "okunamadi %s", f);
				continue;
			}
			uint32_t rate = 0;
			const uint32_t frames = WavFrames(data.Get(), &rate);
			AIL_init_sample(s);
			CHECK(AIL_set_sample_file(s, data.Get(), (S32) data.size) == 1, "set file %s", f);
			AIL_set_sample_volume_pan(s, 1.0f, 0.5f);
			AIL_start_sample(s);
			const uint32_t exp = (uint32_t) ceil(frames * (double) g_rate / rate);
			std::vector<float> out = Render(exp + 256, false);
			const SLevels lv = Rms(out, 0, exp);
			CHECK(AIL_sample_status(s) == SMP_DONE, "%s bitmeli", f);
			CHECK(lv.l > 1e-3 && lv.r > 1e-3 && AllFinite(out), "%s sessiz degil (%.4f %.4f)", f, lv.l, lv.r);

			// Cynex: kaynak RMS * 0.812 ile kiyas
			SWav w;
			double src = 0;
			if (ParseTestWav((const uint8_t *) data.Get(), 1u << 30, w) && w.bits == 16)
			{
				for (int16_t x : w.pcm)
					src += (double) x * x;
				src = sqrt(src / w.pcm.size()) / 32768.0;
				const double mix = sqrt((lv.l * lv.l + lv.r * lv.r) / 2) / 0.812252;
				// Cynex: dogrusal ara degerleme genis bantli sesi (selale) yukari cevirirken tizi biraz kirpar
				const double tol = rate == g_rate ? 0.01 : (rate < 30000 ? 0.12 : 0.05);
				CHECK(fabs(mix - src) < src * tol, "%s seviye %.4f kaynak %.4f", f, mix, src);

				// Cynex: bagimsiz dogrusal ara degerleme ile ornek ornek
				if (w.channels == 1)
				{
					double worst = 0;
					const size_t n = exp - 2;
					for (size_t i = 0; i < n; ++i)
					{
						const double sp = i * (double) rate / g_rate;
						const size_t k = (size_t) sp;
						if (k + 1 >= w.pcm.size())
							break;
						const double e = (w.pcm[k] + (w.pcm[k + 1] - w.pcm[k]) * (sp - k)) / 32768.0 * 0.812252;
						const double d = fabs(out[i * 2] - e);
						worst = d > worst ? d : worst;
					}
					CHECK(worst < 2e-4, "%s ornek farki %.6f", f, worst);
				}
			}
			if (g_verbose)
				printf("  %s: %u Hz %u kare -> %u kare\n", f, rate, frames, exp);
			data.Free();
		}

		AIL_release_sample_handle(s);
		tone.Free();
	}

	struct S3DCase
	{
		float x, y, z, vol;
		float l, r;	// Cynex: MSS32 loopback olcumu
	};

	void Test3D()
	{
		printf("[3D] Miles Fast 2D Positional Audio - MSS32 olcumleriyle kiyas\n");

		HPROENUM e = HPROENUM_FIRST;
		HPROVIDER prov = 0, fast = 0;
		char * name = NULL;
		int count = 0;
		while (AIL_enumerate_3D_providers(&e, &prov, &name) && count < 32)
		{
			if (!strcmp(name, "Miles Fast 2D Positional Audio"))
				fast = prov;
			++count;
		}
		CHECK(count == 1 && fast != 0, "saglayici listesi");
		AIL_close_3D_provider(fast);
		CHECK(AIL_allocate_3D_sample_handle(fast) == NULL, "saglayici acilmadan sample yok");
		CHECK(AIL_open_3D_provider(fast) == M3D_NOERR, "open provider");
		CHECK(AIL_open_3D_provider(99) == M3D_INVALID_PARAM, "gecersiz saglayici");
		H3DPOBJECT listener = AIL_open_3D_listener(fast);
		CHECK(listener != NULL, "listener");
		H3DSAMPLE s = AIL_allocate_3D_sample_handle(fast);
		CHECK(s && AIL_3D_sample_status(s) == SMP_DONE && AIL_3D_sample_volume(s) == 1.0f, "3D alloc");

		SSoundData tone;
		tone.Read("test/tone.wav");
		CHECK(AIL_set_3D_sample_file(s, tone.Get()) == 1, "set_3D_sample_file");
		AIL_set_3D_position(s, 0, 0, 0);
		AIL_auto_update_3D_position(s, 0);

		const uint32_t expect = (uint32_t) ceil(0.4 * 44100 * (double) g_rate / 44100.0);
		const double ref = 0.5 / sqrt(2.0);

		auto measure = [&](float x, float y, float z, float vol) -> SLevels
		{
			AIL_set_3D_position(s, x, y, z);
			AIL_set_3D_sample_volume(s, vol);
			AIL_set_3D_sample_loop_count(s, 1);
			AIL_start_3D_sample(s);
			std::vector<float> out = Render(expect + 480, false);
			SLevels lv = Rms(out, 2048, expect - 2048);
			lv.l /= ref;
			lv.r /= ref;
			return lv;
		};

		const S3DCase cases[] =
		{
			{ 0, 0, 0, 1, 0.5f, 0.5f },
			{ 0, 0, 0.5f, 1, 0.5f, 0.5f },
			{ 0, 0, 1, 1, 0.5f, 0.5f },
			{ 0, 0, 2, 1, 0.25f, 0.25f },
			{ 0, 0, 4, 1, 0.125f, 0.125f },
			{ 0, 0, 8, 1, 0.0625f, 0.0625f },
			{ 0, 0, 50, 1, 0.0098f, 0.0098f },
			{ 0, 0, 100, 1, 0.0049f, 0.0049f },
			{ 0, 0, 150, 1, 0.0034f, 0.0034f },
			{ 0, 0, 200, 1, 0.0024f, 0.0024f },
			{ 0, 0, 201, 1, 0.0f, 0.0f },
			{ 0, 0, 250, 1, 0.0f, 0.0f },
			{ 1, 0, 0, 1, 0.0f, 1.0f },
			{ -1, 0, 0, 1, 1.0f, 0.0f },
			{ 0, 0, -1, 1, 0.375f, 0.375f },
			{ 2, 0, 2, 1, 0.0884f, 0.2651f },
			{ 0, 2, 0, 1, 0.25f, 0.25f },
			{ 0, 0, 2, 0.5f, 0.0786f, 0.0786f },
			{ 0, 0, 1, 0.1f, 0.0107f, 0.0107f },
			{ 0.5f, 0, 0.866025f, 1, 0.3335f, 0.6665f },
			{ 0.866025f, 0, 0.5f, 1, 0.1665f, 0.8335f },
			{ 0.866025f, 0, -0.5f, 1, 0.125f, 0.625f },
			{ 0.707107f, 0, -0.707107f, 1, 0.1875f, 0.5625f },
			{ 0.5f, 0, -0.866025f, 1, 0.25f, 0.5f },
			{ -0.5f, 0, -0.866025f, 1, 0.5f, 0.25f },
			{ 0.7071f, 0.7071f, 0, 1, 0.25f, 0.75f },
			{ 0.5f, 0.7071f, 0.5f, 1, 0.3335f, 0.6665f },
			{ 0, 0.7071f, -0.7071f, 1, 0.375f, 0.375f },
			{ 0, -0.7071f, 0.7071f, 1, 0.5f, 0.5f },
		};

		double worst = 0;
		for (const S3DCase & c : cases)
		{
			const SLevels lv = measure(c.x, c.y, c.z, c.vol);
			const double tolL = 0.02 * c.l + 0.0006;
			const double tolR = 0.02 * c.r + 0.0006;
			const double dl = fabs(lv.l - c.l), dr = fabs(lv.r - c.r);
			worst = dl > worst ? dl : worst;
			worst = dr > worst ? dr : worst;
			CHECK(dl <= tolL && dr <= tolR, "(%.3f %.3f %.3f) v%.2f -> %.4f %.4f, MSS32 %.4f %.4f", c.x, c.y, c.z, c.vol, lv.l, lv.r, c.l, c.r);
			if (g_verbose)
				printf("  (%.3f %.3f %.3f) v%.2f -> %.4f %.4f, MSS32 %.4f %.4f\n", c.x, c.y, c.z, c.vol, lv.l, lv.r, c.l, c.r);
		}
		printf("  %zu konum, MSS32'den en buyuk sapma %.4f\n", sizeof(cases) / sizeof(cases[0]), worst);

		// Cynex: dinleyici yonu ve konumu
		AIL_set_3D_orientation(listener, 1, 0, 0, 0, 1, 0);
		SLevels lv = measure(0, 0, 2, 1);
		CHECK(fabs(lv.l - 0.5) < 0.01 && lv.r < 0.001, "yuz +x, kaynak +z solda (%.4f %.4f)", lv.l, lv.r);
		lv = measure(2, 0, 0, 1);
		CHECK(fabs(lv.l - 0.25) < 0.006 && fabs(lv.r - 0.25) < 0.006, "yuz +x, kaynak onde");
		AIL_set_3D_orientation(listener, 0, 0, 1, 0, 1, 0);
		AIL_set_3D_position(listener, 5, 0, 0);
		lv = measure(5, 0, 2, 1);
		CHECK(fabs(lv.l - 0.25) < 0.006 && fabs(lv.r - 0.25) < 0.006, "dinleyici tasindi");
		AIL_set_3D_position(listener, 0, 0, 0);

		// Cynex: oyunun kamera donusumu: dunya (sag el, z yukari) -> (x, y, -z); dogu tarafindaki ses sagda
		{
			const float dir[3] = { 0, 1, 0 }, up[3] = { 0, 0, 1 };
			AIL_set_3D_orientation(listener, dir[0], dir[1], -dir[2], up[0], up[1], -up[2]);
			lv = measure(3.0f, 0.0f, -0.0f, 1);
			CHECK(lv.r > lv.l * 10, "kuzeye bakarken dogudaki ses sagda (%.4f %.4f)", lv.l, lv.r);
			AIL_set_3D_orientation(listener, 0, 0, 1, 0, 1, 0);
		}

		// Cynex: max mesafe disinda calmaya devam eder ama sessizdir
		AIL_set_3D_position(s, 0, 0, 300);
		AIL_set_3D_sample_loop_count(s, 1);
		AIL_start_3D_sample(s);
		std::vector<float> out = Render(4800, false);
		CHECK(Peak(out) == 0.0 && AIL_3D_sample_status(s) == SMP_PLAYING, "max otesi sessiz ama PLAYING");
		AIL_end_3D_sample(s);

		// Cynex: hiz + otomatik guncelleme: 0.01 birim/ms ile +x yonunde 1 sn -> x=10
		AIL_set_3D_position(s, 0, 0, 0);
		AIL_set_3D_sample_loop_count(s, 0);
		AIL_set_3D_sample_volume(s, 1);
		AIL_set_3D_velocity(s, 1, 0, 0, 0.01f);
		AIL_auto_update_3D_position(s, 1);
		AIL_start_3D_sample(s);
		out = Render(g_rate, false);
		lv = Rms(out, g_rate - 4800, g_rate);
		CHECK(fabs(lv.r / ref - 0.1) < 0.006 && lv.l / ref < 0.003, "otomatik guncelleme x=10 -> %.4f %.4f", lv.l / ref, lv.r / ref);
		AIL_auto_update_3D_position(s, 0);
		AIL_set_3D_position(s, 0, 0, 0);
		AIL_set_3D_velocity(s, 0, 0, 1, 0.5f);
		AIL_update_3D_position(s, 8.0f);
		out = Render(4800, false);
		lv = Rms(out, 0, 4800);
		CHECK(fabs(lv.l / ref - 0.125) < 0.005 && fabs(lv.r / ref - 0.125) < 0.005, "update_3D_position z=4 -> %.4f %.4f", lv.l / ref, lv.r / ref);
		AIL_end_3D_sample(s);
		AIL_set_3D_velocity(s, 0, 0, 0, 0);

		// Cynex: durumlar (MSS32 ile ayni)
		AIL_set_3D_position(s, 0, 0, 0);
		AIL_set_3D_sample_loop_count(s, 1);
		AIL_resume_3D_sample(s);
		CHECK(AIL_3D_sample_status(s) == SMP_PLAYING, "3D DONE'da resume baslatir");
		AIL_stop_3D_sample(s);
		CHECK(AIL_3D_sample_status(s) == SMP_STOPPED, "3D stop");
		AIL_resume_3D_sample(s);
		CHECK(AIL_3D_sample_status(s) == SMP_PLAYING, "3D resume");
		AIL_end_3D_sample(s);
		CHECK(AIL_3D_sample_status(s) == SMP_DONE, "3D end");
		AIL_start_3D_sample(s);
		uint32_t played = FramesUntilDone3D(s, g_rate * 5);
		CHECK(played >= expect && played <= expect + 64, "3D sure %u (beklenen %u)", played, expect);
		AIL_set_3D_sample_loop_count(s, 2);
		AIL_start_3D_sample(s);
		played = FramesUntilDone3D(s, g_rate * 5);
		CHECK(played >= expect * 2 - 2 && played <= expect * 2 + 64, "3D 2 dongu %u", played);
		CHECK(AIL_3D_sample_status(NULL) == 0, "NULL 3D status 0");

		U32 * raw = (U32 *) AIL_file_read("sound/ambience/warp_test.mp3", FILE_READ_WITH_SIZE);
		CHECK(AIL_set_3D_sample_file(s, raw + 1) == 0 && !strcmp(AIL_last_error(), "Not a valid .WAV file"), "3D ham mp3 reddedilir");
		AIL_mem_free_lock(raw);

		// Cynex: stereo kaynak 3D'de monoya iner
		SSoundData st;
		st.Read("sound/monster2/fennec_fox/dead_1.wav");
		CHECK(AIL_set_3D_sample_file(s, st.Get()) == 1, "stereo 3D");
		AIL_set_3D_position(s, 1, 0, 0);
		AIL_set_3D_sample_loop_count(s, 1);
		AIL_start_3D_sample(s);
		out = Render(g_rate / 2, false);
		lv = Rms(out, 0, g_rate / 2);
		CHECK(lv.r > 0.001 && lv.l < 1e-6, "stereo kaynak sagda (%.5f %.5f)", lv.l, lv.r);
		AIL_end_3D_sample(s);

		// Cynex: oyun akisi: 32 instance, CSoundManager3D::SetInstance gibi
		std::vector<H3DSAMPLE> inst;
		std::vector<SSoundData> datas(4);
		const char * files[] = { "sound/ambience/waterfall.wav", "sound/common/etc/m_7015_01.wav", "sound/pc2/assassin/bow/attack.wav", "sound/common/etc/salute.mp3" };
		for (int i = 0; i < 4; ++i)
			datas[i].Read(files[i]);
		for (int i = 0; i < 32; ++i)
		{
			H3DSAMPLE h = AIL_allocate_3D_sample_handle(fast);
			AIL_set_3D_sample_file(h, datas[i % 4].Get());
			AIL_set_3D_position(h, (float) (i % 7) - 3.0f, 0.0f, (float) (i % 5));
			AIL_auto_update_3D_position(h, 0);
			AIL_set_3D_sample_volume(h, 0.8f);
			AIL_set_3D_sample_loop_count(h, (i % 3) == 0 ? 0 : 1);
			AIL_start_3D_sample(h);
			inst.push_back(h);
		}
		out = Render(g_rate, false);
		CHECK(AllFinite(out) && Peak(out) > 0.01, "32 ses birlikte");
		int playing = 0;
		for (H3DSAMPLE h : inst)
			playing += AIL_3D_sample_status(h) == SMP_PLAYING;
		CHECK(playing >= 11, "donguluer hala caliyor (%d)", playing);
		for (H3DSAMPLE h : inst)
			AIL_release_3D_sample_handle(h);
		for (auto & d : datas)
			d.Free();
		st.Free();

		AIL_release_3D_sample_handle(s);
		AIL_close_3D_listener(listener);
		AIL_close_3D_provider(fast);
		tone.Free();
	}

	uint32_t RenderStreamUntilDone(HSTREAM st, uint32_t limit, std::vector<float> * keep = nullptr, uint32_t block = 480)
	{
		uint32_t frames = 0;
		std::vector<float> tmp(block * 2);
		while (AIL_stream_status(st) == SMP_PLAYING && frames < limit)
		{
			CynexMssTest_ServiceStreams();
			CynexMssTest_Render(tmp.data(), block);
			if (keep)
				keep->insert(keep->end(), tmp.begin(), tmp.end());
			frames += block;
		}
		return frames;
	}

	void TestStreams(HDIGDRIVER dig)
	{
		printf("[stream] BGM mp3 - durum, dongu, duraklatma, seviye, kapatma\n");

		const int opens0 = g_opens, closes0 = g_closes;
		HSTREAM st = AIL_open_stream(dig, "bgm/login_window.mp3", 0);
		CHECK(st != NULL, "open_stream");
		CHECK(g_opens == opens0 + 1 && g_openNow == 1, "stream geri cagirimla acilir, acik kalir");
		CHECK(AIL_stream_status(st) == SMP_DONE, "baslamadan DONE (MSS32: 2)");
		F32 l = 0, r = 0;
		AIL_stream_volume_levels(st, &l, &r);
		CHECK(fabs(l - 0.812252f) < 1e-4 && fabs(r - 0.812252f) < 1e-4, "varsayilan seviye %f", l);

		AIL_set_stream_volume_levels(st, 1.0f, 1.0f);
		AIL_set_stream_loop_count(st, 1);
		AIL_start_stream(st);
		CHECK(AIL_stream_status(st) == SMP_PLAYING, "start PLAYING");

		std::vector<float> pcm;
		const auto t0 = std::chrono::steady_clock::now();
		const uint32_t frames = RenderStreamUntilDone(st, g_rate * 60, &pcm);
		const double cpu = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
		const double ms = frames * 1000.0 / g_rate;
		printf("  login_window.mp3: %.0f ms (MSS32 toplam 19226 ms), cozme+miks %.0f ms\n", ms, cpu);
		CHECK(fabs(ms - 19226) < 80, "stream suresi %.0f ms", ms);
		CHECK(AIL_stream_status(st) == SMP_DONE, "bitince DONE");
		const SLevels lv = Rms(pcm, 0, pcm.size() / 2);
		CHECK(lv.l > 0.01 && lv.r > 0.01 && AllFinite(pcm), "stream sessiz degil");
		CHECK(g_badReads == 0, "hatali okuma yok");

		// Cynex: ayni cozucuyle ASI kiyasi: ilk 5 sn, 48k'ya cevrilmis MSS32 ciktisiyla korelasyon
		{
			std::vector<uint8_t> refBytes;
			LoadDisk(std::string(ASSETS) + "asi_bgm.wav", refBytes);
			SWav ref;
			ParseTestWav(refBytes.data(), refBytes.size(), ref);
			const size_t n = (size_t) g_rate * 5;
			std::vector<int16_t> a(n), b(n);
			for (size_t i = 0; i < n; ++i)
			{
				a[i] = (int16_t) (pcm[i * 2] * 32767.0f);
				const double srcPos = i * (double) ref.rate / g_rate;
				const size_t k = (size_t) srcPos;
				const double f = srcPos - k;
				const double s0 = ref.pcm[k * ref.channels], s1 = ref.pcm[(k + 1) * ref.channels];
				b[i] = (int16_t) (s0 + (s1 - s0) * f);
			}
			int lag = 0;
			const double corr = BestCorrelation(a.data(), n, b.data(), n, 2600, &lag);
			printf("  stream cikisi MSS32 ASI ile: korelasyon %.5f kayma %d\n", corr, lag);
			CHECK(corr > 0.98, "stream korelasyon %.4f", corr);
		}

		// Cynex: bitmisken yeniden baslat
		AIL_start_stream(st);
		CHECK(AIL_stream_status(st) == SMP_PLAYING, "yeniden start");
		Render(g_rate, true);

		// Cynex: duraklat
		AIL_pause_stream(st, 1);
		CHECK(AIL_stream_status(st) == SMP_STOPPED, "pause STOPPED (MSS32: 8)");
		std::vector<float> out = Render(g_rate, true);
		CHECK(Peak(out) == 0.0, "duraklatinca sessiz");
		AIL_pause_stream(st, 0);
		CHECK(AIL_stream_status(st) == SMP_PLAYING, "unpause PLAYING (MSS32: 4)");

		// Cynex: seviye: 0.5 -> yarisi
		out = Render(g_rate, true);
		const SLevels full = Rms(out, 0, g_rate);
		AIL_set_stream_volume_levels(st, 0.5f, 0.25f);
		Render(4800, true);
		out = Render(g_rate, true);
		const SLevels half = Rms(out, 0, g_rate);
		AIL_stream_volume_levels(st, &l, &r);
		CHECK(l == 0.5f && r == 0.25f, "seviye geri okuma");
		CHECK(full.l > 0.01 && half.l / full.l > 0.3 && half.l / full.l < 0.7 && half.r / half.l > 0.4 && half.r / half.l < 0.6, "seviye orani %.3f / %.3f", half.l / full.l, half.r / half.l);

		// Cynex: dongu 2 -> iki kat
		AIL_set_stream_volume_levels(st, 1.0f, 1.0f);
		AIL_set_stream_loop_count(st, 2);
		AIL_start_stream(st);
		uint32_t f2 = RenderStreamUntilDone(st, g_rate * 90, nullptr, 960);
		CHECK(fabs(f2 * 1000.0 / g_rate - 2 * 19226) < 160, "2 dongu %.0f ms", f2 * 1000.0 / g_rate);

		// Cynex: 0 = sonsuz (oyun Play(0) cagirir)
		AIL_set_stream_loop_count(st, 0);
		AIL_start_stream(st);
		f2 = RenderStreamUntilDone(st, g_rate * 45, nullptr, 960);
		CHECK(AIL_stream_status(st) == SMP_PLAYING && f2 >= g_rate * 45, "sonsuz dongu");

		AIL_close_stream(st);
		CHECK(g_closes == closes0 + 1 && g_openNow == 0, "close_stream dosyayi kapatir");
		CHECK(AIL_stream_status(st) == -1 && AIL_stream_status(NULL) == -1, "kapali/NULL stream -1");
		AIL_set_stream_volume_levels(st, 1, 1);
		AIL_pause_stream(st, 1);

		CHECK(AIL_open_stream(dig, "bgm/yok.mp3", 0) == NULL && !strcmp(AIL_last_error(), "Unable to open file."), "eksik stream");

		// Cynex: wav stream (tone): mono -> iki kanal, seviye 0.5 -> 0.5 (MSS32 loopback)
		HSTREAM ws = AIL_open_stream(dig, "test/tone.wav", 0);
		CHECK(ws != NULL, "wav stream");
		AIL_set_stream_volume_levels(ws, 0.5f, 0.5f);
		AIL_set_stream_loop_count(ws, 1);
		AIL_start_stream(ws);
		std::vector<float> wp;
		const uint32_t wf = RenderStreamUntilDone(ws, g_rate * 5, &wp, 240);
		const double ref = 0.5 / sqrt(2.0);
		const SLevels wl = Rms(wp, 2048, (size_t) (g_rate * 0.4) - 2048);
		CHECK(fabs(wl.l / ref - 0.5) < 0.01 && fabs(wl.r / ref - 0.5) < 0.01, "wav stream seviye %.4f %.4f", wl.l / ref, wl.r / ref);
		CHECK(fabs(wf * 1000.0 / g_rate - 400) < 12, "wav stream suresi %.1f ms", wf * 1000.0 / g_rate);
		AIL_close_stream(ws);

		// Cynex: CSoundManager::FadeInMusic gibi: iki stream ust uste, ikisi de acik dosya tutar
		HSTREAM a = AIL_open_stream(dig, "bgm/login_window.mp3", 0);
		HSTREAM b = AIL_open_stream(dig, "bgm/characterselect.mp3", 0);
		CHECK(a && b && g_openNow == 2, "iki stream");
		AIL_set_stream_volume_levels(a, 1, 1);
		AIL_set_stream_volume_levels(b, 0, 0);
		AIL_set_stream_loop_count(a, 0);
		AIL_set_stream_loop_count(b, 0);
		AIL_start_stream(a);
		AIL_start_stream(b);
		for (int i = 0; i <= 100; ++i)
		{
			AIL_set_stream_volume_levels(a, 1.0f - i * 0.01f, 1.0f - i * 0.01f);
			AIL_set_stream_volume_levels(b, i * 0.01f, i * 0.01f);
			Render(480, true);
		}
		out = Render(g_rate, true);
		CHECK(Peak(out) > 0.01 && AllFinite(out), "gecis sonrasi ses var");
		AIL_close_stream(a);
		AIL_close_stream(b);
		CHECK(g_openNow == 0, "stream dosyalari kapandi");
	}

	void TestPerformance(HDIGDRIVER dig)
	{
		printf("[perf] 32 3D + 4 2D + 2 stream, 60 sn miks\n");

		HPROENUM e = HPROENUM_FIRST;
		HPROVIDER fast = 0;
		char * name;
		AIL_enumerate_3D_providers(&e, &fast, &name);
		AIL_open_3D_provider(fast);
		H3DPOBJECT lis = AIL_open_3D_listener(fast);

		SSoundData d[3];
		d[0].Read("sound/ambience/waterfall.wav");
		d[1].Read("sound/common/etc/m_7015_01.wav");
		d[2].Read("sound/monster2/fennec_fox/dead_1.wav");

		std::vector<H3DSAMPLE> s3;
		for (int i = 0; i < 32; ++i)
		{
			H3DSAMPLE h = AIL_allocate_3D_sample_handle(fast);
			AIL_set_3D_sample_file(h, d[i % 3].Get());
			AIL_set_3D_position(h, (float) i - 16, 0, 3);
			AIL_set_3D_sample_loop_count(h, 0);
			AIL_set_3D_sample_volume(h, 0.3f);
			AIL_start_3D_sample(h);
			s3.push_back(h);
		}
		std::vector<HSAMPLE> s2;
		for (int i = 0; i < 4; ++i)
		{
			HSAMPLE h = AIL_allocate_sample_handle(dig);
			AIL_init_sample(h);
			AIL_set_sample_file(h, d[i % 3].Get(), (S32) d[i % 3].size);
			AIL_set_sample_loop_count(h, 0);
			AIL_set_sample_volume_pan(h, 0.3f, 0.5f);
			AIL_start_sample(h);
			s2.push_back(h);
		}
		HSTREAM a = AIL_open_stream(dig, "bgm/login_window.mp3", 0);
		HSTREAM b = AIL_open_stream(dig, "bgm/characterselect.mp3", 0);
		AIL_set_stream_loop_count(a, 0);
		AIL_set_stream_loop_count(b, 0);
		AIL_start_stream(a);
		AIL_start_stream(b);

		const uint32_t block = 256;
		std::vector<float> buf(block * 2);
		const uint32_t total = g_rate * 60;
		double mixMs = 0, serviceMs = 0;
		for (uint32_t done = 0; done < total; done += block)
		{
			auto t0 = std::chrono::steady_clock::now();
			if ((done / block) % 8 == 0)
				CynexMssTest_ServiceStreams();
			auto t1 = std::chrono::steady_clock::now();
			CynexMssTest_Render(buf.data(), block);
			auto t2 = std::chrono::steady_clock::now();
			serviceMs += std::chrono::duration<double, std::milli>(t1 - t0).count();
			mixMs += std::chrono::duration<double, std::milli>(t2 - t1).count();
		}
		printf("  60000 ms ses: miks %.1f ms (%.3f%%), mp3 cozme %.1f ms (%.3f%%) - masaustu tek cekirdek\n", mixMs, mixMs / 600.0, serviceMs, serviceMs / 600.0);
#ifdef NDEBUG
		CHECK(mixMs + serviceMs < 3000, "60 sn ses 3 sn'den kisa surede islenmeli");
#endif
		CHECK(AIL_stream_status(a) == SMP_PLAYING && AIL_stream_status(b) == SMP_PLAYING, "streamler kesintisiz");

		for (auto h : s3)
			AIL_release_3D_sample_handle(h);
		for (auto h : s2)
			AIL_release_sample_handle(h);
		AIL_close_stream(a);
		AIL_close_stream(b);
		AIL_close_3D_listener(lis);
		AIL_close_3D_provider(fast);
		for (auto & x : d)
			x.Free();
	}

	// Cynex: MilesLib'in Create/Destroy sirasi
	struct SGameSound
	{
		HDIGDRIVER	dig = NULL;
		HPROVIDER	provider = 0;
		H3DPOBJECT	listener = NULL;

		bool Create()
		{
			AIL_set_redist_directory("data/miles");
			AIL_startup();
			dig = AIL_open_digital_driver(44100, 16, 2, 0);
			if (!dig)
				return false;

			HPROENUM e = HPROENUM_FIRST;
			HPROVIDER p;
			char * name;
			while (AIL_enumerate_3D_providers(&e, &p, &name))
			{
				if (!strcmp(name, "Miles Fast 2D Positional Audio"))
					provider = p;
			}
			if (AIL_open_3D_provider(provider) != M3D_NOERR)
				return false;
			listener = AIL_open_3D_listener(provider);
			return listener != NULL;
		}

		void Destroy()
		{
			AIL_close_3D_listener(listener);
			AIL_close_3D_provider(provider);
			AIL_close_digital_driver(dig);
			AIL_shutdown();
		}
	};

	void TestRealtime(int mode)
	{
		printf("[%s] gercek zamanli threadler, Suspend/Resume\n", mode == CYNEX_MSS_TEST_NULL_BACKEND ? "null backend" : "cihaz");

		CynexMssTest_SetMode(mode, 0);
		const U32 underruns0 = CynexMssTest_StreamUnderruns();
		SGameSound game;
		CHECK(game.Create(), "Create");
		CHECK(CynexMssTest_DeviceRunning() == 1, "cihaz calisiyor");
		printf("  cihaz hizi %u Hz\n", CynexMssTest_MixerRate());

		HSAMPLE s = AIL_allocate_sample_handle(game.dig);
		SSoundData tone;
		tone.Read("test/tone.wav");
		AIL_init_sample(s);
		AIL_set_sample_file(s, tone.Get(), (S32) tone.size);
		AIL_set_sample_volume_pan(s, mode == CYNEX_MSS_TEST_DEVICE ? 0.05f : 1.0f, 0.5f);

		HSTREAM st = AIL_open_stream(game.dig, "bgm/characterselect.mp3", 0);
		AIL_set_stream_volume_levels(st, mode == CYNEX_MSS_TEST_DEVICE ? 0.03f : 1.0f, mode == CYNEX_MSS_TEST_DEVICE ? 0.03f : 1.0f);
		AIL_set_stream_loop_count(st, 0);
		AIL_start_stream(st);

		AIL_start_sample(s);
		const auto t0 = std::chrono::steady_clock::now();
		while (AIL_sample_status(s) == SMP_PLAYING && std::chrono::steady_clock::now() - t0 < std::chrono::seconds(3))
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
		CHECK(AIL_sample_status(s) == SMP_DONE && ms > 300 && ms < 700, "400 ms ton gercek zamanda bitti (%.0f ms)", ms);

		// Cynex: arka plan: cihaz durur, sample yerinde bekler
		AIL_start_sample(s);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		CynexMss_Suspend();
		CynexMss_Suspend();
		CHECK(CynexMssTest_DeviceRunning() == 0, "Suspend cihazi durdurur");
		std::this_thread::sleep_for(std::chrono::milliseconds(800));
		CHECK(AIL_sample_status(s) == SMP_PLAYING, "Suspend'de sample ilerlemez");
		CHECK(AIL_stream_status(st) == SMP_PLAYING, "Suspend'de stream yerinde");
		CynexMss_Resume();
		CynexMss_Resume();
		CHECK(CynexMssTest_DeviceRunning() == 1, "Resume cihazi baslatir");
		const auto t1 = std::chrono::steady_clock::now();
		while (AIL_sample_status(s) == SMP_PLAYING && std::chrono::steady_clock::now() - t1 < std::chrono::seconds(3))
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		const double rest = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count();
		CHECK(AIL_sample_status(s) == SMP_DONE && rest > 150 && rest < 500, "kalan ~300 ms Resume'dan sonra calindi (%.0f ms)", rest);

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		CHECK(AIL_stream_status(st) == SMP_PLAYING, "stream servis threadiyle suruyor");

		// Cynex: duraklat -> servis uyur; devam -> uyanip yetismeli
		AIL_pause_stream(st, 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		AIL_pause_stream(st, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(1200));
		CHECK(CynexMssTest_StreamUnderruns() == underruns0, "gercek zamanda stream acligi yok (%u)", CynexMssTest_StreamUnderruns() - underruns0);

		// Cynex: ana threadden yogun cagrilar ses/servis threadleriyle birlikte
		for (int i = 0; i < 20000; ++i)
		{
			AIL_set_stream_volume_levels(st, 0.01f, 0.01f);
			AIL_stream_status(st);
			AIL_sample_status(s);
		}

		AIL_close_stream(st);
		AIL_release_sample_handle(s);
		tone.Free();
		game.Destroy();
		CHECK(g_openNow == 0, "kapanista acik dosya yok");

		// Cynex: Suspend'de surucu acilirsa cihaz baslamaz
		CynexMss_Suspend();
		game.Create();
		CHECK(CynexMssTest_DeviceRunning() == 0, "Suspend'de acilan cihaz baslamaz");
		CynexMss_Resume();
		CHECK(CynexMssTest_DeviceRunning() == 1, "Resume sonra baslatir");
		game.Destroy();
	}
}

int main(int argc, char ** argv)
{
	bool device = false;
	for (int i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-v"))
			g_verbose = true;
		else if (!strcmp(argv[i], "--device"))
			device = true;
	}

	for (const char * f : { "tone.wav", "ima.wav", "ima_st.wav", "ms.wav", "u8.wav" })
	{
		std::vector<uint8_t> bytes;
		if (!LoadDisk(std::string(ASSETS) + f, bytes))
		{
			printf("varlik eksik: %s%s\n", ASSETS, f);
			return 2;
		}
		g_memFiles[std::string("test/") + f] = bytes;
	}
	g_roots = { SES_ROOT, CLIENT_ROOT };

	// Cynex: CSoundData::SetPackMode, AIL_startup'tan once
	ResetGameFs();
	AIL_set_file_callbacks(open_callback, close_callback, seek_callback, read_callback);

	CynexMssTest_SetMode(CYNEX_MSS_TEST_OFFLINE, g_rate);
	SGameSound game;
	CHECK(game.Create(), "MilesLib Create sirasi");
	CHECK(CynexMssTest_MixerRate() == g_rate, "cevrimdisi hiz");

	TestFiles();
	{
		HSAMPLE s2 = AIL_allocate_sample_handle(game.dig);
		H3DSAMPLE s3 = AIL_allocate_3D_sample_handle(game.provider);
		TestAllGameFiles(s2, s3);
		TestCorrupt(s2, s3);
		AIL_release_sample_handle(s2);
		AIL_release_3D_sample_handle(s3);
	}
	TestDecompress();
	Test2D(game.dig);
	game.Destroy();

	CHECK(game.Create(), "ikinci Create");
	Test3D();
	TestStreams(game.dig);
	TestPerformance(game.dig);

	// Cynex: shutdown birakilmamis her seyi kapatir
	HSAMPLE leak = AIL_allocate_sample_handle(game.dig);
	(void) leak;
	HSTREAM leakStream = AIL_open_stream(game.dig, "bgm/login_window.mp3", 0);
	CHECK(leakStream != NULL && g_openNow == 1, "kapanmamis stream");
	game.Destroy();
	CHECK(g_openNow == 0, "shutdown stream dosyasini kapatir");

	// Cynex: 44.1k cikis: hiz cevirme olmadan sure
	g_rate = 44100;
	CynexMssTest_SetMode(CYNEX_MSS_TEST_OFFLINE, g_rate);
	game.Create();
	{
		HSAMPLE s = AIL_allocate_sample_handle(game.dig);
		SSoundData tone;
		tone.Read("test/tone.wav");
		AIL_init_sample(s);
		AIL_set_sample_file(s, tone.Get(), (S32) tone.size);
		AIL_start_sample(s);
		const uint32_t played = FramesUntilDone2D(s, 44100 * 2);
		CHECK(played >= 17640 && played <= 17640 + 64, "44.1k cikista %u kare", played);
		std::vector<float> out(64 * 2);
		AIL_start_sample(s);
		CynexMssTest_Render(out.data(), 64);
		const int16_t * src = (const int16_t *) ((const uint8_t *) tone.Get() + 44);
		bool exact = true;
		for (int i = 0; i < 64; ++i)
			exact = exact && fabs(out[i * 2] - src[i] / 32768.0f * 0.812252f) < 1e-5f;
		CHECK(exact, "ayni hizda ornekler birebir");
		tone.Free();
	}
	game.Destroy();
	g_rate = 48000;

	TestRealtime(CYNEX_MSS_TEST_NULL_BACKEND);
	if (device)
		TestRealtime(CYNEX_MSS_TEST_DEVICE);

	printf("\nsonuc: %d gecti, %d kaldi (dosya acma %d, kapama %d, en fazla acik %d, hatali okuma %d)\n", g_pass, g_fail, g_opens, g_closes, g_openMax, g_badReads);
	return g_fail ? 1 : 0;
}
