#pragma once

#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
class CPythonPrivateItemShop : public CSingleton<CPythonPrivateItemShop>
{
public:
	struct TItemShopData
	{
		DWORD		itemID;
		int			cateID;
		int			subCateID;
		DWORD		vnum;
		#ifdef ENABLE_NEW_MAX_COUNT
		short		count;
		#else
		BYTE		count;
		#endif
		#ifdef ENABLE_FULL_YANG
		int64_t		price;
		#else
		DWORD		price;
		#endif
		int			discount;
		BYTE		timeType;
		BYTE		status;
		int			stock;
		int			totalSold;
		int			sellType;
		char		addTime[25];
		char		offerTime[25];
		LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
	};

	typedef std::vector<TItemShopData> TItemInstanceVector;

public:
	CPythonPrivateItemShop();
	virtual ~CPythonPrivateItemShop();

	void	AddItemData(const TItemShopData & rItemData);
	void	ClearItemData();
	void	CreateItemData();
	DWORD	GetItemDataPtr(DWORD index, TItemShopData ** ppInstance);

protected:
	TItemInstanceVector m_ItemInstanceVector;
};
#endif
