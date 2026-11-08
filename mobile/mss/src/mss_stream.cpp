// Cynex: BGM stream'leri - dosya oyunun geri cagirimlariyla parca parca okunur, servis threadinde cozulur
#include "mss_internal.h"

#include <math.h>
#include <string.h>
#include <chrono>
#include <condition_variable>
#include <thread>

#ifdef CYNEX_MSS_TESTING
#include "mss_testing.h"
#endif

namespace CynexMss
{
	namespace
	{
		// Cynex: 16384 kare ~340 ms (48 kHz); servis 40 ms'de bir doldurur, ses threadi sadece kopyalar
		const uint32_t		RING_FRAMES = 16384;
		const uint32_t		PREFILL_FRAMES = 8192;
		const uint32_t		REFILL_MIN_FRAMES = 2048;
		const int			SERVICE_INTERVAL_MS = 40;

		typedef std::shared_ptr<CynexMssStream> TStreamPtr;

		std::vector<TStreamPtr>		s_streams;		// Cynex: Lock() altinda

		std::mutex					s_serviceLock;
		std::condition_variable		s_serviceCv;
		std::thread *				s_service = NULL;
		bool						s_serviceRun = false;
		bool						s_serviceKick = false;

#ifdef CYNEX_MSS_TESTING
		uint32_t					s_underruns = 0;
#endif

		ma_result OnDecoderRead(ma_decoder * decoder, void * out, size_t bytes, size_t * bytesRead)
		{
			CynexMssStream * st = (CynexMssStream *) decoder->pUserData;
			const uint32_t want = bytes > 0x40000000 ? 0x40000000 : (uint32_t) bytes;
			const uint32_t got = FileRead(&st->file, out, want);

			if (bytesRead)
				*bytesRead = got;

			return (got == 0 && bytes > 0) ? MA_AT_END : MA_SUCCESS;
		}

		ma_result OnDecoderSeek(ma_decoder * decoder, ma_int64 offset, ma_seek_origin origin)
		{
			CynexMssStream * st = (CynexMssStream *) decoder->pUserData;
			int64_t base = 0;

			if (origin == ma_seek_origin_current)
				base = st->file.pos;
			else if (origin == ma_seek_origin_end)
				base = st->file.size;

			const int64_t target = base + offset;
			if (target < 0 || target > (int64_t) st->file.size)
				return MA_BAD_SEEK;

			FileSeek(&st->file, (uint32_t) target);
			return MA_SUCCESS;
		}

		// Cynex: decodeLock altinda cagrilir; dongu sayisi burada islenir, bitis ses tarafinda (eof + ring bos)
		void Fill(CynexMssStream * st, uint32_t maxFrames)
		{
			uint32_t produced = 0;
			bool rewound = false;

			while (produced < maxFrames && !st->eof.load(std::memory_order_relaxed))
			{
				ma_uint32 avail = maxFrames - produced;
				void * buffer;
				if (ma_pcm_rb_acquire_write(&st->ring, &avail, &buffer) != MA_SUCCESS || !avail)
					break;

				ma_uint64 got = 0;
				const ma_result result = ma_decoder_read_pcm_frames(&st->decoder, buffer, avail, &got);
				ma_pcm_rb_commit_write(&st->ring, (ma_uint32) got);
				produced += (uint32_t) got;

				if (got)
				{
					st->decodedAny = true;
					rewound = false;
				}

				if (result != MA_SUCCESS || !got)
				{
					// Cynex: bos/bozuk dosyada sonsuz geri sarma olmasin
					if (st->loopsLeft == 1 || rewound)
					{
						st->eof.store(true, std::memory_order_release);
						break;
					}

					if (st->loopsLeft > 1)
						--st->loopsLeft;

					if (ma_decoder_seek_to_pcm_frame(&st->decoder, 0) != MA_SUCCESS)
					{
						st->eof.store(true, std::memory_order_release);
						break;
					}

					rewound = true;
				}
			}
		}

		TStreamPtr Find(HSTREAM stream)
		{
			for (size_t i = 0; i < s_streams.size(); ++i)
			{
				if (s_streams[i].get() == stream)
					return s_streams[i];
			}
			return TStreamPtr();
		}

		void CloseStream(const TStreamPtr & st)
		{
			std::lock_guard<std::mutex> guard(st->decodeLock);
			st->closed = true;

			if (st->decoderReady)
			{
				ma_decoder_uninit(&st->decoder);
				st->decoderReady = false;
			}

			if (st->ringReady)
			{
				ma_pcm_rb_uninit(&st->ring);
				st->ringReady = false;
			}

			FileClose(&st->file);
			st->magic = MAGIC_DEAD;
		}

		bool ServiceOnce()
		{
			std::vector<TStreamPtr> work;
			bool active = false;

			{
				std::lock_guard<std::mutex> guard(Lock());
				for (size_t i = 0; i < s_streams.size(); ++i)
				{
					CynexMssStream * st = s_streams[i].get();
					if (st->status != SMP_PLAYING || st->eof.load(std::memory_order_relaxed))
						continue;

					active = true;

					if (ma_pcm_rb_available_write(&st->ring) >= REFILL_MIN_FRAMES)
						work.push_back(s_streams[i]);
				}
			}

			for (size_t i = 0; i < work.size(); ++i)
			{
				CynexMssStream * st = work[i].get();
				std::lock_guard<std::mutex> guard(st->decodeLock);
				if (!st->closed && st->decoderReady)
					Fill(st, ma_pcm_rb_available_write(&st->ring));
			}

			return active;
		}

		void ServiceLoop()
		{
			std::unique_lock<std::mutex> lock(s_serviceLock);

			while (s_serviceRun)
			{
				s_serviceKick = false;
				lock.unlock();
				const bool active = ServiceOnce();
				lock.lock();

				if (!s_serviceRun)
					break;

				if (s_serviceKick)
					continue;

				// Cynex: calan stream yoksa uyur, pil yemesin
				if (active)
					s_serviceCv.wait_for(lock, std::chrono::milliseconds(SERVICE_INTERVAL_MS), [] { return s_serviceKick || !s_serviceRun; });
				else
					s_serviceCv.wait(lock, [] { return s_serviceKick || !s_serviceRun; });
			}
		}

		void KickService()
		{
			if (!UseServiceThread())
				return;

			std::lock_guard<std::mutex> guard(s_serviceLock);
			if (!s_service)
			{
				s_serviceRun = true;
				s_service = new std::thread(ServiceLoop);
			}

			s_serviceKick = true;
			s_serviceCv.notify_one();
		}

		void StopService()
		{
			std::thread * thread;
			{
				std::lock_guard<std::mutex> guard(s_serviceLock);
				thread = s_service;
				s_service = NULL;
				s_serviceRun = false;
				s_serviceCv.notify_one();
			}

			if (thread)
			{
				thread->join();
				delete thread;
			}
		}

		ma_encoding_format SniffFormat(const uint8_t * head, uint32_t n)
		{
			if (n >= 12 && memcmp(head, "RIFF", 4) == 0 && memcmp(head + 8, "WAVE", 4) == 0)
				return ma_encoding_format_wav;
			if (n >= 3 && head[0] == 'I' && head[1] == 'D' && head[2] == '3')
				return ma_encoding_format_mp3;
			if (n >= 2 && head[0] == 0xFF && (head[1] & 0xE0) == 0xE0)
				return ma_encoding_format_mp3;
			return ma_encoding_format_unknown;
		}
	}

	void MixStreams(float * out, uint32_t frames)
	{
		for (size_t s = 0; s < s_streams.size(); ++s)
		{
			CynexMssStream * st = s_streams[s].get();
			if (st->status != SMP_PLAYING)
				continue;

			if (st->snapGain)
			{
				st->curL = st->levelL;
				st->curR = st->levelR;
				st->snapGain = false;
			}

			const bool ended = st->eof.load(std::memory_order_acquire);
			float gl = st->curL;
			float gr = st->curR;
			const float dl = (st->levelL - gl) / (float) frames;
			const float dr = (st->levelR - gr) / (float) frames;
			uint32_t done = 0;

			while (done < frames)
			{
				ma_uint32 n = frames - done;
				void * buffer;
				if (ma_pcm_rb_acquire_read(&st->ring, &n, &buffer) != MA_SUCCESS || !n)
					break;

				const float * src = (const float *) buffer;
				float * dst = out + done * 2;
				for (ma_uint32 i = 0; i < n; ++i)
				{
					dst[i * 2] += src[i * 2] * gl;
					dst[i * 2 + 1] += src[i * 2 + 1] * gr;
					gl += dl;
					gr += dr;
				}

				ma_pcm_rb_commit_read(&st->ring, n);
				done += n;
			}

			st->curL = st->levelL;
			st->curR = st->levelR;

			if (done < frames && ended && ma_pcm_rb_available_read(&st->ring) == 0)
				st->status = SMP_DONE;
#ifdef CYNEX_MSS_TESTING
			else if (done < frames)
				++s_underruns;
#endif
		}
	}

	void CloseAllStreams()
	{
		StopService();

		std::vector<TStreamPtr> streams;
		{
			std::lock_guard<std::mutex> guard(Lock());
			streams.swap(s_streams);
		}

		for (size_t i = 0; i < streams.size(); ++i)
			CloseStream(streams[i]);
	}
}

using namespace CynexMss;

extern "C" HSTREAM AILCALL AIL_open_stream(HDIGDRIVER dig, char const * filename, S32 stream_mem)
{
	(void) stream_mem;

	if (!dig || dig->magic != MAGIC_DRIVER)
	{
		SetError("Invalid driver handle.");
		return NULL;
	}

	TStreamPtr st = std::make_shared<CynexMssStream>();

	if (!FileOpen(filename, &st->file))
	{
		SetError("Unable to open file.");
		return NULL;
	}

	uint8_t head[12];
	const uint32_t n = FileRead(&st->file, head, sizeof(head));
	FileSeek(&st->file, 0);

	U32 rate;
	{
		std::lock_guard<std::mutex> guard(Lock());
		rate = MixerRate();
	}

	// Cynex: cikis cihaz bicimine cozulur (f32 stereo, miks hizi); mono kaynak iki kanala kopyalanir
	ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, rate);
	config.encodingFormat = SniffFormat(head, n);

	if (ma_decoder_init(OnDecoderRead, OnDecoderSeek, st.get(), &config, &st->decoder) != MA_SUCCESS)
	{
		FileClose(&st->file);
		SetError("Unknown file type.");
		return NULL;
	}
	st->decoderReady = true;

	if (ma_pcm_rb_init(ma_format_f32, 2, RING_FRAMES, NULL, NULL, &st->ring) != MA_SUCCESS)
	{
		CloseStream(st);
		SetError("Out of memory.");
		return NULL;
	}
	st->ringReady = true;

	// Cynex: MSS32 varsayilani hacim 1 pan 0.5 -> seviye 0.5^0.3
	st->levelL = st->levelR = powf(0.5f, 0.3f);

	{
		std::lock_guard<std::mutex> guard(Lock());
		s_streams.push_back(st);
	}

	return st.get();
}

extern "C" void AILCALL AIL_close_stream(HSTREAM stream)
{
	TStreamPtr st;
	{
		std::lock_guard<std::mutex> guard(Lock());
		for (size_t i = 0; i < s_streams.size(); ++i)
		{
			if (s_streams[i].get() == stream)
			{
				st = s_streams[i];
				s_streams.erase(s_streams.begin() + i);
				break;
			}
		}

		if (!st)
			return;

		st->status = SMP_DONE;
	}

	CloseStream(st);
}

extern "C" void AILCALL AIL_start_stream(HSTREAM stream)
{
	TStreamPtr st;
	{
		std::lock_guard<std::mutex> guard(Lock());
		st = Find(stream);
		if (!st)
			return;

		// Cynex: once ses threadinin okumasini kes, sonra ring'i sifirla
		st->status = SMP_DONE;
	}

	{
		std::lock_guard<std::mutex> guard(st->decodeLock);
		if (st->closed)
			return;

		ma_pcm_rb_reset(&st->ring);
		st->eof.store(false, std::memory_order_relaxed);

		// Cynex: MSS32'de start her zaman bastan calar
		if (st->decodedAny)
			ma_decoder_seek_to_pcm_frame(&st->decoder, 0);

		st->loopsLeft = st->loopCount;
		Fill(st.get(), PREFILL_FRAMES);
	}

	{
		std::lock_guard<std::mutex> guard(Lock());
		st->status = SMP_PLAYING;
		st->snapGain = true;
	}

	KickService();
}

extern "C" void AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff)
{
	bool resumed = false;
	{
		std::lock_guard<std::mutex> guard(Lock());
		TStreamPtr st = Find(stream);
		if (!st)
			return;

		if (onoff && st->status == SMP_PLAYING)
		{
			st->status = SMP_STOPPED;
		}
		else if (!onoff && st->status == SMP_STOPPED)
		{
			st->status = SMP_PLAYING;
			resumed = true;
		}
	}

	// Cynex: duraklatilmis stream varken servis uyuyor olabilir
	if (resumed)
		KickService();
}

extern "C" S32 AILCALL AIL_stream_status(HSTREAM stream)
{
	std::lock_guard<std::mutex> guard(Lock());
	TStreamPtr st = Find(stream);
	return st ? (S32) st->status : -1;
}

extern "C" void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count)
{
	TStreamPtr st;
	{
		std::lock_guard<std::mutex> guard(Lock());
		st = Find(stream);
	}

	if (!st)
		return;

	std::lock_guard<std::mutex> guard(st->decodeLock);
	st->loopCount = count < 0 ? 1 : count;
	st->loopsLeft = st->loopCount;
}

extern "C" void AILCALL AIL_set_stream_volume_levels(HSTREAM S, F32 left_level, F32 right_level)
{
	std::lock_guard<std::mutex> guard(Lock());
	TStreamPtr st = Find(S);
	if (!st)
		return;

	st->levelL = left_level > 0.0f ? left_level : 0.0f;
	st->levelR = right_level > 0.0f ? right_level : 0.0f;
}

extern "C" void AILCALL AIL_stream_volume_levels(HSTREAM S, F32 * left_level, F32 * right_level)
{
	std::lock_guard<std::mutex> guard(Lock());
	TStreamPtr st = Find(S);
	if (!st)
		return;

	if (left_level)
		*left_level = st->levelL;
	if (right_level)
		*right_level = st->levelR;
}

#ifdef CYNEX_MSS_TESTING
extern "C" void CynexMssTest_ServiceStreams(void)
{
	ServiceOnce();
}

extern "C" U32 CynexMssTest_StreamUnderruns(void)
{
	std::lock_guard<std::mutex> guard(Lock());
	return s_underruns;
}
#endif
