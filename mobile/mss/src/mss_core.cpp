// Cynex: surucu, cihaz ve miks dongusu
#include "mss_internal.h"

#include <math.h>
#include <string.h>
#include <string>

#ifdef CYNEX_MSS_TESTING
#include "mss_testing.h"
#endif

namespace CynexMss
{
	namespace
	{
		std::mutex		s_lock;

		// Cynex: cihaz islemleri ayri kilitte; ma_device_stop miks kilidini bekleyen callback'i bekler
		std::mutex		s_deviceLock;
		ma_device		s_device;
		bool			s_deviceReady = false;
		bool			s_suspended = false;
		int				s_driverCount = 0;
		U32				s_mixerRate = 44100;

		std::string		s_redistDir;

#ifdef CYNEX_MSS_TESTING
		int				s_testMode = CYNEX_MSS_TEST_DEVICE;
		U32				s_testRate = 48000;
#endif

		void MixAll(float * out, uint32_t frames)
		{
			if (!frames)
				return;

			memset(out, 0, sizeof(float) * 2 * frames);

			std::lock_guard<std::mutex> guard(s_lock);
			const uint32_t rate = s_mixerRate;
			Mix2D(out, frames, rate);
			Mix3D(out, frames, rate);
			MixStreams(out, frames);
		}

		void DataCallback(ma_device * device, void * output, const void * input, ma_uint32 frames)
		{
			(void) device;
			(void) input;
			MixAll((float *) output, frames);
		}

		bool DeviceOpen(U32 frequency)
		{
#ifdef CYNEX_MSS_TESTING
			if (s_testMode == CYNEX_MSS_TEST_OFFLINE)
			{
				std::lock_guard<std::mutex> guard(s_lock);
				s_mixerRate = s_testRate;
				return true;
			}
#endif
			ma_device_config config = ma_device_config_init(ma_device_type_playback);
			config.playback.format = ma_format_f32;
			config.playback.channels = 2;
			config.dataCallback = DataCallback;
			// Cynex: tamponu MixAll zaten sifirliyor, callback her boyutu kaldirir - fazladan kopya olmasin
			config.noPreSilencedOutputBuffer = MA_TRUE;
			config.noFixedSizedCallback = MA_TRUE;
#if defined(__ANDROID__)
			// Cynex: yerel hizda ac, AAudio ikinci kez resample etmesin; ornekleri miksde biz ceviriyoruz
			(void) frequency;
			config.sampleRate = 0;
			config.aaudio.usage = ma_aaudio_usage_game;
			config.opensl.streamType = ma_opensl_stream_type_media;
#else
			config.sampleRate = frequency ? frequency : 44100;
#endif

			ma_result result;
#ifdef CYNEX_MSS_TESTING
			if (s_testMode == CYNEX_MSS_TEST_NULL_BACKEND)
			{
				ma_backend backends[] = { ma_backend_null };
				ma_context * context = new ma_context;
				if (ma_context_init(backends, 1, NULL, context) != MA_SUCCESS)
				{
					delete context;
					return false;
				}
				result = ma_device_init(context, &config, &s_device);
				if (result != MA_SUCCESS)
				{
					ma_context_uninit(context);
					delete context;
				}
			}
			else
#endif
			result = ma_device_init(NULL, &config, &s_device);

			if (result != MA_SUCCESS)
				return false;

			{
				std::lock_guard<std::mutex> guard(s_lock);
				s_mixerRate = s_device.sampleRate;
			}

			s_deviceReady = true;

			if (!s_suspended)
				ma_device_start(&s_device);

			return true;
		}

		void DeviceClose()
		{
			if (!s_deviceReady)
				return;

			ma_context * ownContext = NULL;
#ifdef CYNEX_MSS_TESTING
			if (s_testMode == CYNEX_MSS_TEST_NULL_BACKEND)
				ownContext = s_device.pContext;
#endif
			ma_device_uninit(&s_device);
			if (ownContext)
			{
				ma_context_uninit(ownContext);
				delete ownContext;
			}
			s_deviceReady = false;
		}
	}

	std::mutex & Lock()
	{
		return s_lock;
	}

	U32 MixerRate()
	{
		return s_mixerRate;
	}

	bool UseServiceThread()
	{
#ifdef CYNEX_MSS_TESTING
		return s_testMode != CYNEX_MSS_TEST_OFFLINE;
#else
		return true;
#endif
	}

	float VolumeCurve(float volume)
	{
		// Cynex: MSS32 olcumu: seviye = hacim^(5/3) (0.5 -> 0.31498)
		if (volume <= 0.0f)
			return 0.0f;
		if (volume >= 1.0f)
			return 1.0f;
		return powf(volume, 5.0f / 3.0f);
	}
}

using namespace CynexMss;

extern "C" S32 AILCALL AIL_startup(void)
{
	return 1;
}

extern "C" void AILCALL AIL_shutdown(void)
{
	CloseAllStreams();
	ReleaseAllSamples();

	std::lock_guard<std::mutex> guard(s_deviceLock);
	DeviceClose();
	s_driverCount = 0;
}

extern "C" char * AILCALL AIL_set_redist_directory(char const * dir)
{
	// Cynex: eklenti klasoru yok, sadece saklanir
	std::lock_guard<std::mutex> guard(s_lock);
	s_redistDir = dir ? dir : "";
	return &s_redistDir[0];
}

extern "C" HDIGDRIVER AILCALL AIL_open_digital_driver(U32 frequency, S32 bits, S32 channels, U32 flags)
{
	(void) bits;
	(void) channels;
	(void) flags;

	std::lock_guard<std::mutex> guard(s_deviceLock);

	if (s_driverCount == 0)
	{
		if (!DeviceOpen(frequency))
		{
			SetError("Unable to open audio device.");
			return NULL;
		}
	}

	++s_driverCount;

	CynexMssDriver * driver = new CynexMssDriver;
	driver->magic = MAGIC_DRIVER;
	return driver;
}

extern "C" void AILCALL AIL_close_digital_driver(HDIGDRIVER dig)
{
	if (!dig || dig->magic != MAGIC_DRIVER)
		return;

	dig->magic = MAGIC_DEAD;
	delete dig;

	std::lock_guard<std::mutex> guard(s_deviceLock);
	if (s_driverCount > 0 && --s_driverCount == 0)
		DeviceClose();
}

extern "C" void CynexMss_Suspend(void)
{
	std::lock_guard<std::mutex> guard(s_deviceLock);
	if (s_suspended)
		return;

	s_suspended = true;
	if (s_deviceReady)
		ma_device_stop(&s_device);
}

extern "C" void CynexMss_Resume(void)
{
	std::lock_guard<std::mutex> guard(s_deviceLock);
	if (!s_suspended)
		return;

	s_suspended = false;
	if (s_deviceReady)
		ma_device_start(&s_device);
}

#ifdef CYNEX_MSS_TESTING
extern "C" void CynexMssTest_SetMode(int mode, U32 offlineRate)
{
	s_testMode = mode;
	s_testRate = offlineRate ? offlineRate : 48000;
}

extern "C" void CynexMssTest_Render(float * out, U32 frames)
{
	MixAll(out, frames);
}

extern "C" U32 CynexMssTest_MixerRate(void)
{
	std::lock_guard<std::mutex> guard(s_lock);
	return s_mixerRate;
}

extern "C" int CynexMssTest_DeviceRunning(void)
{
	std::lock_guard<std::mutex> guard(s_deviceLock);
	return s_deviceReady && ma_device_is_started(&s_device) ? 1 : 0;
}
#endif
