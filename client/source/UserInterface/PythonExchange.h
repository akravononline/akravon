#pragma once
#include "Packet.h"

class CPythonExchange : public CSingleton<CPythonExchange>
{
	public:
		enum
		{
			EXCHANGE_ITEM_MAX_NUM = 24,
		};

		typedef struct trade
		{
			char					name[CHARACTER_NAME_MAX_LEN + 1];
			#ifdef ENABLE_LEVEL_IN_TRADE
			DWORD					level;
			#endif

			DWORD					item_vnum[EXCHANGE_ITEM_MAX_NUM];
			#ifdef ENABLE_NEW_MAX_COUNT
			short					item_count[EXCHANGE_ITEM_MAX_NUM];
			#else
			BYTE					item_count[EXCHANGE_ITEM_MAX_NUM];
			#endif
			DWORD					item_metin[EXCHANGE_ITEM_MAX_NUM][ITEM_SOCKET_SLOT_MAX_NUM];
			TPlayerItemAttribute	item_attr[EXCHANGE_ITEM_MAX_NUM][ITEM_ATTRIBUTE_SLOT_MAX_NUM];

			BYTE					accept;

			#ifdef ENABLE_FULL_YANG
			uint64_t				elk;
			#else
			DWORD					elk;
			#endif

			#ifdef ENABLE_EVOLUTION_SYSTEM
			int						evolution[EXCHANGE_ITEM_MAX_NUM];
			#endif

			#ifdef ENABLE_CHANGELOOK_SYSTEM
			DWORD					transmutation[EXCHANGE_ITEM_MAX_NUM];
			#endif
		} TExchangeData;

	public:
		CPythonExchange();
		virtual ~CPythonExchange();

		void			Clear();

		void			Start();
		void			End();
		bool			isTrading();

		// Interface

		void			SetSelfName(const char *name);
		void			SetTargetName(const char *name);

		char			*GetNameFromSelf();
		char			*GetNameFromTarget();

		#ifdef ENABLE_LEVEL_IN_TRADE
		void			SetSelfLevel(DWORD level);
		void			SetTargetLevel(DWORD level);

		DWORD			GetLevelFromSelf();
		DWORD			GetLevelFromTarget();
		#endif

		#ifdef ENABLE_FULL_YANG
		void			SetElkToTarget(uint64_t elk);
		void			SetElkToSelf(uint64_t elk);
		uint64_t		GetElkFromTarget();
		uint64_t		GetElkFromSelf();
		#else
		void			SetElkToTarget(DWORD elk);
		void			SetElkToSelf(DWORD elk);
		DWORD			GetElkFromTarget();
		DWORD			GetElkFromSelf();
		#endif

		#ifdef ENABLE_NEW_MAX_COUNT
		void			SetItemToTarget(DWORD pos, DWORD vnum, short count);
		void			SetItemToSelf(DWORD pos, DWORD vnum, short count);
		#else
		void			SetItemToTarget(DWORD pos, DWORD vnum, BYTE count);
		void			SetItemToSelf(DWORD pos, DWORD vnum, BYTE count);
		#endif

		void			SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum);
		void			SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum);

		void			SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue);
		void			SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue);

		void			DelItemOfTarget(BYTE pos);
		void			DelItemOfSelf(BYTE pos);

		DWORD			GetItemVnumFromTarget(BYTE pos);
		DWORD			GetItemVnumFromSelf(BYTE pos);

		#ifdef ENABLE_NEW_MAX_COUNT
		short			GetItemCountFromTarget(BYTE pos);
		short			GetItemCountFromSelf(BYTE pos);
		#else
		BYTE			GetItemCountFromTarget(BYTE pos);
		BYTE			GetItemCountFromSelf(BYTE pos);
		#endif

		DWORD			GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos);
		DWORD			GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos);

		void			GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue);
		void			GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue);

		#ifdef ENABLE_EVOLUTION_SYSTEM
		void			SetItemEvolutionToTarget(int pos, int evolution);
		void			SetItemEvolutionToSelf(int pos, int evolution);

		int				GetItemEvolutionFromTarget(int pos);
		int				GetItemEvolutionFromSelf(int pos);
		#endif

		#ifdef ENABLE_CHANGELOOK_SYSTEM
		void			SetItemTransmutation(int iPos, DWORD dwTransmutation, bool bSelf);
		DWORD			GetItemTransmutation(int iPos, bool bSelf);
		#endif

		void			SetAcceptToTarget(BYTE Accept);
		void			SetAcceptToSelf(BYTE Accept);

		bool			GetAcceptFromTarget();
		bool			GetAcceptFromSelf();

		bool			GetElkMode();
		void			SetElkMode(bool value);

		#ifdef ENABLE_EXCHANGE_RENEWAL
		void			ClearLocalItemData();
		void			AddLocalItemData(TItemPos itemPos, BYTE bDisplayPos);
		void			DelLocalItemData(TItemPos itemPos);
		bool			GetLocalItemData(TItemPos itemPos);
		#endif

	protected:
		bool				m_isTrading;

		bool				m_elk_mode;
		TExchangeData		m_self;
		TExchangeData		m_victim;

		#ifdef ENABLE_EXCHANGE_RENEWAL
		typedef std::map<TItemPos, DWORD> TExchangeLocalItemData;
		TExchangeLocalItemData	m_ExchangeLocalItemData;
		#endif
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
