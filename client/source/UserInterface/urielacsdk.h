#pragma once

#include <intrin.h>
#include <stdint.h>
#include <unordered_map>
#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/secblock.h>
#include <cryptopp/modes.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/arc4.h>

#pragma comment(linker, "/ALIGN:0x10000")

#ifdef _WIN64
#pragma comment(lib, "client_x64.lib")
#else
#pragma comment(lib, "client_x86.lib")
#endif

#pragma pack(push, 1)
typedef struct PacketUrielHeartBeat
{
	BYTE	bHeader;
	BYTE	bPacketData[64];
} TPacketUrielHeartBeat;

typedef struct PacketUrielHeartBeatResponse
{
	BYTE	bHeader;
	BYTE	bPacketData[32];
} TPacketUrielHeartBeatResponse;
#pragma pack(pop)

template<class T>
class safe_variable_weak
{
public:

	__forceinline safe_variable_weak()
	{
		key1 = __rdtsc() ^ 0x4FE74C4893A6AC4B;
		key2 = __rdtsc() ^ 0x5784FF3C4E8F9F9D;
		key3 = __rdtsc() ^ 0x7D7DDB54C4CD4DBE;

		key2 ^= key1 + key3;
		key1 ^= _byteswap_uint64(key3);
	}

	__forceinline safe_variable_weak(T value)
	{
		key1 = __rdtsc() ^ 0x4FE74C4893A6AC4B;
		key2 = __rdtsc() ^ 0x5784FF3C4E8F9F9D;
		key3 = __rdtsc() ^ 0x7D7DDB54C4CD4DBE;

		key2 ^= key1 + key3;
		key1 ^= _byteswap_uint64(key3);
		encrypt(value);
	}

	__forceinline safe_variable_weak<T>& operator=(T value)
	{
		encrypt(value);
		return *this;
	}

	__forceinline operator T()
	{
		return decrypt();
	}

	__forceinline operator T() const
	{
		return decrypt();
	}

	__forceinline safe_variable_weak<T>& operator +=(const T& other)
	{
		encrypt(get() + other);
		return *this;
	}

	__forceinline safe_variable_weak<T>& operator ++()
	{
		encrypt(get() + 1);
		return *this;
	}

	__forceinline safe_variable_weak<T>& operator -=(const T& other)
	{
		encrypt(get() - other);
		return *this;
	}

	__forceinline safe_variable_weak<T>& operator /=(const T& other)
	{
		encrypt(get() / other);
		return *this;
	}

	__forceinline safe_variable_weak<T>& operator *=(const T& other)
	{
		encrypt(get() * other);
		return *this;
	}

	__forceinline T get()
	{
		return decrypt();
	}

private:
	uint64_t _data;
	uint64_t key1;
	uint64_t key2;
	uint64_t key3;

	__forceinline void encrypt(T data)
	{
		uint64_t p = 0;
		memcpy(&p, &data, sizeof(T));

		p = _byteswap_uint64(p);

		switch (key1 % 5)
		{
		case 0:
		{
			p ^= key1;
			p += 0xADAEBD824E8C54C3;
			p ^= key3;
			p -= 0xDD939937BD35B57E;
			p += key1 ^ key2 ^ key3;
			break;
		}
		case 1:
		{
			p += 0x9C24829ACC33FA5A;
			p ^= 0x742FF29F27D4ABEF;
			p ^= key2;
			p += key3;
			p -= key2 ^ key3;
			break;
		}
		case 2:
		{
			p ^= key3;
			p -= key1 ^ key2;
			p += 0xAF3FD6C3CBC6555F;
			p ^= key2;
			p -= 0xCB7FAD627B2599BF;
			break;
		}
		case 3:
		{
			p = _rotr64(p, (int)(((key1 ^ key2) + (0x9EE5773284F3373B)) % 64));
			p ^= key3;
			p += 0x9C84FF9A444A9368;
			p ^= key1 + key2;
			p -= 0x3AD984CAAAD93F68;
			break;
		}
		case 4:
		{
			p ^= 0x2B5CCE3AFAC53367;
			p -= 0x36F4DDD737B73E89;
			p ^= 0xA643D783BF68D762;
			p += 0x42A569758982EEAD;
			p = _rotl64(p, (int)((((key3 + key2) ^ 0x7634528B7E35CE67) + 0x2B46BE6C49EA26A4) % 64));
			break;
		}
		}

		p = _byteswap_uint64(p);

		_data = p;
	}

	__forceinline T decrypt() const
	{
		uint64_t p = _data;

		p = _byteswap_uint64(p);

		switch (key1 % 5)
		{
		case 0:
		{
			p -= key1 ^ key2 ^ key3;
			p += 0xDD939937BD35B57E;
			p ^= key3;
			p -= 0xADAEBD824E8C54C3;
			p ^= key1;
			break;
		}
		case 1:
		{
			p += key2 ^ key3;
			p -= key3;
			p ^= key2;
			p ^= 0x742FF29F27D4ABEF;
			p -= 0x9C24829ACC33FA5A;
			break;
		}
		case 2:
		{
			p += 0xCB7FAD627B2599BF;
			p ^= key2;
			p -= 0xAF3FD6C3CBC6555F;
			p += key1 ^ key2;
			p ^= key3;
			break;
		}
		case 3:
		{
			p += 0x3AD984CAAAD93F68;
			p ^= key1 + key2;
			p -= 0x9C84FF9A444A9368;
			p ^= key3;
			p = _rotl64(p, (int)(((key1 ^ key2) + (0x9EE5773284F3373B)) % 64));
			break;
		}
		case 4:
		{
			p = _rotr64(p, (int)((((key3 + key2) ^ 0x7634528B7E35CE67) + 0x2B46BE6C49EA26A4) % 64));
			p -= 0x42A569758982EEAD;
			p ^= 0xA643D783BF68D762;
			p += 0x36F4DDD737B73E89;
			p ^= 0x2B5CCE3AFAC53367;
			break;
		}
		}

		p = _byteswap_uint64(p);

		T t{};
		memcpy(&t, &p, sizeof(T));

		return t;
	}

	__forceinline T decrypt()
	{
		uint64_t p = _data;

		p = _byteswap_uint64(p);

		switch (key1 % 5)
		{
		case 0:
		{
			p -= key1 ^ key2 ^ key3;
			p += 0xDD939937BD35B57E;
			p ^= key3;
			p -= 0xADAEBD824E8C54C3;
			p ^= key1;
			break;
		}
		case 1:
		{
			p += key2 ^ key3;
			p -= key3;
			p ^= key2;
			p ^= 0x742FF29F27D4ABEF;
			p -= 0x9C24829ACC33FA5A;
			break;
		}
		case 2:
		{
			p += 0xCB7FAD627B2599BF;
			p ^= key2;
			p -= 0xAF3FD6C3CBC6555F;
			p += key1 ^ key2;
			p ^= key3;
			break;
		}
		case 3:
		{
			p += 0x3AD984CAAAD93F68;
			p ^= key1 + key2;
			p -= 0x9C84FF9A444A9368;
			p ^= key3;
			p = _rotl64(p, (int)(((key1 ^ key2) + (0x9EE5773284F3373B)) % 64));
			break;
		}
		case 4:
		{
			p = _rotr64(p, (int)((((key3 + key2) ^ 0x7634528B7E35CE67) + 0x2B46BE6C49EA26A4) % 64));
			p -= 0x42A569758982EEAD;
			p ^= 0xA643D783BF68D762;
			p += 0x36F4DDD737B73E89;
			p ^= 0x2B5CCE3AFAC53367;
			break;
		}
		}

		p = _byteswap_uint64(p);

		T t{};
		memcpy(&t, &p, sizeof(T));

		return t;
	}
};

class UrielAntiCheat
{
public:
	virtual bool Initialize(int port, int header);
	virtual std::vector<uint8_t> Tick(const std::vector<uint8_t>& randomData, uintptr_t* returnAddress);
	virtual bool GetLoginToken(std::string username, char* token_buff);
	virtual bool Heartbeat(char* data);
	virtual bool GetAttackHash(DWORD dwVID, DWORD dwOrder, unsigned int attackSpeed, DWORD* lpRandom, unsigned char* verifyHash);
	virtual void CheckReturnAddress(void* ReturnAddress);

	__forceinline void SetGameSendPacketPointer(void* ptr)
	{
		p_GameSendPacket = ptr;
	}

	__forceinline void GameSendPacket(const char* buff, size_t size)
	{
		return reinterpret_cast<void(*)(const char*, size_t)>(p_GameSendPacket)(buff, size);
	}

	static void SendPacket(const char* buff, size_t size);

	__forceinline static std::unordered_map<DWORD, unsigned int>& GetAttackOrderMap()
	{
		return m_attackOrder;
	}

private:
	void* p_GameSendPacket;
	static std::unordered_map<DWORD, unsigned int> m_attackOrder;
};


extern "C" __declspec(dllimport)
bool FireInTheHole(UrielAntiCheat** SDKPointer);
