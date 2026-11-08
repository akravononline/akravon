// Cynex: yalnizca CYNEX_MSS_TESTING ile derlenen test kancalari (cynex_mss_testing hedefi)
#pragma once

#include <mss.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	// Cynex: OFFLINE'da cihaz ve servis threadi yok, miks CynexMssTest_Render ile elle
	CYNEX_MSS_TEST_DEVICE = 0,
	CYNEX_MSS_TEST_OFFLINE = 1,
	CYNEX_MSS_TEST_NULL_BACKEND = 2,
};

void	CynexMssTest_SetMode(int mode, U32 offlineRate);
void	CynexMssTest_Render(float * out, U32 frames);
void	CynexMssTest_ServiceStreams(void);
U32		CynexMssTest_MixerRate(void);
int		CynexMssTest_DeviceRunning(void);
U32		CynexMssTest_StreamUnderruns(void);

#ifdef __cplusplus
}
#endif
