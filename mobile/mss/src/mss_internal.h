// Cynex: cynex_mss ic yapilari - disariya acilmaz
#pragma once

#include <mss.h>

#include <stdint.h>
#include <stdio.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "../../thirdparty/miniaudio/miniaudio.h"

namespace CynexMss
{
	enum EMagic : uint32_t
	{
		MAGIC_DRIVER	= 0x44524956,
		MAGIC_SAMPLE	= 0x534D5032,
		MAGIC_OBJECT	= 0x4F424A33,
		MAGIC_STREAM	= 0x5354524D,
		MAGIC_DEAD		= 0xDEADDEAD,
	};

	enum
	{
		PROVIDER_FAST2D = 1,
	};

	struct SVec3
	{
		float x, y, z;
	};

	// Cynex: s16 interleaved, 1-2 kanal; oyunun bellegini yerinde okur, gerekirse owned'a cevrilir
	struct SPcm
	{
		const int16_t *			frames = nullptr;
		uint32_t				frameCount = 0;
		uint32_t				channels = 0;
		uint32_t				rate = 0;
		std::vector<int16_t>	owned;

		void Clear()
		{
			frames = nullptr;
			frameCount = 0;
			channels = 0;
			rate = 0;
			std::vector<int16_t>().swap(owned);
		}
	};

	struct SVoice
	{
		SPcm		pcm;
		U32			status = SMP_DONE;
		S32			loopCount = 1;		// Cynex: Miles gibi 0 = sonsuz
		S32			loopsLeft = 1;
		uint64_t	pos = 0;			// Cynex: 32.32 kaynak kare konumu
		float		gainL = 0.0f;
		float		gainR = 0.0f;
		float		curL = 0.0f;
		float		curR = 0.0f;
		bool		snapGain = true;

		void Start()
		{
			if (!pcm.frames || !pcm.frameCount)
				return;

			pos = 0;
			loopsLeft = loopCount;
			status = SMP_PLAYING;
			snapGain = true;
		}
	};

	struct SFile
	{
		AIL_file_close_callback	closecb = nullptr;
		AIL_file_seek_callback	seekcb = nullptr;
		AIL_file_read_callback	readcb = nullptr;
		U32			handle = 0;
		FILE *		fp = nullptr;
		uint32_t	size = 0;
		uint32_t	pos = 0;
		bool		open = false;
	};

	struct SWavInfo
	{
		uint16_t		format = 0;
		uint16_t		channels = 0;
		uint32_t		rate = 0;
		uint16_t		bits = 0;
		uint16_t		blockAlign = 0;
		uint16_t		samplesPerBlock = 0;
		const uint8_t *	data = nullptr;
		uint32_t		dataLen = 0;
		uint32_t		imageLen = 0;
	};

	struct SListener
	{
		SVec3	pos = { 0.0f, 0.0f, 0.0f };
		SVec3	face = { 0.0f, 0.0f, 1.0f };
		SVec3	up = { 0.0f, 1.0f, 0.0f };
	};

	U32 MixerRate();
	bool UseServiceThread();
	float VolumeCurve(float volume);

	void SetError(const char * msg);
	bool FileOpen(const char * filename, SFile * file);
	uint32_t FileRead(SFile * file, void * dest, uint32_t bytes);
	bool FileSeek(SFile * file, uint32_t pos);
	void FileClose(SFile * file);
	bool ParseWav(const void * data, uint32_t limit, SWavInfo * info);
	bool LoadPcm(const void * image, uint32_t size, bool allowCompressed, SPcm * pcm);

	void Mix2D(float * out, uint32_t frames, uint32_t outRate);
	void Mix3D(float * out, uint32_t frames, uint32_t outRate);
	void ReleaseAllSamples();

	void MixStreams(float * out, uint32_t frames);
	void CloseAllStreams();

	// Cynex: tek kilit; ses threadi miks boyunca tutar, API cagrilari kisa sure alir
	std::mutex & Lock();
}

struct CynexMssDriver
{
	uint32_t	magic;
};

struct CynexMssSample
{
	uint32_t			magic;
	CynexMss::SVoice	voice;
	float				volume;
	float				pan;
};

struct CynexMss3DObject
{
	enum EKind
	{
		KIND_LISTENER,
		KIND_SAMPLE,
	};

	uint32_t			magic;
	EKind				kind;
	CynexMss::SVec3		pos;
	CynexMss::SVec3		vel;
	CynexMss::SVec3		face;
	CynexMss::SVec3		up;
	bool				autoUpdate;

	CynexMss::SVoice	voice;
	float				volume;
	float				minDist;
	float				maxDist;
};

struct CynexMssStream
{
	uint32_t				magic = CynexMss::MAGIC_STREAM;

	// Cynex: decodeLock altinda - servis threadi ve kontrol cagrilari
	std::mutex				decodeLock;
	CynexMss::SFile			file;
	ma_decoder				decoder;
	bool					decoderReady = false;
	bool					ringReady = false;
	bool					decodedAny = false;
	bool					closed = false;
	S32						loopCount = 1;
	S32						loopsLeft = 1;

	// Cynex: ring tek yazar (decodeLock) tek okuyucu (Lock) - reset ikisini de ister
	ma_pcm_rb				ring;
	std::atomic<bool>		eof { false };

	// Cynex: Lock altinda
	U32						status = SMP_DONE;
	float					levelL = 0.0f;
	float					levelR = 0.0f;
	float					curL = 0.0f;
	float					curR = 0.0f;
	bool					snapGain = true;
};
