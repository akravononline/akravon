#include "stdafx.h"
#include "PythonNetworkStream.h"

#ifdef ENABLE_URIEL_ANTI_CHAT
void UrielAntiCheat::SendPacket(const char* buff, size_t size)
{
	CPythonNetworkStream::Instance().Send(size, buff);
}
#endif
