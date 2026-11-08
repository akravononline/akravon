#pragma once

#include "../eterLib/NetStream.h"

class CServerStateChecker : public CSingleton<CServerStateChecker>
{
	public:
		CServerStateChecker();
		virtual ~CServerStateChecker();

		void Create(PyObject* poWnd);
		void AddChannel(UINT uServerIndex, const char* c_szAddr, UINT uPort);
		void Request();
		void Update();

		void Initialize();

	private:
		typedef struct SChannel
		{
			UINT uServerIndex;
			const char* c_szAddr;
			UINT uPort;
		} TChannel;

		PyObject* m_poWnd;

		std::list<TChannel> m_lstChannel;

		CNetworkStream m_kStream;
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
