// Cynex: MSS32 (Miles 6.5) yerine miniaudio ustunde cynex_mss - yalnizca MilesLib'in kullandigi kisim
#ifndef __CYNEX_MSS_H__
#define __CYNEX_MSS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AILCALL
#define AILCALL
#endif

#ifndef AILCALLBACK
#define AILCALLBACK
#endif

typedef int32_t		S32;
typedef uint32_t	U32;
typedef float		F32;
typedef char		C8;

typedef struct CynexMssDriver *		HDIGDRIVER;
typedef struct CynexMssSample *		HSAMPLE;
typedef struct CynexMssStream *		HSTREAM;
typedef struct CynexMss3DObject *	H3DPOBJECT;
typedef H3DPOBJECT					H3DSAMPLE;
typedef U32							HPROVIDER;
typedef U32							HPROENUM;
typedef S32							M3DRESULT;

#define HPROENUM_FIRST				0

#define M3D_NOERR					0
#define M3D_INVALID_PARAM			3
#define M3D_NOT_INIT				8

#define SMP_FREE					0x0001
#define SMP_DONE					0x0002
#define SMP_PLAYING					0x0004
#define SMP_STOPPED					0x0008

#define AILFILETYPE_UNKNOWN			0
#define AILFILETYPE_PCM_WAV			1
#define AILFILETYPE_ADPCM_WAV		2
#define AILFILETYPE_OTHER_WAV		3
#define AILFILETYPE_MPEG_L1_AUDIO	11
#define AILFILETYPE_MPEG_L2_AUDIO	12
#define AILFILETYPE_MPEG_L3_AUDIO	13
#define AILFILETYPE_OTHER_ASI_WAV	14

#define FILE_READ_WITH_SIZE			((void *) (intptr_t) -1)

#define AIL_FILE_SEEK_BEGIN			0
#define AIL_FILE_SEEK_CURRENT		1
#define AIL_FILE_SEEK_END			2

typedef struct CynexMssSoundInfo
{
	S32				format;
	void const *	data_ptr;
	U32				data_len;
	U32				rate;
	S32				bits;
	S32				channels;
	U32				samples;
	U32				block_size;
	void const *	initial_ptr;
} AILSOUNDINFO;

typedef U32  (AILCALLBACK * AIL_file_open_callback)(char const * filename, U32 * file_handle);
typedef void (AILCALLBACK * AIL_file_close_callback)(U32 file_handle);
typedef S32  (AILCALLBACK * AIL_file_seek_callback)(U32 file_handle, S32 offset, U32 type);
typedef U32  (AILCALLBACK * AIL_file_read_callback)(U32 file_handle, void * buffer, U32 bytes);
typedef S32  (AILCALLBACK * AILLENGTHYCB)(U32 state, U32 user);

S32			AILCALL AIL_startup(void);
void		AILCALL AIL_shutdown(void);
char *		AILCALL AIL_set_redist_directory(char const * dir);
char *		AILCALL AIL_last_error(void);

HDIGDRIVER	AILCALL AIL_open_digital_driver(U32 frequency, S32 bits, S32 channels, U32 flags);
void		AILCALL AIL_close_digital_driver(HDIGDRIVER dig);

void		AILCALL AIL_set_file_callbacks(AIL_file_open_callback opencb, AIL_file_close_callback closecb, AIL_file_seek_callback seekcb, AIL_file_read_callback readcb);
void *		AILCALL AIL_file_read(char const * filename, void * dest);
void		AILCALL AIL_mem_free_lock(void * ptr);
S32			AILCALL AIL_file_type(void const * data, U32 size);
S32			AILCALL AIL_WAV_info(void const * data, AILSOUNDINFO * info);
S32			AILCALL AIL_decompress_ADPCM(AILSOUNDINFO const * info, void ** outdata, U32 * outsize);
S32			AILCALL AIL_decompress_ASI(void const * indata, U32 insize, char const * filename_ext, void ** wav, U32 * wavsize, AILLENGTHYCB callback);

HSAMPLE		AILCALL AIL_allocate_sample_handle(HDIGDRIVER dig);
void		AILCALL AIL_release_sample_handle(HSAMPLE S);
void		AILCALL AIL_init_sample(HSAMPLE S);
S32			AILCALL AIL_set_sample_file(HSAMPLE S, void const * file_image, S32 block);
void		AILCALL AIL_start_sample(HSAMPLE S);
void		AILCALL AIL_stop_sample(HSAMPLE S);
void		AILCALL AIL_resume_sample(HSAMPLE S);
void		AILCALL AIL_end_sample(HSAMPLE S);
U32			AILCALL AIL_sample_status(HSAMPLE S);
void		AILCALL AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count);
void		AILCALL AIL_set_sample_volume_pan(HSAMPLE S, F32 volume, F32 pan);
void		AILCALL AIL_sample_volume_pan(HSAMPLE S, F32 * volume, F32 * pan);

HSTREAM		AILCALL AIL_open_stream(HDIGDRIVER dig, char const * filename, S32 stream_mem);
void		AILCALL AIL_close_stream(HSTREAM stream);
void		AILCALL AIL_start_stream(HSTREAM stream);
void		AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff);
S32			AILCALL AIL_stream_status(HSTREAM stream);
void		AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count);
void		AILCALL AIL_set_stream_volume_levels(HSTREAM S, F32 left_level, F32 right_level);
void		AILCALL AIL_stream_volume_levels(HSTREAM S, F32 * left_level, F32 * right_level);

S32			AILCALL AIL_enumerate_3D_providers(HPROENUM * next, HPROVIDER * dest, C8 ** name);
M3DRESULT	AILCALL AIL_open_3D_provider(HPROVIDER lib);
void		AILCALL AIL_close_3D_provider(HPROVIDER lib);
H3DPOBJECT	AILCALL AIL_open_3D_listener(HPROVIDER lib);
void		AILCALL AIL_close_3D_listener(H3DPOBJECT listener);

H3DSAMPLE	AILCALL AIL_allocate_3D_sample_handle(HPROVIDER lib);
void		AILCALL AIL_release_3D_sample_handle(H3DSAMPLE S);
S32			AILCALL AIL_set_3D_sample_file(H3DSAMPLE S, void const * file_image);
void		AILCALL AIL_start_3D_sample(H3DSAMPLE S);
void		AILCALL AIL_stop_3D_sample(H3DSAMPLE S);
void		AILCALL AIL_resume_3D_sample(H3DSAMPLE S);
void		AILCALL AIL_end_3D_sample(H3DSAMPLE S);
U32			AILCALL AIL_3D_sample_status(H3DSAMPLE S);
void		AILCALL AIL_set_3D_sample_loop_count(H3DSAMPLE S, U32 loops);
void		AILCALL AIL_set_3D_sample_volume(H3DSAMPLE S, F32 volume);
F32			AILCALL AIL_3D_sample_volume(H3DSAMPLE S);

void		AILCALL AIL_set_3D_position(H3DPOBJECT obj, F32 X, F32 Y, F32 Z);
void		AILCALL AIL_set_3D_velocity(H3DPOBJECT obj, F32 dX_per_ms, F32 dY_per_ms, F32 dZ_per_ms, F32 magnitude);
void		AILCALL AIL_set_3D_orientation(H3DPOBJECT obj, F32 X_face, F32 Y_face, F32 Z_face, F32 X_up, F32 Y_up, F32 Z_up);
void		AILCALL AIL_auto_update_3D_position(H3DPOBJECT obj, S32 enable);
void		AILCALL AIL_update_3D_position(H3DPOBJECT obj, F32 dt_milliseconds);

// Cynex: android yasam dongusu - onPause/onResume'dan cagrilir, her threadden guvenli
void		CynexMss_Suspend(void);
void		CynexMss_Resume(void);

#ifdef __cplusplus
}
#endif

#endif
