// Cynex: 2D/3D sample'lar, "Miles Fast 2D Positional Audio" saglayicisi ve ses miksi
#include "mss_internal.h"

#include <math.h>
#include <string.h>
#include <algorithm>

namespace CynexMss
{
	namespace
	{
		std::vector<CynexMssSample *>		s_samples;
		std::vector<CynexMss3DObject *>		s_objects;
		CynexMss3DObject *					s_listener = NULL;
		bool								s_providerOpen = false;
		char								s_providerName[] = "Miles Fast 2D Positional Audio";

		// Cynex: Fast 2D varsayilanlari MSS32'den olculdu (min 1, max 200; max'in otesi sessiz)
		const float		DEFAULT_MIN_DIST = 1.0f;
		const float		DEFAULT_MAX_DIST = 200.0f;
		const float		REAR_ATTENUATION = 0.75f;
		const float		PI = 3.14159265358979f;

		inline bool IsSample(HSAMPLE S)
		{
			return S && S->magic == MAGIC_SAMPLE;
		}

		inline bool IsObject(H3DPOBJECT obj)
		{
			return obj && obj->magic == MAGIC_OBJECT;
		}

		inline bool Is3DSample(H3DSAMPLE S)
		{
			return IsObject(S) && S->kind == CynexMss3DObject::KIND_SAMPLE;
		}

		inline float Clamp01(float v)
		{
			return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
		}

		inline float Dot(const SVec3 & a, const SVec3 & b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		inline SVec3 Cross(const SVec3 & a, const SVec3 & b)
		{
			SVec3 r = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
			return r;
		}

		inline bool Normalize(SVec3 & v)
		{
			const float len = sqrtf(Dot(v, v));
			if (len < 1e-12f)
				return false;
			v.x /= len;
			v.y /= len;
			v.z /= len;
			return true;
		}

		void Update2DGains(CynexMssSample * S)
		{
			// Cynex: MSS32 olcumu: sol = egri(hacim) * (1 - pan)^0.3, sag = egri(hacim) * pan^0.3
			const float level = VolumeCurve(S->volume);
			const float pan = Clamp01(S->pan);
			S->voice.gainL = level * powf(1.0f - pan, 0.3f);
			S->voice.gainR = level * powf(pan, 0.3f);
		}

		void Update3DGains(CynexMss3DObject * S)
		{
			SListener listener;
			if (s_listener)
			{
				listener.pos = s_listener->pos;
				listener.face = s_listener->face;
				listener.up = s_listener->up;
			}

			SVec3 d = { S->pos.x - listener.pos.x, S->pos.y - listener.pos.y, S->pos.z - listener.pos.z };
			const float dist = sqrtf(Dot(d, d));

			if (dist > S->maxDist)
			{
				S->voice.gainL = 0.0f;
				S->voice.gainR = 0.0f;
				return;
			}

			// Cynex: MSS32 Fast 2D olcumu: ters mesafe, pan = 0.5 + asin(sag bileseni)/pi, arka yari 0.75
			const float attenuation = dist > S->minDist ? S->minDist / dist : 1.0f;
			float pan = 0.5f;
			float rear = 1.0f;

			if (dist > 1e-6f)
			{
				const SVec3 dir = { d.x / dist, d.y / dist, d.z / dist };
				SVec3 face = listener.face;
				SVec3 right = Cross(listener.up, face);

				if (Normalize(face) && Normalize(right))
				{
					float side = Dot(dir, right);
					side = side < -1.0f ? -1.0f : (side > 1.0f ? 1.0f : side);
					pan = 0.5f + asinf(side) / PI;

					if (Dot(dir, face) < -1e-4f)
						rear = REAR_ATTENUATION;
				}
			}

			const float gain = VolumeCurve(S->volume) * attenuation * rear;
			S->voice.gainL = gain * (1.0f - pan);
			S->voice.gainR = gain * pan;
		}

		void UpdateAll3DGains()
		{
			for (size_t i = 0; i < s_objects.size(); ++i)
			{
				if (s_objects[i]->kind == CynexMss3DObject::KIND_SAMPLE)
					Update3DGains(s_objects[i]);
			}
		}

		void ObjectMoved(CynexMss3DObject * obj)
		{
			if (obj->kind == CynexMss3DObject::KIND_LISTENER)
			{
				if (obj == s_listener)
					UpdateAll3DGains();
			}
			else
			{
				Update3DGains(obj);
			}
		}

		void SetSampleIdle(SVoice & voice)
		{
			voice.status = SMP_DONE;
			voice.pos = 0;
		}

		void Resume(SVoice & voice)
		{
			// Cynex: MSS32'de bitmis sample'a resume bastan baslatir
			if (voice.status == SMP_STOPPED)
				voice.status = SMP_PLAYING;
			else if (voice.status == SMP_DONE)
				voice.Start();
		}

		void Stop(SVoice & voice)
		{
			if (voice.status == SMP_PLAYING)
				voice.status = SMP_STOPPED;
		}

		// Cynex: dogrusal ara degerleme ile hiz cevirme; STEREO_OUT=false ise stereo kaynak monoya indirilir
		template <int CHANNELS, bool STEREO_OUT>
		void MixVoiceT(SVoice & voice, float * out, uint32_t frames, uint64_t step)
		{
			const int16_t * src = voice.pcm.frames;
			const uint32_t count = voice.pcm.frameCount;
			const uint64_t end = (uint64_t) count << 32;
			const float scale = 1.0f / 32768.0f;

			float gl = voice.curL;
			float gr = voice.curR;
			const float dl = (voice.gainL - gl) / (float) frames;
			const float dr = (voice.gainR - gr) / (float) frames;

			uint64_t pos = voice.pos;

			for (uint32_t i = 0; i < frames; ++i)
			{
				const uint32_t index = (uint32_t) (pos >> 32);
				const uint32_t next = index + 1 < count ? index + 1 : index;
				const float frac = (float) (uint32_t) pos * (1.0f / 4294967296.0f);

				if (CHANNELS == 1)
				{
					const float a = src[index];
					const float s = (a + (src[next] - a) * frac) * scale;
					out[0] += s * gl;
					out[1] += s * gr;
				}
				else
				{
					const float al = src[index * 2];
					const float ar = src[index * 2 + 1];
					const float sl = (al + (src[next * 2] - al) * frac) * scale;
					const float sr = (ar + (src[next * 2 + 1] - ar) * frac) * scale;
					if (STEREO_OUT)
					{
						out[0] += sl * gl;
						out[1] += sr * gr;
					}
					else
					{
						const float s = (sl + sr) * 0.5f;
						out[0] += s * gl;
						out[1] += s * gr;
					}
				}

				out += 2;
				gl += dl;
				gr += dr;
				pos += step;

				if (pos >= end)
				{
					if (voice.loopsLeft == 1)
					{
						SetSampleIdle(voice);
						break;
					}

					if (voice.loopsLeft > 1)
						--voice.loopsLeft;

					pos -= end;
					if (pos >= end)
						pos %= end;
				}
			}

			if (voice.status == SMP_PLAYING)
				voice.pos = pos;

			voice.curL = voice.gainL;
			voice.curR = voice.gainR;
		}

		void MixVoice(SVoice & voice, float * out, uint32_t frames, uint32_t outRate, bool stereoOut)
		{
			if (voice.status != SMP_PLAYING || !voice.pcm.frames || !outRate)
				return;

			if (voice.snapGain)
			{
				voice.curL = voice.gainL;
				voice.curR = voice.gainR;
				voice.snapGain = false;
			}

			const uint64_t step = ((uint64_t) voice.pcm.rate << 32) / outRate;

			if (voice.pcm.channels == 1)
				MixVoiceT<1, true>(voice, out, frames, step);
			else if (stereoOut)
				MixVoiceT<2, true>(voice, out, frames, step);
			else
				MixVoiceT<2, false>(voice, out, frames, step);
		}

		CynexMss3DObject * NewObject(CynexMss3DObject::EKind kind)
		{
			CynexMss3DObject * obj = new CynexMss3DObject();
			obj->magic = MAGIC_OBJECT;
			obj->kind = kind;
			obj->face.z = 1.0f;
			obj->up.y = 1.0f;
			obj->volume = 1.0f;
			obj->minDist = DEFAULT_MIN_DIST;
			obj->maxDist = DEFAULT_MAX_DIST;
			return obj;
		}

		void FreeObject(CynexMss3DObject * obj)
		{
			s_objects.erase(std::remove(s_objects.begin(), s_objects.end(), obj), s_objects.end());
			if (s_listener == obj)
			{
				s_listener = NULL;
				for (size_t i = 0; i < s_objects.size(); ++i)
				{
					if (s_objects[i]->kind == CynexMss3DObject::KIND_LISTENER)
						s_listener = s_objects[i];
				}
				UpdateAll3DGains();
			}
			obj->magic = MAGIC_DEAD;
			delete obj;
		}
	}

	void Mix2D(float * out, uint32_t frames, uint32_t outRate)
	{
		for (size_t i = 0; i < s_samples.size(); ++i)
			MixVoice(s_samples[i]->voice, out, frames, outRate, true);
	}

	void Mix3D(float * out, uint32_t frames, uint32_t outRate)
	{
		if (!outRate)
			return;

		const float elapsedMs = (float) frames * 1000.0f / (float) outRate;
		bool listenerMoved = false;

		for (size_t i = 0; i < s_objects.size(); ++i)
		{
			CynexMss3DObject * obj = s_objects[i];
			if (!obj->autoUpdate || (obj->vel.x == 0.0f && obj->vel.y == 0.0f && obj->vel.z == 0.0f))
				continue;

			obj->pos.x += obj->vel.x * elapsedMs;
			obj->pos.y += obj->vel.y * elapsedMs;
			obj->pos.z += obj->vel.z * elapsedMs;

			if (obj->kind == CynexMss3DObject::KIND_SAMPLE)
				Update3DGains(obj);
			else if (obj == s_listener)
				listenerMoved = true;
		}

		if (listenerMoved)
			UpdateAll3DGains();

		for (size_t i = 0; i < s_objects.size(); ++i)
		{
			if (s_objects[i]->kind == CynexMss3DObject::KIND_SAMPLE)
				MixVoice(s_objects[i]->voice, out, frames, outRate, false);
		}
	}

	void ReleaseAllSamples()
	{
		std::lock_guard<std::mutex> guard(Lock());

		for (size_t i = 0; i < s_samples.size(); ++i)
		{
			s_samples[i]->magic = MAGIC_DEAD;
			delete s_samples[i];
		}
		s_samples.clear();

		for (size_t i = 0; i < s_objects.size(); ++i)
		{
			s_objects[i]->magic = MAGIC_DEAD;
			delete s_objects[i];
		}
		s_objects.clear();

		s_listener = NULL;
		s_providerOpen = false;
	}
}

using namespace CynexMss;

extern "C" HSAMPLE AILCALL AIL_allocate_sample_handle(HDIGDRIVER dig)
{
	if (!dig || dig->magic != MAGIC_DRIVER)
	{
		SetError("Invalid driver handle.");
		return NULL;
	}

	CynexMssSample * S = new CynexMssSample();
	S->magic = MAGIC_SAMPLE;
	S->volume = 1.0f;
	S->pan = 0.5f;
	Update2DGains(S);

	std::lock_guard<std::mutex> guard(Lock());
	s_samples.push_back(S);
	return S;
}

extern "C" void AILCALL AIL_release_sample_handle(HSAMPLE S)
{
	{
		std::lock_guard<std::mutex> guard(Lock());
		if (!IsSample(S))
			return;

		s_samples.erase(std::remove(s_samples.begin(), s_samples.end(), S), s_samples.end());
		S->magic = MAGIC_DEAD;
	}

	delete S;
}

extern "C" void AILCALL AIL_init_sample(HSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsSample(S))
		return;

	S->voice.pcm.Clear();
	S->voice.loopCount = 1;
	SetSampleIdle(S->voice);
	S->volume = 1.0f;
	S->pan = 0.5f;
	Update2DGains(S);
}

extern "C" S32 AILCALL AIL_set_sample_file(HSAMPLE S, void const * file_image, S32 block)
{
	if (!file_image)
	{
		SetError("Invalid file image.");
		return 0;
	}

	// Cynex: ayristirma/acma kilit disinda, ses threadi beklemesin
	SPcm pcm;
	const bool ok = LoadPcm(file_image, block > 0 ? (uint32_t) block : 0, true, &pcm);

	std::lock_guard<std::mutex> guard(Lock());
	if (!IsSample(S))
		return 0;

	SetSampleIdle(S->voice);
	S->voice.pcm.Clear();

	if (!ok)
		return 0;

	// Cynex: vector tasinirken tampon yerinde kalir, frames gecerli
	S->voice.pcm = std::move(pcm);
	return 1;
}

extern "C" void AILCALL AIL_start_sample(HSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (IsSample(S))
		S->voice.Start();
}

extern "C" void AILCALL AIL_stop_sample(HSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (IsSample(S))
		Stop(S->voice);
}

extern "C" void AILCALL AIL_resume_sample(HSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (IsSample(S))
		Resume(S->voice);
}

extern "C" void AILCALL AIL_end_sample(HSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (IsSample(S))
		SetSampleIdle(S->voice);
}

extern "C" U32 AILCALL AIL_sample_status(HSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	return IsSample(S) ? S->voice.status : 0;
}

extern "C" void AILCALL AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsSample(S))
		return;

	S->voice.loopCount = loop_count < 0 ? 1 : loop_count;
	if (S->voice.status != SMP_DONE)
		S->voice.loopsLeft = S->voice.loopCount;
}

extern "C" void AILCALL AIL_set_sample_volume_pan(HSAMPLE S, F32 volume, F32 pan)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsSample(S))
		return;

	S->volume = Clamp01(volume);
	S->pan = Clamp01(pan);
	Update2DGains(S);
}

extern "C" void AILCALL AIL_sample_volume_pan(HSAMPLE S, F32 * volume, F32 * pan)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsSample(S))
		return;

	if (volume)
		*volume = S->volume;
	if (pan)
		*pan = S->pan;
}

extern "C" S32 AILCALL AIL_enumerate_3D_providers(HPROENUM * next, HPROVIDER * dest, C8 ** name)
{
	if (!next || *next != HPROENUM_FIRST)
		return 0;

	*next = 1;
	if (dest)
		*dest = PROVIDER_FAST2D;
	if (name)
		*name = s_providerName;
	return 1;
}

extern "C" M3DRESULT AILCALL AIL_open_3D_provider(HPROVIDER lib)
{
	if (lib != PROVIDER_FAST2D)
	{
		SetError("Invalid 3D provider.");
		return M3D_INVALID_PARAM;
	}

	std::lock_guard<std::mutex> guard(Lock());
	s_providerOpen = true;
	return M3D_NOERR;
}

extern "C" void AILCALL AIL_close_3D_provider(HPROVIDER lib)
{
	if (lib != PROVIDER_FAST2D)
		return;

	// Cynex: tutamaclar oyunda kaliyor, sadece susturulur; bellek release/shutdown'da
	std::lock_guard<std::mutex> guard(Lock());
	s_providerOpen = false;
	for (size_t i = 0; i < s_objects.size(); ++i)
	{
		if (s_objects[i]->kind == CynexMss3DObject::KIND_SAMPLE)
			SetSampleIdle(s_objects[i]->voice);
	}
}

extern "C" H3DPOBJECT AILCALL AIL_open_3D_listener(HPROVIDER lib)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (lib != PROVIDER_FAST2D || !s_providerOpen)
	{
		SetError("3D provider not open.");
		return NULL;
	}

	CynexMss3DObject * obj = NewObject(CynexMss3DObject::KIND_LISTENER);
	s_objects.push_back(obj);
	s_listener = obj;
	UpdateAll3DGains();
	return obj;
}

extern "C" void AILCALL AIL_close_3D_listener(H3DPOBJECT listener)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (IsObject(listener) && listener->kind == CynexMss3DObject::KIND_LISTENER)
		FreeObject(listener);
}

extern "C" H3DSAMPLE AILCALL AIL_allocate_3D_sample_handle(HPROVIDER lib)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (lib != PROVIDER_FAST2D || !s_providerOpen)
	{
		SetError("3D provider not open.");
		return NULL;
	}

	CynexMss3DObject * obj = NewObject(CynexMss3DObject::KIND_SAMPLE);
	Update3DGains(obj);
	s_objects.push_back(obj);
	return obj;
}

extern "C" void AILCALL AIL_release_3D_sample_handle(H3DSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (Is3DSample(S))
		FreeObject(S);
}

extern "C" S32 AILCALL AIL_set_3D_sample_file(H3DSAMPLE S, void const * file_image)
{
	if (!file_image)
	{
		SetError("Invalid file image.");
		return 0;
	}

	SPcm pcm;
	const bool ok = LoadPcm(file_image, 0, false, &pcm);

	std::lock_guard<std::mutex> guard(Lock());
	if (!Is3DSample(S))
		return 0;

	SetSampleIdle(S->voice);
	S->voice.pcm.Clear();

	if (!ok)
		return 0;

	// Cynex: vector tasinirken tampon yerinde kalir, frames gecerli
	S->voice.pcm = std::move(pcm);
	S->minDist = DEFAULT_MIN_DIST;
	S->maxDist = DEFAULT_MAX_DIST;
	Update3DGains(S);
	return 1;
}

extern "C" void AILCALL AIL_start_3D_sample(H3DSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (Is3DSample(S))
		S->voice.Start();
}

extern "C" void AILCALL AIL_stop_3D_sample(H3DSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (Is3DSample(S))
		Stop(S->voice);
}

extern "C" void AILCALL AIL_resume_3D_sample(H3DSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (Is3DSample(S))
		Resume(S->voice);
}

extern "C" void AILCALL AIL_end_3D_sample(H3DSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (Is3DSample(S))
		SetSampleIdle(S->voice);
}

extern "C" U32 AILCALL AIL_3D_sample_status(H3DSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	return Is3DSample(S) ? S->voice.status : 0;
}

extern "C" void AILCALL AIL_set_3D_sample_loop_count(H3DSAMPLE S, U32 loops)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!Is3DSample(S))
		return;

	S->voice.loopCount = loops > 0x7FFFFFFFu ? 1 : (S32) loops;
	if (S->voice.status != SMP_DONE)
		S->voice.loopsLeft = S->voice.loopCount;
}

extern "C" void AILCALL AIL_set_3D_sample_volume(H3DSAMPLE S, F32 volume)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!Is3DSample(S))
		return;

	S->volume = Clamp01(volume);
	Update3DGains(S);
}

extern "C" F32 AILCALL AIL_3D_sample_volume(H3DSAMPLE S)
{
	std::lock_guard<std::mutex> guard(Lock());
	return Is3DSample(S) ? S->volume : 0.0f;
}

extern "C" void AILCALL AIL_set_3D_position(H3DPOBJECT obj, F32 X, F32 Y, F32 Z)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsObject(obj))
		return;

	obj->pos.x = X;
	obj->pos.y = Y;
	obj->pos.z = Z;
	ObjectMoved(obj);
}

extern "C" void AILCALL AIL_set_3D_velocity(H3DPOBJECT obj, F32 dX_per_ms, F32 dY_per_ms, F32 dZ_per_ms, F32 magnitude)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsObject(obj))
		return;

	obj->vel.x = dX_per_ms * magnitude;
	obj->vel.y = dY_per_ms * magnitude;
	obj->vel.z = dZ_per_ms * magnitude;
}

extern "C" void AILCALL AIL_set_3D_orientation(H3DPOBJECT obj, F32 X_face, F32 Y_face, F32 Z_face, F32 X_up, F32 Y_up, F32 Z_up)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsObject(obj))
		return;

	obj->face.x = X_face;
	obj->face.y = Y_face;
	obj->face.z = Z_face;
	obj->up.x = X_up;
	obj->up.y = Y_up;
	obj->up.z = Z_up;

	// Cynex: Fast 2D'de koni yok, sample yonu sesi degistirmez
	if (obj->kind == CynexMss3DObject::KIND_LISTENER)
		ObjectMoved(obj);
}

extern "C" void AILCALL AIL_auto_update_3D_position(H3DPOBJECT obj, S32 enable)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (IsObject(obj))
		obj->autoUpdate = enable != 0;
}

extern "C" void AILCALL AIL_update_3D_position(H3DPOBJECT obj, F32 dt_milliseconds)
{
	std::lock_guard<std::mutex> guard(Lock());
	if (!IsObject(obj))
		return;

	obj->pos.x += obj->vel.x * dt_milliseconds;
	obj->pos.y += obj->vel.y * dt_milliseconds;
	obj->pos.z += obj->vel.z * dt_milliseconds;
	ObjectMoved(obj);
}
