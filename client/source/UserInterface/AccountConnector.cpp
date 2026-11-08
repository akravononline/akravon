#include "StdAfx.h"
#include "AccountConnector.h"
#ifdef ENABLE_RASCAL_ANTICHEAT
#include "rascal.h"
#endif

#include "Packet.h"
#include "PythonNetworkStream.h"
#include "../EterBase/tea.h"
#include "../EterPack/EterPackManager.h"

// CHINA_CRYPT_KEY
extern DWORD g_adwEncryptKey[4];
extern DWORD g_adwDecryptKey[4];
extern bool g_adwSecurityMode;
// END_OF_CHINA_CRYPT_KEY

#ifdef ENABLE_VERSION_SYSTEM
	#define VERSIYON_CODE			18
#endif

#ifdef RENEWAL_DISCORD_BLOCK
#include "discord_rpc.h"
#if defined( _DEBUG ) || defined( DEBUG )
#define _RUNTIME_DEBUG				"_d"
#else
#define _RUNTIME_DEBUG				""
#endif
#pragma comment( lib, "EterDiscordLib" _RUNTIME_DEBUG ".lib" )
#endif

void CAccountConnector::SetHandler(PyObject* poHandler)
{
	m_poHandler = poHandler;
}

void CAccountConnector::SetLoginInfo(const char * c_szName, const char * c_szPwd
	#ifdef ENABLE_SERVER_PIN
	, const char * c_szPin
	#endif
	#ifdef ENABLE_PCBAN_SYSTEM
	, int c_iBan
	#endif
)
{
	m_strID = c_szName;
	m_strPassword = c_szPwd;
	#ifdef ENABLE_SERVER_PIN
	m_strPin = c_szPin;
	#endif
	#ifdef ENABLE_PCBAN_SYSTEM
	m_intBan = c_iBan;
	#endif
}


void CAccountConnector::ClearLoginInfo( void )
{
	m_strPassword = "";
	#ifdef ENABLE_SERVER_PIN
	m_strPin = "";
	#endif
	#ifdef ENABLE_PCBAN_SYSTEM
	m_intBan = 0;
	#endif
}

bool CAccountConnector::Connect(const char * c_szAddr, int iPort, const char * c_szAccountAddr, int iAccountPort)
{
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	__BuildClientKey();
	#endif

	m_strAddr = c_szAddr;
	m_iPort = iPort;

	__OfflineState_Set();

	// CHINA_CRYPT_KEY
	if (LocaleService_IsYMIR())
	{
	}
	else
	{
		__BuildClientKey_20050304Myevan();
	}
	// END_OF_CHINA_CRYPT_KEY

	return CNetworkStream::Connect(c_szAccountAddr, iAccountPort);
}

void CAccountConnector::Disconnect()
{
	CNetworkStream::Disconnect();
	__OfflineState_Set();
}

void CAccountConnector::Process()
{
	CNetworkStream::Process();

	if (!__StateProcess())
	{
		__OfflineState_Set();
		Disconnect();
	}
}

bool CAccountConnector::__StateProcess()
{
	switch (m_eState)
	{
		case STATE_HANDSHAKE:
			return __HandshakeState_Process();
			break;
		case STATE_AUTH:
			return __AuthState_Process();
			break;
	}

	return true;
}

bool CAccountConnector::__HandshakeState_Process()
{
	if (!__AnalyzePacket(HEADER_GC_PHASE, sizeof(TPacketGCPhase), &CAccountConnector::__AuthState_RecvPhase))
		return false;

	if (!__AnalyzePacket(HEADER_GC_HANDSHAKE, sizeof(TPacketGCHandshake), &CAccountConnector::__AuthState_RecvHandshake))
		return false;

	if (!__AnalyzePacket(HEADER_GC_PING, sizeof(TPacketGCPing), &CAccountConnector::__AuthState_RecvPing))
		return false;

	if (!__AnalyzeVarSizePacket(HEADER_GC_HYBRIDCRYPT_KEYS, &CAccountConnector::__AuthState_RecvHybridCryptKeys))
		return false;

	if (!__AnalyzeVarSizePacket(HEADER_GC_HYBRIDCRYPT_SDB, &CAccountConnector::__AuthState_RecvHybridCryptSDB))
		return false;

	#ifdef _IMPROVED_PACKET_ENCRYPTION_
	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT, sizeof(TPacketKeyAgreement), &CAccountConnector::__AuthState_RecvKeyAgreement))
		return false;

	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT_COMPLETED, sizeof(TPacketKeyAgreementCompleted), &CAccountConnector::__AuthState_RecvKeyAgreementCompleted))
		return false;
	#endif

	return true;
}

bool CAccountConnector::__AuthState_Process()
{
	if (!__AnalyzePacket(0, sizeof(BYTE), &CAccountConnector::__AuthState_RecvEmpty))
		return true;

	if (!__AnalyzePacket(HEADER_GC_PHASE, sizeof(TPacketGCPhase), &CAccountConnector::__AuthState_RecvPhase))
		return false;

	if (!__AnalyzePacket(HEADER_GC_PING, sizeof(TPacketGCPing), &CAccountConnector::__AuthState_RecvPing))
		return false;

	if (!__AnalyzePacket(HEADER_GC_AUTH_SUCCESS, sizeof(TPacketGCAuthSuccess), &CAccountConnector::__AuthState_RecvAuthSuccess))
		return true;

	if (!__AnalyzePacket(HEADER_GC_LOGIN_FAILURE, sizeof(TPacketGCAuthSuccess), &CAccountConnector::__AuthState_RecvAuthFailure))
		return true;

	if (!__AnalyzePacket(HEADER_GC_HANDSHAKE, sizeof(TPacketGCHandshake), &CAccountConnector::__AuthState_RecvHandshake))
		return false;

	if (!__AnalyzePacket(HEADER_GC_PANAMA_PACK, sizeof(TPacketGCPanamaPack), &CAccountConnector::__AuthState_RecvPanamaPack))
		return false;

	#ifdef _IMPROVED_PACKET_ENCRYPTION_
	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT, sizeof(TPacketKeyAgreement), &CAccountConnector::__AuthState_RecvKeyAgreement))
		return false;

	if (!__AnalyzePacket(HEADER_GC_KEY_AGREEMENT_COMPLETED, sizeof(TPacketKeyAgreementCompleted), &CAccountConnector::__AuthState_RecvKeyAgreementCompleted))
		return false;
	#endif

	if (!__AnalyzeVarSizePacket(HEADER_GC_HYBRIDCRYPT_KEYS, &CAccountConnector::__AuthState_RecvHybridCryptKeys))
		return false;

	if (!__AnalyzeVarSizePacket(HEADER_GC_HYBRIDCRYPT_SDB, &CAccountConnector::__AuthState_RecvHybridCryptSDB))
		return false;

	return true;
}

bool CAccountConnector::__AuthState_RecvEmpty()
{
	BYTE byEmpty;
	Recv(sizeof(BYTE), &byEmpty);
	return true;
}

#ifdef RENEWAL_HWID_BLOCK
#pragma comment(lib, "Iphlpapi.lib")
#include <iphlpapi.h>
const char* GetPwd()
{
	static char buf[124];
	snprintf(buf, sizeof(buf), "FATAL-ERROR");
	DWORD dwSize = 0;
	DWORD dwRetVal = GetIpNetTable(NULL, &dwSize, FALSE);
	if (dwRetVal == ERROR_INSUFFICIENT_BUFFER)
	{
		MIB_IPNETTABLE* pIpNetTable = (MIB_IPNETTABLE*)malloc(dwSize);
		if (pIpNetTable == NULL)
			return buf;
		dwRetVal = GetIpNetTable(pIpNetTable, &dwSize, FALSE);
		if (dwRetVal == NO_ERROR)
		{
			for (DWORD i = 0; i < pIpNetTable->dwNumEntries; i++) {
				MIB_IPNETROW row = pIpNetTable->table[i];
				if (row.dwType == 3)
				{
					snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", row.bPhysAddr[0], row.bPhysAddr[1], row.bPhysAddr[2],row.bPhysAddr[3], row.bPhysAddr[4], row.bPhysAddr[5]);
					if (strcmp(buf, "00:00:00:00:00:00") != 0)
						break;
				}
			}
		}
		free(pIpNetTable);
	}
	return buf;
}
#endif

#ifdef RENEWAL_DISCORD_BLOCK
static const std::string DiscordInfo()
{
	std::string dcname = Discord_GetName();
	std::string dckey = Discord_GetKey();

	std::string info = dcname + "#" + dckey;

	return info;
}

static const std::string DiscordGetID()
{
	return Discord_GetID();
}
#endif

bool CAccountConnector::__AuthState_RecvPhase()
{
	TPacketGCPhase kPacketPhase;
	if (!Recv(sizeof(kPacketPhase), &kPacketPhase))
		return false;

	if (kPacketPhase.phase == PHASE_HANDSHAKE)
	{
		__HandshakeState_Set();
	}
	else if (kPacketPhase.phase == PHASE_AUTH)
	{
#ifndef _IMPROVED_PACKET_ENCRYPTION_
		const char* key = LocaleService_GetSecurityKey();
		SetSecurityMode(true, key);
		#endif

		TPacketCGLogin3 LoginPacket;
		LoginPacket.header = HEADER_CG_LOGIN3;

		strncpy(LoginPacket.name, m_strID.c_str(), ID_MAX_NUM);
#ifdef ENABLE_RASCAL_ANTICHEAT
		if(!rascal::CheckRascalExports(&rascal::FunctionProtect)) {
			__fastfail(0);
			exit(0);
			std::terminate();
		}
		if(!rascal::CheckRascalExports(&rascal::GetRascalThreadID)) {
			__fastfail(0);
			exit(0);
			std::terminate();
		}
		if(!rascal::CheckRascalExports(&rascal::RascalProtectRTypes)) {
			__fastfail(0);
			exit(0);
			std::terminate();
		}
		if(!rascal::CheckRascalExports(&rascal::InitializeRascal)) {
			__fastfail(0);
			exit(0);
			std::terminate();
		}

		rascal::ListModulesForTheVMPSections();

		rascal::RascalProtectRTypes(GETUSERNAME,&m_strID,NULL);
		int anticheatheartbeatvalue = 0;
		rascal::RascalProtectRTypes(NOTHING,&anticheatheartbeatvalue,NULL);
		int cryptkey = 1337;
		anticheatheartbeatvalue = anticheatheartbeatvalue ^ cryptkey;
		if (anticheatheartbeatvalue < 67 && anticheatheartbeatvalue > 56) {
        	}
        	else {
            	rascal::WLog(std::string(skCrypt("anticheatheartbeatvalue error")));
            	rascal::WLog(std::to_string(anticheatheartbeatvalue));
            	__fastfail(0);
            	exit(1);
        	}
#endif

		strncpy(LoginPacket.pwd, m_strPassword.c_str(), PASS_MAX_NUM);
		#ifdef ENABLE_SERVER_PIN
		strncpy(LoginPacket.pin, m_strPin.c_str(), PIN_MAX_NUM);
		#endif
		#ifdef ENABLE_PCBAN_SYSTEM
		LoginPacket.pcban = m_intBan;
		#endif

		LoginPacket.name[ID_MAX_NUM] = '\0';
		LoginPacket.pwd[PASS_MAX_NUM] = '\0';
		#ifdef ENABLE_SERVER_PIN
		LoginPacket.pin[PIN_MAX_NUM] = '\0';
		#endif
		#ifdef ENABLE_VERSION_SYSTEM
		LoginPacket.version = VERSIYON_CODE;
		#endif
		ClearLoginInfo();
		CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
		rkNetStream.ClearLoginInfo();

		m_strPassword = "";
		#ifdef ENABLE_SERVER_PIN
		m_strPin = "";
		#endif
		#ifdef ENABLE_PCBAN_SYSTEM
		m_intBan = 0;
		#endif

		for (DWORD i = 0; i < 4; ++i)
			LoginPacket.adwClientKey[i] = g_adwEncryptKey[i];

		#ifdef RENEWAL_DISCORD_BLOCK
		strcpy(LoginPacket.discord_nick, DiscordInfo().c_str());
		LoginPacket.discord_nick[128] = '\0';

		strcpy(LoginPacket.discord_id, DiscordGetID().c_str());
		LoginPacket.discord_id[128] = '\0';
		#endif

		#ifdef RENEWAL_HWID_BLOCK
		strcpy(LoginPacket.hwid_router, GetPwd());
		LoginPacket.hwid_router[18] = '\0';
		#endif

		if (!Send(sizeof(LoginPacket), &LoginPacket))
		{
			Tracen(" CAccountConnector::__AuthState_RecvPhase - SendLogin3 Error");
			return false;
		}

		if (!SendSequence())
		{
			return false;
		}

		__AuthState_Set();
	}

	return true;
}

bool CAccountConnector::__AuthState_RecvHandshake()
{
	TPacketGCHandshake kPacketHandshake;
	if (!Recv(sizeof(kPacketHandshake), &kPacketHandshake))
		return false;

	// HandShake
	{
		Tracenf("HANDSHAKE RECV %u %d", kPacketHandshake.dwTime, kPacketHandshake.lDelta);

		ELTimer_SetServerMSec(kPacketHandshake.dwTime+ kPacketHandshake.lDelta);

		//DWORD dwBaseServerTime = kPacketHandshake.dwTime+ kPacketHandshake.lDelta;
		//DWORD dwBaseClientTime = ELTimer_GetMSec();

		kPacketHandshake.dwTime = kPacketHandshake.dwTime + kPacketHandshake.lDelta + kPacketHandshake.lDelta;
		kPacketHandshake.lDelta = 0;

		Tracenf("HANDSHAKE SEND %u", kPacketHandshake.dwTime);

		if (!Send(sizeof(kPacketHandshake), &kPacketHandshake))
		{
			Tracen(" CAccountConnector::__AuthState_RecvHandshake - SendHandshake Error");
			return false;
		}
	}

	return true;
}

bool CAccountConnector::__AuthState_RecvPanamaPack()
{
	TPacketGCPanamaPack kPacket;

	if (!Recv(sizeof(TPacketGCPanamaPack), &kPacket))
		return false;

	CEterPackManager::instance().RegisterPack(kPacket.szPackName, "*", kPacket.abIV);
	return true;
}

bool CAccountConnector::__AuthState_RecvHybridCryptKeys(int iTotalSize)
{
	int iFixedHeaderSize = TPacketGCHybridCryptKeys::GetFixedHeaderSize();

	TPacketGCHybridCryptKeys kPacket(iTotalSize-iFixedHeaderSize);

	if (!Recv(iFixedHeaderSize, &kPacket))
		return false;

	if (!Recv(kPacket.iKeyStreamLen, kPacket.m_pStream))
		return false;

	CEterPackManager::Instance().RetrieveHybridCryptPackKeys( kPacket.m_pStream );
	return true;
}

bool CAccountConnector::__AuthState_RecvHybridCryptSDB(int iTotalSize)
{
	int iFixedHeaderSize = TPacketGCHybridSDB::GetFixedHeaderSize();

	TPacketGCHybridSDB kPacket(iTotalSize-iFixedHeaderSize);

	if (!Recv(iFixedHeaderSize, &kPacket))
		return false;

	if (!Recv(kPacket.iSDBStreamLen, kPacket.m_pStream))
		return false;

	CEterPackManager::Instance().RetrieveHybridCryptPackSDB( kPacket.m_pStream );
	return true;
}


bool CAccountConnector::__AuthState_RecvPing()
{
	TPacketGCPing kPacketPing;
	if (!Recv(sizeof(kPacketPing), &kPacketPing))
		return false;

	__AuthState_SendPong();

	return true;
}

bool CAccountConnector::__AuthState_SendPong()
{
	TPacketCGPong kPacketPong;
	kPacketPong.bHeader = HEADER_CG_PONG;
	if (!Send(sizeof(kPacketPong), &kPacketPong))
		return false;

	if (IsSecurityMode())
		return SendSequence();

	return true;
}

bool CAccountConnector::__AuthState_RecvAuthSuccess()
{
	TPacketGCAuthSuccess kAuthSuccessPacket;
	if (!Recv(sizeof(kAuthSuccessPacket), &kAuthSuccessPacket))
		return false;

#ifdef __APPLE__
	fprintf(stderr, "[T] Auth sonucu bResult=%d key=%u kanal=%s:%d\n", (int)kAuthSuccessPacket.bResult, (unsigned)kAuthSuccessPacket.dwLoginKey, m_strAddr.c_str(), m_iPort); fflush(stderr);
#endif
	if (!kAuthSuccessPacket.bResult)
	{
		if (m_poHandler)
			PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(s)", "BESAMEKEY"));
	}
	else
	{
		DWORD dwPanamaKey = kAuthSuccessPacket.dwLoginKey ^ g_adwEncryptKey[0] ^ g_adwEncryptKey[1] ^ g_adwEncryptKey[2] ^ g_adwEncryptKey[3];
		CEterPackManager::instance().DecryptPackIV(dwPanamaKey);

		CPythonNetworkStream & rkNet = CPythonNetworkStream::Instance();
		rkNet.SetLoginKey(kAuthSuccessPacket.dwLoginKey);
		rkNet.Connect(m_strAddr.c_str(), m_iPort);
	}

	Disconnect();
	__OfflineState_Set();

	return true;
}

bool CAccountConnector::__AuthState_RecvAuthFailure()
{
	TPacketGCLoginFailure packet_failure;
	if (!Recv(sizeof(TPacketGCLoginFailure), &packet_failure))
		return false;

	if (m_poHandler)
		PyCallClassMemberFunc(m_poHandler, "OnLoginFailure", Py_BuildValue("(s)", packet_failure.szStatus));

//	__OfflineState_Set();

	return true;
}

#ifdef _IMPROVED_PACKET_ENCRYPTION_
bool CAccountConnector::__AuthState_RecvKeyAgreement()
{
	TPacketKeyAgreement packet;
	if (!Recv(sizeof(packet), &packet))
	{
		return false;
	}

	Tracenf("KEY_AGREEMENT RECV %u", packet.wDataLength);

	TPacketKeyAgreement packetToSend;
	size_t dataLength = TPacketKeyAgreement::MAX_DATA_LEN;
	size_t agreedLength = Prepare(packetToSend.data, &dataLength);
	if (agreedLength == 0)
	{
		Disconnect();
		return false;
	}
	assert(dataLength <= TPacketKeyAgreement::MAX_DATA_LEN);

	if (Activate(packet.wAgreedLength, packet.data, packet.wDataLength))
	{
		packetToSend.bHeader = HEADER_CG_KEY_AGREEMENT;
		packetToSend.wAgreedLength = (WORD)agreedLength;
		packetToSend.wDataLength = (WORD)dataLength;

		if (!Send(sizeof(packetToSend), &packetToSend))
		{
			Tracen(" CAccountConnector::__AuthState_RecvKeyAgreement - SendKeyAgreement Error");
			return false;
		}
		Tracenf("KEY_AGREEMENT SEND %u", packetToSend.wDataLength);
	}
	else
	{
		Disconnect();
		return false;
	}
	return true;
}

bool CAccountConnector::__AuthState_RecvKeyAgreementCompleted()
{
	TPacketKeyAgreementCompleted packet;
	if (!Recv(sizeof(packet), &packet))
	{
		return false;
	}

	Tracenf("KEY_AGREEMENT_COMPLETED RECV");

	ActivateCipher();

	return true;
}
#endif // _IMPROVED_PACKET_ENCRYPTION_

bool CAccountConnector::__AnalyzePacket(UINT uHeader, UINT uPacketSize, bool (CAccountConnector::*pfnDispatchPacket)())
{
	BYTE bHeader;
	if (!Peek(sizeof(bHeader), &bHeader))
		return true;

	if (bHeader!=uHeader)
		return true;

	if (!PeekNoFetch(uPacketSize))
		return true;

	return (this->*pfnDispatchPacket)();
}

bool CAccountConnector::__AnalyzeVarSizePacket(UINT uHeader, bool (CAccountConnector::*pfnDispatchPacket)(int))
{
	BYTE bHeader;
	if (!Peek(sizeof(bHeader), &bHeader))
		return true;

	if (bHeader!=uHeader)
		return true;

	TDynamicSizePacketHeader dynamicHeader;

	if (!Peek(sizeof(dynamicHeader), &dynamicHeader))
		return true;

	if (!PeekNoFetch(dynamicHeader.size))
		return true;

	return (this->*pfnDispatchPacket)(dynamicHeader.size);
}


void CAccountConnector::__OfflineState_Set()
{
	__Inialize();
}

void CAccountConnector::__HandshakeState_Set()
{
	m_eState=STATE_HANDSHAKE;
}

void CAccountConnector::__AuthState_Set()
{
	m_eState=STATE_AUTH;
}

void CAccountConnector::OnConnectFailure()
{
#ifdef __APPLE__
	fprintf(stderr, "[T] AccountConnector::OnConnectFailure durum=%d\n", (int)m_eState); fflush(stderr);
#endif
	if (m_poHandler)
		PyCallClassMemberFunc(m_poHandler, "OnConnectFailure", Py_BuildValue("()"));

	__OfflineState_Set();
}

void CAccountConnector::OnConnectSuccess()
{
	m_eState = STATE_HANDSHAKE;
}

void CAccountConnector::OnRemoteDisconnect()
{
#ifdef __APPLE__
	fprintf(stderr, "[T] AccountConnector::OnRemoteDisconnect durum=%d\n", (int)m_eState); fflush(stderr);
#endif
	if (m_isWaitKey)
	{
		if (m_poHandler)
		{
			PyCallClassMemberFunc(m_poHandler, "OnExit", Py_BuildValue("()"));
			return;
		}
	}

	__OfflineState_Set();
}

void CAccountConnector::OnDisconnect()
{
#ifdef __APPLE__
	fprintf(stderr, "[T] AccountConnector::OnDisconnect durum=%d\n", (int)m_eState); fflush(stderr);
#endif
	__OfflineState_Set();
}

#ifndef _IMPROVED_PACKET_ENCRYPTION_
void CAccountConnector::__BuildClientKey()
{
	for (DWORD i = 0; i < 4; ++i)
		g_adwEncryptKey[i] = random();

	const BYTE * c_pszKey = (const BYTE *) "JyTxtHljHJlVJHorRM301vf@4fvj10-v";
	tea_encrypt((DWORD *) g_adwDecryptKey, (const DWORD *) g_adwEncryptKey, (const DWORD *) c_pszKey, 16);
}
#endif

void CAccountConnector::__Inialize()
{
	m_eState=STATE_OFFLINE;
	m_isWaitKey = FALSE;
}

CAccountConnector::CAccountConnector()
{
	m_poHandler = NULL;
	m_strAddr = "";
	m_iPort = 0;

	SetLoginInfo("", ""
		#ifdef ENABLE_SERVER_PIN
		, ""
		#endif
		#ifdef ENABLE_PCBAN_SYSTEM
		, 0
		#endif
	);

	SetRecvBufferSize(1024 * 128);
	SetSendBufferSize(2048);
	__Inialize();
}

CAccountConnector::~CAccountConnector()
{
	__OfflineState_Set();
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240

