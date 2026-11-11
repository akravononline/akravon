#pragma once

#include "../eterBase/Singleton.h"


class CPythonTextTail : public CSingleton<CPythonTextTail>
{
	public:
		typedef struct STextTail
		{
			CGraphicTextInstance*			pTextInstance;
			CGraphicTextInstance*			pOwnerTextInstance;

			CGraphicMarkInstance*			pMarkInstance;
			CGraphicTextInstance*			pGuildNameTextInstance;

			CGraphicTextInstance*			pTitleTextInstance;
			CGraphicTextInstance*			pLevelTextInstance;


			CGraphicObjectInstance *		pOwner;

			DWORD							dwVirtualID;

			float							x, y, z;
			float							fDistanceFromPlayer;
			D3DXCOLOR						Color;
			BOOL							bNameFlag;

			float							xStart, yStart;
			float							xEnd, yEnd;

			DWORD							LivingTime;

			float							fHeight;

#if defined(ENABLE_IKASHOP_RENEWAL) && defined(ENABLE_IKASHOP_ENTITIES)
			bool  bIsShop;
			bool  bRender;
#endif

			// AKRAVON: isim yaninda diger krallik bayragi ikonu (isiltili, nefes alan)
			CGraphicImageInstance *			pEmpireFlag;
			CGraphicImageInstance *			pEmpireGlow;
			int								iEmpire;
			bool							bEmpireShow;
			float							fEmpireX, fEmpireY, fEmpireScale;

			STextTail() : pEmpireFlag(NULL), pEmpireGlow(NULL), iEmpire(0), bEmpireShow(false), fEmpireX(0.0f), fEmpireY(0.0f), fEmpireScale(1.0f) {}
			virtual ~STextTail() {}
		} TTextTail;

		typedef std::map<DWORD, TTextTail*>		TTextTailMap;
		typedef std::list<TTextTail*>			TTextTailList;
		typedef TTextTailMap					TChatTailMap;

	public:
		CPythonTextTail(void);
		virtual ~CPythonTextTail(void);

		void GetInfo(std::string* pstInfo);

		void Initialize();
		void Destroy();
		void Clear();

		void UpdateAllTextTail();
		void UpdateShowingTextTail();
		void Render();

		void ArrangeTextTail();
		void HideAllTextTail();
		void ShowAllTextTail();
		void ShowCharacterTextTail(DWORD VirtualID);
		void ShowItemTextTail(DWORD VirtualID);

		#if defined(ENABLE_IKASHOP_RENEWAL) && defined(ENABLE_IKASHOP_ENTITIES)
		void RegisterShopInstanceTextTail(DWORD dwVirtualID, const char* c_szName, CGraphicObjectInstance* pOwner);
		void DeleteShopTextTail(DWORD VirtualID);
		TTextTail* RegisterShopTextTail(DWORD dwVirtualID, const char* c_szText, CGraphicObjectInstance* pOwner);
		bool GetPickedNewShop(DWORD* pdwVID);
		#endif

		void RegisterCharacterTextTail(DWORD dwGuildID, DWORD dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight=10.0f);
		void RegisterItemTextTail(DWORD VirtualID, const char * c_szText, CGraphicObjectInstance * pOwner);
		void RegisterChatTail(DWORD VirtualID, const char * c_szChat);
		void RegisterInfoTail(DWORD VirtualID, const char * c_szChat);
		void SetCharacterTextTailColor(DWORD VirtualID, const D3DXCOLOR & c_rColor);
		void SetItemTextTailOwner(DWORD dwVID, const char * c_szName);
		void DeleteCharacterTextTail(DWORD VirtualID);
		void DeleteItemTextTail(DWORD VirtualID);

		int Pick(int ixMouse, int iyMouse);
		void SelectItemName(DWORD dwVirtualID);

		bool GetTextTailPosition(DWORD dwVID, float* px, float* py, float* pz);
		bool IsChatTextTail(DWORD dwVID);

		void EnablePKTitle(BOOL bFlag);
		void AttachTitle(DWORD dwVID, const char * c_szName, const D3DXCOLOR& c_rColor);
		void DetachTitle(DWORD dwVID);

		void AttachLevel(DWORD dwVID, const char* c_szText, const D3DXCOLOR& c_rColor);
		void DetachLevel(DWORD dwVID);


	protected:
		TTextTail * RegisterTextTail(DWORD dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, float fHeight, const D3DXCOLOR & c_rColor);
		void DeleteTextTail(TTextTail * pTextTail);

		void UpdateTextTail(TTextTail * pTextTail);
		void RenderTextTailBox(TTextTail * pTextTail);
		void RenderTextTailName(TTextTail * pTextTail);
		void UpdateDistance(const TPixelPosition & c_rCenterPosition, TTextTail * pTextTail);

		bool isIn(TTextTail * pSource, TTextTail * pTarget);

	protected:
		TTextTailMap				m_CharacterTextTailMap;
		TTextTailMap				m_ItemTextTailMap;
		TChatTailMap				m_ChatTailMap;

		TTextTailList				m_CharacterTextTailList;
		TTextTailList				m_ItemTextTailList;
#if defined(ENABLE_IKASHOP_RENEWAL) && defined(ENABLE_IKASHOP_ENTITIES)
		TTextTailMap m_ShopTextTailMap;
#endif

	private:
		CDynamicPool<STextTail>		m_TextTailPool;
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
