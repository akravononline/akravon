// Cynex: bellek, log, saat ve matris yardimcilari
#include "granny_internal.h"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <mutex>

#if defined(_WIN32)
#include <malloc.h>
#endif

namespace CynexGranny
{
	const float c_afIdentity4x4[16] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	void * AlignedAlloc(size_t size, size_t alignment)
	{
		if (alignment < sizeof(void *))
			alignment = sizeof(void *);
#if defined(_WIN32)
		return _aligned_malloc(size, alignment);
#else
		void * p = NULL;
		if (posix_memalign(&p, alignment, size) != 0)
			return NULL;
		return p;
#endif
	}

	void AlignedFree(void * p)
	{
#if defined(_WIN32)
		_aligned_free(p);
#else
		free(p);
#endif
	}

	static std::mutex			gs_kLogLock;
	static granny_log_callback	gs_kLogCallback = { NULL, NULL };

	void Log(granny_log_message_type eType, granny_log_message_origin eOrigin, const char * c_szFile, int iLine, const char * c_szFormat, ...)
	{
		granny_log_callback kCallback;
		{
			std::lock_guard<std::mutex> kGuard(gs_kLogLock);
			kCallback = gs_kLogCallback;
		}
		if (!kCallback.Function)
			return;

		char szMessage[GrannyMaximumMessageBufferSize];
		va_list args;
		va_start(args, c_szFormat);
		vsnprintf(szMessage, sizeof(szMessage), c_szFormat, args);
		va_end(args);

		kCallback.Function(eType, eOrigin, c_szFile, iLine, szMessage, kCallback.UserData);
	}

	// Cynex: satir vektor duzeni, M = SS^T * R(q), oteleme son satir
	void BuildCompositeMatrix(const granny_transform & c_rTransform, float * m)
	{
		const float x = c_rTransform.Orientation[0];
		const float y = c_rTransform.Orientation[1];
		const float z = c_rTransform.Orientation[2];
		const float w = c_rTransform.Orientation[3];

		float r[3][3];
		r[0][0] = 1.0f - 2.0f * (y * y + z * z);
		r[0][1] = 2.0f * (x * y + z * w);
		r[0][2] = 2.0f * (x * z - y * w);
		r[1][0] = 2.0f * (x * y - z * w);
		r[1][1] = 1.0f - 2.0f * (x * x + z * z);
		r[1][2] = 2.0f * (y * z + x * w);
		r[2][0] = 2.0f * (x * z + y * w);
		r[2][1] = 2.0f * (y * z - x * w);
		r[2][2] = 1.0f - 2.0f * (x * x + y * y);

		if (c_rTransform.Flags & GrannyHasScaleShear)
		{
			const granny_triple * ss = c_rTransform.ScaleShear;
			for (int i = 0; i < 3; ++i)
			{
				m[i * 4 + 0] = ss[0][i] * r[0][0] + ss[1][i] * r[1][0] + ss[2][i] * r[2][0];
				m[i * 4 + 1] = ss[0][i] * r[0][1] + ss[1][i] * r[1][1] + ss[2][i] * r[2][1];
				m[i * 4 + 2] = ss[0][i] * r[0][2] + ss[1][i] * r[1][2] + ss[2][i] * r[2][2];
				m[i * 4 + 3] = 0.0f;
			}
		}
		else
		{
			for (int i = 0; i < 3; ++i)
			{
				m[i * 4 + 0] = r[i][0];
				m[i * 4 + 1] = r[i][1];
				m[i * 4 + 2] = r[i][2];
				m[i * 4 + 3] = 0.0f;
			}
		}

		m[12] = c_rTransform.Position[0];
		m[13] = c_rTransform.Position[1];
		m[14] = c_rTransform.Position[2];
		m[15] = 1.0f;
	}

	// Cynex: A ve B afin (son sutun 0001), sonuc = A * B
	void MultiplyAffine4x4(float * r, const float * a, const float * b)
	{
		float t[16];
		for (int i = 0; i < 3; ++i)
		{
			const float a0 = a[i * 4 + 0];
			const float a1 = a[i * 4 + 1];
			const float a2 = a[i * 4 + 2];
			t[i * 4 + 0] = a0 * b[0] + a1 * b[4] + a2 * b[8];
			t[i * 4 + 1] = a0 * b[1] + a1 * b[5] + a2 * b[9];
			t[i * 4 + 2] = a0 * b[2] + a1 * b[6] + a2 * b[10];
			t[i * 4 + 3] = 0.0f;
		}
		const float a0 = a[12];
		const float a1 = a[13];
		const float a2 = a[14];
		t[12] = a0 * b[0] + a1 * b[4] + a2 * b[8] + b[12];
		t[13] = a0 * b[1] + a1 * b[5] + a2 * b[9] + b[13];
		t[14] = a0 * b[2] + a1 * b[6] + a2 * b[10] + b[14];
		t[15] = 1.0f;
		memcpy(r, t, sizeof(t));
	}
}

using namespace CynexGranny;

GRANNY_DYNLINK(void) GrannySetLogCallback(granny_log_callback const * LogCallback)
{
	std::lock_guard<std::mutex> kGuard(gs_kLogLock);
	if (LogCallback)
		gs_kLogCallback = *LogCallback;
	else
	{
		gs_kLogCallback.Function = NULL;
		gs_kLogCallback.UserData = NULL;
	}
}

GRANNY_DYNLINK(char const *) GrannyGetLogMessageTypeString(granny_log_message_type Type)
{
	static const char * const s_aszNames[] = { "Ignored", "Note", "Warning", "Error" };
	if ((int) Type >= 0 && (int) Type < (int) (sizeof(s_aszNames) / sizeof(s_aszNames[0])))
		return s_aszNames[Type];
	return "UnknownMessageType";
}

GRANNY_DYNLINK(char const *) GrannyGetLogMessageOriginString(granny_log_message_origin Origin)
{
	static const char * const s_aszNames[] =
	{
		"NotImplemented", "Application", "Win32Subsystem", "Win64Subsystem", "WinXXSubsystem", "MacOSSubsystem",
		"ANSISubsystem", "NACLSubsystem", "GamecubeSubsystem", "PS2Subsystem", "PSPSubsystem", "PS3Subsystem",
		"XboxSubsystem", "XenonSubsystem", "N3DSSubsystem", "WiiSubsystem", "WiiUSubsystem", "DurangoSubsystem",
		"PS4Subsystem", "MAXSubsystem", "MayaSubsystem", "XSISubsystem", "ModoSubsystem", "LightwaveSubsystem",
		"FileWriting", "FileReading", "Exporter", "Compressor", "String", "StringTable", "VertexLayout", "Mesh",
		"Property", "Skeleton", "Animation", "SetupGraph", "Texture", "BSpline", "Hash", "Linker", "Instantiator",
		"DataType", "NameMap", "Material", "Model", "StackAllocator", "FixedAllocator", "Scene", "TrackMask",
		"LocalPose", "WorldPose", "NameLibrary", "Control", "MeshBinding", "Math", "Version", "Memory", "Deformer",
		"Voxel", "Bitmap", "IK", "Curve", "TrackGroup", "ThreadSafety", "Quantize", "BlendDag", "Assertion",
		"ArenaAllocator", "FileOperation", "Camera", "Telemetry", "GState",
	};
	if ((int) Origin >= 0 && (int) Origin < (int) (sizeof(s_aszNames) / sizeof(s_aszNames[0])))
		return s_aszNames[Origin];
	return "UnknownMessageOrigin";
}

// Cynex: system_clock icine nanosaniye sayaci (Data[0] alt, Data[1] ust 32 bit)
GRANNY_DYNLINK(void) GrannyGetSystemSeconds(granny_system_clock * Result)
{
	if (!Result)
		return;
	const uint64_t ullNanos = (uint64_t) std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	Result->Data[0] = (granny_uint32) (ullNanos & 0xFFFFFFFFu);
	Result->Data[1] = (granny_uint32) (ullNanos >> 32);
	Result->Data[2] = 0;
	Result->Data[3] = 0;
}

GRANNY_DYNLINK(granny_real32) GrannyGetSecondsElapsed(granny_system_clock const * StartClock, granny_system_clock const * EndClock)
{
	if (!StartClock || !EndClock)
		return 0.0f;
	const uint64_t ullStart = ((uint64_t) StartClock->Data[1] << 32) | StartClock->Data[0];
	const uint64_t ullEnd = ((uint64_t) EndClock->Data[1] << 32) | EndClock->Data[0];
	return (granny_real32) ((double) (int64_t) (ullEnd - ullStart) * 1e-9);
}
