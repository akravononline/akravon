// Cynex: dosya geri cagirimlari, wav/mp3 tanima ve acma
#include "mss_internal.h"

#include <stdlib.h>
#include <string.h>

namespace CynexMss
{
	namespace
	{
		// Cynex: oyunun geri cagirimlari thread guvenli degil (5 slotluk statik tablo), hepsi bu kilitle sirali
		std::mutex					s_fileLock;
		AIL_file_open_callback		s_openCb = NULL;
		AIL_file_close_callback		s_closeCb = NULL;
		AIL_file_seek_callback		s_seekCb = NULL;
		AIL_file_read_callback		s_readCb = NULL;

		thread_local char			s_error[256];

		inline uint16_t Read16(const uint8_t * p)
		{
			return (uint16_t) (p[0] | (p[1] << 8));
		}

		inline uint32_t Read32(const uint8_t * p)
		{
			return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
		}

		inline void Write16(uint8_t * p, uint16_t v)
		{
			p[0] = (uint8_t) v;
			p[1] = (uint8_t) (v >> 8);
		}

		inline void Write32(uint8_t * p, uint32_t v)
		{
			p[0] = (uint8_t) v;
			p[1] = (uint8_t) (v >> 8);
			p[2] = (uint8_t) (v >> 16);
			p[3] = (uint8_t) (v >> 24);
		}

		// Cynex: kbps, [surum1?][katman-1][indeks]
		const uint16_t s_mpegBitrates[2][3][16] =
		{
			{
				{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 },
				{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
				{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 },
			},
			{
				{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 },
				{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0 },
				{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 },
			},
		};

		const uint32_t s_mpegRates[3] = { 44100, 48000, 32000 };

		// Cynex: gecerli MPEG ses cercevesi ise katman (1-3) ve uzunluk, degilse 0
		int MpegFrame(const uint8_t * p, uint32_t * frameLen, uint32_t * key)
		{
			if (p[0] != 0xFF || (p[1] & 0xE0) != 0xE0)
				return 0;

			// Cynex: surum bitleri 0 = 2.5, 2 = 2, 3 = 1
			const int version = (p[1] >> 3) & 3;
			const int layerBits = (p[1] >> 1) & 3;
			const int bitrateIndex = p[2] >> 4;
			const int rateIndex = (p[2] >> 2) & 3;
			const int padding = (p[2] >> 1) & 1;

			if (version == 1 || layerBits == 0 || bitrateIndex == 0 || bitrateIndex == 15 || rateIndex == 3)
				return 0;

			const int layer = 4 - layerBits;
			const bool v1 = version == 3;
			const uint32_t bitrate = s_mpegBitrates[v1 ? 1 : 0][layer - 1][bitrateIndex] * 1000;
			uint32_t rate = s_mpegRates[rateIndex];
			if (version == 2)
				rate /= 2;
			else if (version == 0)
				rate /= 4;

			if (layer == 1)
				*frameLen = (12 * bitrate / rate + padding) * 4;
			else if (layer == 3 && !v1)
				*frameLen = 72 * bitrate / rate + padding;
			else
				*frameLen = 144 * bitrate / rate + padding;

			*key = (uint32_t) ((version << 4) | (layerBits << 2) | rateIndex);
			return *frameLen >= 4 ? layer : 0;
		}

		int MpegLayer(const uint8_t * data, uint32_t size)
		{
			uint32_t start = 0;

			if (size >= 10 && data[0] == 'I' && data[1] == 'D' && data[2] == '3')
			{
				const uint32_t tagSize = ((data[6] & 0x7F) << 21) | ((data[7] & 0x7F) << 14) | ((data[8] & 0x7F) << 7) | (data[9] & 0x7F);
				start = 10 + tagSize + ((data[5] & 0x10) ? 10 : 0);
			}

			const uint32_t scanEnd = start + 4096 < size ? start + 4096 : size;

			for (uint32_t i = start; i + 4 <= scanEnd; ++i)
			{
				uint32_t len, key;
				const int layer = MpegFrame(data + i, &len, &key);
				if (!layer)
					continue;

				// Cynex: yanlis eslesmeyi ele - sonraki cerceve de ayni turde olmali
				if (i + len + 4 > size)
					return layer;

				uint32_t len2, key2;
				if (MpegFrame(data + i + len, &len2, &key2) && key2 == key)
					return layer;
			}

			return 0;
		}

		bool DecodeAll(const void * data, size_t size, ma_encoding_format format, SPcm * pcm)
		{
			ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 0, 0);
			config.encodingFormat = format;

			ma_decoder decoder;
			if (ma_decoder_init_memory(data, size, &config, &decoder) != MA_SUCCESS)
				return false;

			if (decoder.outputChannels > 2)
			{
				ma_decoder_uninit(&decoder);
				config.channels = 2;
				if (ma_decoder_init_memory(data, size, &config, &decoder) != MA_SUCCESS)
					return false;
			}

			const uint32_t channels = decoder.outputChannels;
			const uint32_t rate = decoder.outputSampleRate;
			if (!channels || !rate)
			{
				ma_decoder_uninit(&decoder);
				return false;
			}

			std::vector<int16_t> out;
			const ma_uint64 chunk = 8192;
			uint64_t total = 0;

			for (;;)
			{
				out.resize((size_t) ((total + chunk) * channels));
				ma_uint64 got = 0;
				const ma_result result = ma_decoder_read_pcm_frames(&decoder, &out[(size_t) (total * channels)], chunk, &got);
				total += got;
				if (result != MA_SUCCESS || got < chunk)
					break;
			}

			ma_decoder_uninit(&decoder);

			if (!total || total > 0x7FFFFFFF)
				return false;

			out.resize((size_t) (total * channels));
			pcm->Clear();
			pcm->owned.swap(out);
			pcm->frames = pcm->owned.data();
			pcm->frameCount = (uint32_t) total;
			pcm->channels = channels;
			pcm->rate = rate;
			return true;
		}

		void * BuildWavImage(const SPcm & pcm, U32 * outSize)
		{
			const uint32_t dataLen = pcm.frameCount * pcm.channels * 2;
			uint8_t * image = (uint8_t *) malloc(44 + dataLen);
			if (!image)
				return NULL;

			memcpy(image, "RIFF", 4);
			Write32(image + 4, 36 + dataLen);
			memcpy(image + 8, "WAVEfmt ", 8);
			Write32(image + 16, 16);
			Write16(image + 20, 1);
			Write16(image + 22, (uint16_t) pcm.channels);
			Write32(image + 24, pcm.rate);
			Write32(image + 28, pcm.rate * pcm.channels * 2);
			Write16(image + 32, (uint16_t) (pcm.channels * 2));
			Write16(image + 34, 16);
			memcpy(image + 36, "data", 4);
			Write32(image + 40, dataLen);
			memcpy(image + 44, pcm.frames, dataLen);

			*outSize = 44 + dataLen;
			return image;
		}
	}

	void SetError(const char * msg)
	{
		strncpy(s_error, msg, sizeof(s_error) - 1);
		s_error[sizeof(s_error) - 1] = '\0';
	}

	bool FileOpen(const char * filename, SFile * file)
	{
		*file = SFile();

		if (!filename || !*filename)
			return false;

		std::lock_guard<std::mutex> guard(s_fileLock);

		if (s_openCb)
		{
			U32 handle = 0;
			if (!s_openCb(filename, &handle))
				return false;

			file->closecb = s_closeCb;
			file->seekcb = s_seekCb;
			file->readcb = s_readCb;
			file->handle = handle;

			const S32 end = s_seekCb(handle, 0, AIL_FILE_SEEK_END);
			file->size = end > 0 ? (uint32_t) end : 0;
			s_seekCb(handle, 0, AIL_FILE_SEEK_BEGIN);
		}
		else
		{
			file->fp = fopen(filename, "rb");
			if (!file->fp)
				return false;

			fseek(file->fp, 0, SEEK_END);
			const long end = ftell(file->fp);
			fseek(file->fp, 0, SEEK_SET);
			file->size = end > 0 ? (uint32_t) end : 0;
		}

		file->pos = 0;
		file->open = true;
		return true;
	}

	uint32_t FileRead(SFile * file, void * dest, uint32_t bytes)
	{
		if (!file->open || file->pos >= file->size)
			return 0;

		// Cynex: oyunun read_callback'i boyutu asan istekte hic kopyalamadan tam sayi donuyor, kalan kadar iste
		if (bytes > file->size - file->pos)
			bytes = file->size - file->pos;

		uint32_t done = 0;
		std::lock_guard<std::mutex> guard(s_fileLock);

		if (file->readcb)
		{
			while (done < bytes)
			{
				uint32_t got = file->readcb(file->handle, (uint8_t *) dest + done, bytes - done);
				if (!got)
					break;
				if (got > bytes - done)
					got = bytes - done;
				done += got;
			}
		}
		else if (file->fp)
		{
			done = (uint32_t) fread(dest, 1, bytes, file->fp);
		}

		file->pos += done;
		return done;
	}

	bool FileSeek(SFile * file, uint32_t pos)
	{
		if (!file->open)
			return false;

		if (pos > file->size)
			pos = file->size;

		std::lock_guard<std::mutex> guard(s_fileLock);

		if (file->seekcb)
			file->seekcb(file->handle, (S32) pos, AIL_FILE_SEEK_BEGIN);
		else if (file->fp)
			fseek(file->fp, (long) pos, SEEK_SET);

		file->pos = pos;
		return true;
	}

	void FileClose(SFile * file)
	{
		if (!file->open)
			return;

		{
			std::lock_guard<std::mutex> guard(s_fileLock);
			if (file->closecb)
				file->closecb(file->handle);
			else if (file->fp)
				fclose(file->fp);
		}

		*file = SFile();
	}

	bool ParseWav(const void * image, uint32_t limit, SWavInfo * info)
	{
		const uint8_t * p = (const uint8_t *) image;
		*info = SWavInfo();

		if (!p)
			return false;
		if (limit && limit < 12)
			return false;
		if (memcmp(p, "RIFF", 4) != 0 || memcmp(p + 8, "WAVE", 4) != 0)
			return false;

		if (!limit)
		{
			// Cynex: boyutu bilinmeyen goruntu (3D) - RIFF boyutuna guven, bozuksa parca boyutlarina
			const uint32_t riff = Read32(p + 4);
			limit = (riff >= 36 && riff < 0x7FFFFFF0u) ? riff + 8 : 0xFFFFFFFFu;
		}

		const uint8_t * fmt = NULL;
		uint32_t fmtLen = 0;
		uint32_t pos = 12;

		while (pos + 8 <= limit)
		{
			const uint8_t * chunk = p + pos;
			const uint32_t len = Read32(chunk + 4);

			if (memcmp(chunk, "fmt ", 4) == 0)
			{
				fmt = chunk + 8;
				fmtLen = len;
			}
			else if (memcmp(chunk, "data", 4) == 0)
			{
				if (!fmt)
					return false;

				info->data = chunk + 8;
				const uint32_t avail = limit - (pos + 8);
				info->dataLen = len < avail ? len : avail;
				info->imageLen = (uint32_t) (info->data - p) + info->dataLen;
				break;
			}

			if (len > limit - pos - 8)
				break;
			pos += 8 + len + (len & 1);
		}

		if (!fmt || fmtLen < 16 || !info->data)
			return false;

		info->format = Read16(fmt);
		info->channels = Read16(fmt + 2);
		info->rate = Read32(fmt + 4);
		info->blockAlign = Read16(fmt + 12);
		info->bits = Read16(fmt + 14);

		if (info->format == 0xFFFE && fmtLen >= 26)
			info->format = Read16(fmt + 24);
		else if (fmtLen >= 20 && (info->format == 0x0011 || info->format == 0x0002))
			info->samplesPerBlock = Read16(fmt + 18);

		return info->channels != 0 && info->rate != 0;
	}

	bool LoadPcm(const void * image, uint32_t size, bool allowCompressed, SPcm * pcm)
	{
		pcm->Clear();

		SWavInfo wav;
		if (!ParseWav(image, size, &wav))
		{
			if (allowCompressed && size && MpegLayer((const uint8_t *) image, size))
			{
				if (DecodeAll(image, size, ma_encoding_format_mp3, pcm))
					return true;
			}

			SetError(allowCompressed ? "Unknown file type." : "Not a valid .WAV file");
			return false;
		}

		const bool aligned = ((uintptr_t) wav.data & 1) == 0;

		if (wav.format == 1 && wav.bits == 16 && wav.channels <= 2 && aligned)
		{
			pcm->frames = (const int16_t *) wav.data;
			pcm->frameCount = wav.dataLen / (2u * wav.channels);
			pcm->channels = wav.channels;
			pcm->rate = wav.rate;
		}
		else if (wav.format == 1 && wav.bits == 8 && wav.channels <= 2)
		{
			pcm->owned.resize(wav.dataLen);
			for (uint32_t i = 0; i < wav.dataLen; ++i)
				pcm->owned[i] = (int16_t) ((wav.data[i] - 128) << 8);

			pcm->frames = pcm->owned.data();
			pcm->frameCount = wav.dataLen / wav.channels;
			pcm->channels = wav.channels;
			pcm->rate = wav.rate;
		}
		else if (!DecodeAll(image, wav.imageLen, ma_encoding_format_wav, pcm))
		{
			SetError("Unsupported .WAV format");
			return false;
		}

		if (!pcm->frameCount)
		{
			pcm->Clear();
			SetError("Empty .WAV file");
			return false;
		}

		return true;
	}
}

using namespace CynexMss;

extern "C" char * AILCALL AIL_last_error(void)
{
	return s_error;
}

extern "C" void AILCALL AIL_set_file_callbacks(AIL_file_open_callback opencb, AIL_file_close_callback closecb, AIL_file_seek_callback seekcb, AIL_file_read_callback readcb)
{
	std::lock_guard<std::mutex> guard(s_fileLock);

	// Cynex: biri bile eksikse varsayilan stdio'ya don
	if (opencb && closecb && seekcb && readcb)
	{
		s_openCb = opencb;
		s_closeCb = closecb;
		s_seekCb = seekcb;
		s_readCb = readcb;
	}
	else
	{
		s_openCb = NULL;
		s_closeCb = NULL;
		s_seekCb = NULL;
		s_readCb = NULL;
	}
}

extern "C" void * AILCALL AIL_file_read(char const * filename, void * dest)
{
	SFile file;
	if (!FileOpen(filename, &file))
	{
		SetError("Unable to open file.");
		return NULL;
	}

	const uint32_t size = file.size;
	uint8_t * buffer;
	uint8_t * target;

	if (dest == FILE_READ_WITH_SIZE)
	{
		buffer = (uint8_t *) malloc((size_t) size + 4);
		if (buffer)
			Write32(buffer, size);
		target = buffer ? buffer + 4 : NULL;
	}
	else if (!dest)
	{
		buffer = (uint8_t *) malloc(size ? size : 1);
		target = buffer;
	}
	else
	{
		buffer = (uint8_t *) dest;
		target = buffer;
	}

	if (!buffer)
	{
		FileClose(&file);
		SetError("Out of memory.");
		return NULL;
	}

	const uint32_t got = FileRead(&file, target, size);
	FileClose(&file);

	if (got != size)
	{
		if (buffer != dest)
			free(buffer);
		SetError("Unable to read file.");
		return NULL;
	}

	return buffer;
}

extern "C" void AILCALL AIL_mem_free_lock(void * ptr)
{
	free(ptr);
}

extern "C" S32 AILCALL AIL_file_type(void const * data, U32 size)
{
	const uint8_t * p = (const uint8_t *) data;
	if (!p || size < 4)
		return AILFILETYPE_UNKNOWN;

	if (size >= 12 && memcmp(p, "RIFF", 4) == 0 && memcmp(p + 8, "WAVE", 4) == 0)
	{
		SWavInfo wav;
		if (!ParseWav(p, size, &wav))
			return AILFILETYPE_UNKNOWN;

		switch (wav.format)
		{
			case 0x0001:
				return AILFILETYPE_PCM_WAV;

			// Cynex: Miles yalnizca IMA tanir; MS-ADPCM'i de acabildigimiz icin ayni yola sokuyoruz
			case 0x0011:
			case 0x0002:
				return AILFILETYPE_ADPCM_WAV;

			case 0x0055:
				return AILFILETYPE_OTHER_ASI_WAV;

			default:
				return AILFILETYPE_OTHER_WAV;
		}
	}

	switch (MpegLayer(p, size))
	{
		case 1:
			return AILFILETYPE_MPEG_L1_AUDIO;
		case 2:
			return AILFILETYPE_MPEG_L2_AUDIO;
		case 3:
			return AILFILETYPE_MPEG_L3_AUDIO;
	}

	return AILFILETYPE_UNKNOWN;
}

extern "C" S32 AILCALL AIL_WAV_info(void const * data, AILSOUNDINFO * info)
{
	SWavInfo wav;
	if (!info || !ParseWav(data, 0, &wav))
	{
		SetError("Not a WAV file.");
		return 0;
	}

	info->format = wav.format;
	info->data_ptr = wav.data;
	info->data_len = wav.dataLen;
	info->rate = wav.rate;
	info->bits = wav.bits;
	info->channels = wav.channels;
	info->block_size = wav.blockAlign;
	info->initial_ptr = wav.data;

	if ((wav.format == 0x0011 || wav.format == 0x0002) && wav.blockAlign)
	{
		uint32_t perBlock = wav.samplesPerBlock;
		if (!perBlock)
		{
			const uint32_t header = (wav.format == 0x0011 ? 4u : 7u) * wav.channels;
			perBlock = wav.blockAlign > header ? (wav.blockAlign - header) * 2 / wav.channels + (wav.format == 0x0011 ? 1 : 2) : 0;
		}
		info->samples = (wav.dataLen / wav.blockAlign) * perBlock;
	}
	else
	{
		const uint32_t frameBytes = (wav.bits / 8u) * wav.channels;
		info->samples = frameBytes ? wav.dataLen / frameBytes : 0;
	}

	return 1;
}

extern "C" S32 AILCALL AIL_decompress_ADPCM(AILSOUNDINFO const * info, void ** outdata, U32 * outsize)
{
	if (!info || !outdata || !outsize || !info->data_ptr || !info->data_len || !info->block_size || info->channels < 1 || info->channels > 2 || !info->rate)
	{
		SetError("Invalid ADPCM data.");
		return 0;
	}

	const bool ima = info->format == 0x0011;
	if (!ima && info->format != 0x0002)
	{
		SetError("Not an ADPCM file.");
		return 0;
	}

	// Cynex: AILSOUNDINFO'dan dr_wav'in acabilecegi tam bir wav goruntusu kur
	static const int16_t s_msCoef[7][2] = { { 256, 0 }, { 512, -256 }, { 0, 0 }, { 192, 64 }, { 240, 0 }, { 460, -208 }, { 392, -232 } };

	const uint32_t channels = (uint32_t) info->channels;
	const uint32_t block = info->block_size;
	const uint32_t header = (ima ? 4u : 7u) * channels;
	if (block <= header)
	{
		SetError("Invalid ADPCM block size.");
		return 0;
	}

	const uint32_t perBlock = (block - header) * 2 / channels + (ima ? 1 : 2);
	const uint32_t extLen = ima ? 2 : 32;
	const uint32_t fmtLen = 18 + extLen;
	const uint32_t headLen = 12 + 8 + fmtLen + 8;

	std::vector<uint8_t> image(headLen + info->data_len);
	uint8_t * p = image.data();

	memcpy(p, "RIFF", 4);
	Write32(p + 4, (uint32_t) image.size() - 8);
	memcpy(p + 8, "WAVEfmt ", 8);
	Write32(p + 16, fmtLen);
	uint8_t * fmt = p + 20;
	Write16(fmt, (uint16_t) info->format);
	Write16(fmt + 2, (uint16_t) channels);
	Write32(fmt + 4, info->rate);
	Write32(fmt + 8, (uint32_t) ((uint64_t) info->rate * block / perBlock));
	Write16(fmt + 12, (uint16_t) block);
	Write16(fmt + 14, 4);
	Write16(fmt + 16, (uint16_t) extLen);
	Write16(fmt + 18, (uint16_t) perBlock);
	if (!ima)
	{
		Write16(fmt + 20, 7);
		for (int i = 0; i < 7; ++i)
		{
			Write16(fmt + 22 + i * 4, (uint16_t) s_msCoef[i][0]);
			Write16(fmt + 24 + i * 4, (uint16_t) s_msCoef[i][1]);
		}
	}
	memcpy(p + 20 + fmtLen, "data", 4);
	Write32(p + 24 + fmtLen, info->data_len);
	memcpy(p + headLen, info->data_ptr, info->data_len);

	SPcm pcm;
	if (!DecodeAll(image.data(), image.size(), ma_encoding_format_wav, &pcm))
	{
		SetError("ADPCM decode failed.");
		return 0;
	}

	void * out = BuildWavImage(pcm, outsize);
	if (!out)
	{
		SetError("Out of memory.");
		return 0;
	}

	*outdata = out;
	return 1;
}

extern "C" S32 AILCALL AIL_decompress_ASI(void const * indata, U32 insize, char const * filename_ext, void ** wav, U32 * wavsize, AILLENGTHYCB callback)
{
	(void) filename_ext;
	(void) callback;

	if (!indata || !insize || !wav || !wavsize)
	{
		SetError("Invalid ASI data.");
		return 0;
	}

	SPcm pcm;
	const ma_encoding_format format = MpegLayer((const uint8_t *) indata, insize) ? ma_encoding_format_mp3 : ma_encoding_format_unknown;
	if (!DecodeAll(indata, insize, format, &pcm))
	{
		SetError("ASI decode failed.");
		return 0;
	}

	void * out = BuildWavImage(pcm, wavsize);
	if (!out)
	{
		SetError("Out of memory.");
		return 0;
	}

	*wav = out;
	return 1;
}
