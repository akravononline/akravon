// Cynex: miniaudio uygulamasi - yalnizca cihaz, wav/mp3 cozucu ve ring buffer gerekiyor
#define MA_NO_ENCODING
#define MA_NO_FLAC
#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE

#if defined(__ANDROID__)
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_AAUDIO
#define MA_ENABLE_OPENSL
#endif

// Cynex: ucuncu parti kod, kendi uyarilari bizim -Wall ciktimizi kirletmesin
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "../../thirdparty/miniaudio/miniaudio.h"
