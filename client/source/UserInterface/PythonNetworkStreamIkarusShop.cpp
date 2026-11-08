#include "StdAfx.h"
#ifdef ENABLE_IKASHOP_RENEWAL
#include "PythonNetworkStream.h"
#include "Packet.h"
#include "PythonIkarusShop.h"

// missing Send overload getting template argument -
// - workaround using a free function

template<class Type>
bool _Send(const Type& value) {
	return CPythonNetworkStream::instance().Send(sizeof(value), &value);
}

#define Send _Send

bool CPythonNetworkStream::RecvOfflineshopPacket()
{
	TPacketGCNewOfflineshop pack;
	if(!Recv(sizeof(pack), &pack))
		return false;

	switch (pack.subheader)
	{
		case ikashop::SUBHEADER_GC_SHOP_OPEN:
			return RecvOfflineshopShopOpen();

		case ikashop::SUBHEADER_GC_SHOP_OPEN_OWNER:
			return RecvOfflineshopShopOpenOwner();

		case ikashop::SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP:
			return RecvOfflineshopShopOpenOwnerNoShop();

		case ikashop::SUBHEADER_GC_SHOP_EXPIRED_GUESTING:
			return RecvOfflineshopShopExpiredGuesting();

		case ikashop::SUBHEADER_GC_SHOP_REMOVE_ITEM_OWNER:
			return RecvOfflineshopShopRemoveItem(true);

		case ikashop::SUBHEADER_GC_SHOP_REMOVE_ITEM_GUEST:
			return RecvOfflineshopShopRemoveItem(false);

		case ikashop::SUBHEADER_GC_SHOP_EDIT_ITEM_GUEST:
			return RecvOfflineshopShopGuestEditItem();

		case ikashop::SUBHEADER_GC_SHOP_EDIT_ITEM_OWNER:
			return RecvOfflineshopShopOwnerEditItem();

		case ikashop::SUBHEADER_GC_SHOP_FILTER_RESULT:
			return RecvOfflineshopShopFilterResult();

		case ikashop::SUBHEADER_GC_OFFER_LIST:
			return RecvOfflineshopOfferList();

		case ikashop::SUBHEADER_GC_SHOP_SAFEBOX_REFRESH:
			return RecvOfflineshopShopSafeboxRefresh();

		case ikashop::SUBHEADER_GC_SHOP_SAFEBOX_REMOVE_ITEM:
			return RecvOfflineshopShopSafeboxRemoveItem();

		case ikashop::SUBHEADER_GC_SHOP_SAFEBOX_ADD_ITEM:
			return RecvOfflineshopShopSafeboxAddItem();

		case ikashop::SUBHEADER_GC_AUCTION_LIST:
			return RecvOfflineshopAuctionList();

		case ikashop::SUBHEADER_GC_OPEN_MY_AUCTION:
			return RecvOfflineshopOpenMyAuction();

		case ikashop::SUBHEADER_GC_OPEN_MY_AUCTION_NO_AUCTION:
			return RecvOfflineshopOpenMyAuctionNoAuction();

		case ikashop::SUBHEADER_GC_OPEN_AUCTION:
			return RecvOfflineshopOpenAuction();

		case ikashop::SUBHEDAER_GC_POPUP_MESSAGE:
			return RecvOfflineshopPopupMessage();

		#ifdef ENABLE_IKASHOP_ENTITIES
		case ikashop::SUBHEADER_GC_INSERT_SHOP_ENTITY:
			return RecvOfflineshopInsertEntity();

		case ikashop::SUBHEADER_GC_REMOVE_SHOP_ENTITY:
			return RecvOfflineshopRemoveEntity();
		#endif

		case ikashop::SUBHEADER_GC_BOARD_COUNTERS:
			return RecvOfflineshopBoardCounters();

		case ikashop::SUBHEADER_GC_SEARCH_RESULT_DELETE:
			return RecvOfflineshopSearchResultDelete();

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		case ikashop::SUBHEADER_GC_SHOP_FILTER_NAME_RESULT:
			return RecvOfflineshopShopFilterNameResult();
		#endif

		#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
		case ikashop::SUBHEADER_GC_SHOP_AVARAGE_AND_MINMAX_PRICE:
			return RecvOfflineshopAvarageAndMinMaxPrice();
		#endif

		default:
			TraceError("UNKNOWN OFFLINESHOP SUBHEADER : %d ",pack.subheader);
			return false;
	}
}

bool CPythonNetworkStream::RecvOfflineshopShopOpen()
{
	ikashop::TSubPacketGCShopOpen subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	std::vector<ikashop::TShopItem> items;

	items.resize(subpack.shop.count);
	if(subpack.shop.count > 0)
		if(!Recv(sizeof(items[0])*items.size(), items.data()))
			return false;

	CPythonIkarusShop::Instance().OpenShop(subpack.shop, items);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopOpenOwner()
{
	ikashop::TSubPacketGCShopOpenOwner subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	std::vector<ikashop::TShopItem> items;
	items.resize(subpack.shop.count);
	
	if(subpack.shop.count!=0)
		if(!Recv(sizeof(items[0]) * subpack.shop.count, items.data()))
			return false;

	CPythonIkarusShop::Instance().OpenShopOwner(subpack.shop, items);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopOpenOwnerNoShop()
{
	CPythonIkarusShop::instance().OpenShopOwnerNoShop();
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopExpiredGuesting()
{
	ikashop::TSubPacketGCShopExpiredGuesting subpack;
	if(!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::instance().ShopExpiredGuesting(subpack.ownerid);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopRemoveItem(bool owner)
{
	ikashop::TSubPacketGCShopRemoveItem subpack;
	if(!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::instance().ShopRemoveItem(owner, subpack.itemid);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopGuestEditItem()
{
	ikashop::TSubPacketGCShopEditItem subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::instance().ShopGuestEditItem(subpack.itemid, subpack.price);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopOwnerEditItem()
{
	ikashop::TSubPacketGCShopEditItem subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::instance().ShopOwnerEditItem(subpack.itemid, subpack.price);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopFilterResult()
{
	ikashop::TSubPacketGCShopFilterResult subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	std::vector<ikashop::TSearchResultInfo> items(static_cast<size_t>(subpack.count));
	if (subpack.count != 0)
	{
		if (!Recv(items.size() * sizeof(items[0]), items.data()))
			return false;
	}

	CPythonIkarusShop::Instance().ShopFilterResult(items);
	return true;
}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
bool CPythonNetworkStream::RecvOfflineshopShopFilterNameResult()
{
	ikashop::TSubPacketGCShopFilterResult subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	std::vector<ikashop::TSearchNameResultInfo> shops(static_cast<size_t>(subpack.count));
	if (subpack.count != 0)
	{
		if (!Recv(shops.size() * sizeof(shops[0]), shops.data()))
			return false;
	}

	CPythonIkarusShop::Instance().ShopFilterNameResult(shops);
	return true;
}
#endif

#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
bool CPythonNetworkStream::RecvOfflineshopAvarageAndMinMaxPrice()
{
	ikashop::TSubPacketGCShopAvarageAndMinMaxPrice subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::Instance().UpdateAvarageAndMinMaxPrice(subpack.avgPrice, subpack.minPrice, subpack.maxPrice);
	return true;
}
#endif

bool CPythonNetworkStream::RecvOfflineshopOfferList() //offlineshop-updated 03/08/19
{
	ikashop::TSubPacketGCShopOfferList subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	std::vector<ikashop::TOfferListElement> vec;
	vec.resize(subpack.offercount);

	if (subpack.offercount != 0)
		if (!Recv(sizeof(ikashop::TOfferListElement)*subpack.offercount, &vec[0]))
			return false;

	CPythonIkarusShop::instance().OfferListReceived(vec);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopSafeboxRefresh()
{
	ikashop::TSubPacketGCShopSafeboxRefresh subpack;
	if(!Recv(sizeof(subpack), &subpack))
		return false;

	std::vector<ikashop::TShopPlayerItem> items;
	items.resize(subpack.itemcount);

	if(subpack.itemcount != 0)
		if (!Recv(sizeof(ikashop::TShopPlayerItem) * items.size(), &items[0]))
			return false;

	CPythonIkarusShop::instance().SafeboxRefresh(subpack.valute, items);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopSafeboxRemoveItem()
{
	ikashop::TSubPacketGCShopSafeboxRemoveItem subpack{};
	if(!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::instance().SafeboxRemoveItem(subpack.itemid);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopShopSafeboxAddItem()
{
	ikashop::TSubPacketGCShoppingSafeboxAddItem subpack{};
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::instance().SafeboxAddItem(subpack.item);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopAuctionList()
{
	ikashop::TSubPacketGCAuctionList subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	std::vector<ikashop::TAuctionListElement> vector;
	vector.resize(subpack.count);

	if(subpack.count!=0)
		if(!Recv(sizeof(vector[0]) * vector.size(), &vector[0]))
			return false;

	CPythonIkarusShop::instance().AuctionList(vector);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopOpenMyAuction()
{
	ikashop::TSubPacketGCAuctionOpen subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	std::vector<ikashop::TAuctionOfferInfo> offers;
	offers.resize(subpack.offercount);

	if(subpack.offercount!= 0)
		if(!Recv(sizeof(offers[0]) * offers.size() , &offers[0]))
			return false;
	CPythonIkarusShop::instance().MyAuctionOpen(subpack.auction, offers);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopOpenMyAuctionNoAuction()
{
	CPythonIkarusShop::instance().OpenMyAuctionNoAuction();
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopOpenAuction()
{
	ikashop::TSubPacketGCAuctionOpen subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	std::vector<ikashop::TAuctionOfferInfo> offers;
	offers.resize(subpack.offercount);

	if(subpack.offercount!= 0)
		if(!Recv(sizeof(offers[0]) * offers.size() , &offers[0]))
			return false;

	CPythonIkarusShop::instance().AuctionOpen(subpack.auction, offers);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopPopupMessage()
{
	// receiving string
	ikashop::TSubPacketGCPopupMessage pack;
	if (!Recv(sizeof(pack), &pack))
		return false;

	// safely terminating string
	pack.localeString[pack.MESSAGE_LEN-1] = 0;

	CPythonIkarusShop::instance().PopupMessage(pack.localeString);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopSearchResultDelete()
{
	// getting itemid
	DWORD itemid = 0;
	if (!Recv(sizeof(itemid), &itemid))
		return false;

	CPythonIkarusShop::instance().DeleteSearchResult(itemid);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopBoardCounters()
{
	ikashop::TSubPacketGCBoardCounters subpack;
	if (!Recv(sizeof(subpack), &subpack))
		return false;

	CPythonIkarusShop::Instance().RefreshBoardCounters(subpack.safebox, subpack.auction, subpack.offers);
	return true;
}



#ifdef ENABLE_IKASHOP_ENTITIES
bool CPythonNetworkStream::RecvOfflineshopInsertEntity()
{
	ikashop::TSubPacketGCInsertShopEntity subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	CPythonIkarusShop::Instance().InsertEntity(subpack.vid, subpack.type, subpack.name, subpack.x, subpack.y, subpack.z);
	return true;
}

bool CPythonNetworkStream::RecvOfflineshopRemoveEntity()
{
	ikashop::TSubPacketGCRemoveShopEntity subpack;
	if(!Recv(sizeof(subpack) , &subpack))
		return false;

	CPythonIkarusShop::Instance().RemoveEntity(subpack.vid);
	return true;
}

void CPythonNetworkStream::SendIkarusShopOnClickShopEntity(DWORD dwPickedShopVID)
{
	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_CLICK_ENTITY;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGShopClickEntity);

	ikashop::TSubPacketCGShopClickEntity subpack;
	subpack.vid = dwPickedShopVID;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.subheader);
}
#endif

void CPythonNetworkStream::SendIkarusShopCreateShop(const ikashop::TShopInfo& shopInfo)
{
	TPacketCGNewOfflineShop pack{};
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_SHOP_CREATE_NEW;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGShopCreate);

	ikashop::TSubPacketCGShopCreate subpack;
	subpack.shop = shopInfo;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}

	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopForceCloseShop()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_FORCE_CLOSE;
	pack.size		= sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopRequestShopList()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_REQUEST_SHOPLIST;
	pack.size		= sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopOpenShop(DWORD dwOwnerID)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_OPEN;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGShopOpen);

	ikashop::TSubPacketCGShopOpen subpack;
	subpack.ownerid = dwOwnerID;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopOpenShopOwner()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_OPEN_OWNER;
	pack.size		= sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}


void CPythonNetworkStream::SendIkarusShopBuyItem(DWORD dwOwnerID, DWORD dwItemID, bool isSearch, long long TotalPriceSeen) //patch seen price check
{
	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_SHOP_BUY_ITEM;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGShopBuyItem);

	ikashop::TSubPacketCGShopBuyItem subpack;
	subpack.itemid = dwItemID;
	subpack.ownerid = dwOwnerID;
	subpack.searching = isSearch;

	//patch seen price check
	subpack.seenprice = TotalPriceSeen;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}

	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopAddItem(ikashop::TShopItemInfo& itemInfo)
{
	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_SHOP_ADD_ITEM;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGAddItem);

	ikashop::TSubPacketCGAddItem subpack;
	subpack.pos = itemInfo.pos;
	subpack.price = itemInfo.price;
	subpack.destpos = itemInfo.destpos;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopRemoveItem(DWORD dwItemID)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_REMOVE_ITEM;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGRemoveItem);

	ikashop::TSubPacketCGRemoveItem subpack;
	subpack.itemid	= dwItemID;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendOfflineShopEditItem(DWORD dwItemID, const ikashop::TPriceInfo& price)
{
	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_EDIT_ITEM;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGEditItem);

	ikashop::TSubPacketCGEditItem subpack;
	subpack.itemid = dwItemID;
	subpack.price = price;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopFilterRequest(const ikashop::TFilterInfo& filter)
{
	TPacketCGNewOfflineShop pack;
	pack.header		= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_FILTER_REQUEST;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGFilterRequest);

	ikashop::TSubPacketCGFilterRequest subpack;
	subpack.filter = filter;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
void CPythonNetworkStream::SendIkarusShopFilterNameRequest(const ikashop::TFilterName& filter)
{
	TPacketCGNewOfflineShop pack;
	pack.header		= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_FILTER_NAME_REQUEST;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGFilterNameRequest);

	ikashop::TSubPacketCGFilterNameRequest subpack;
	subpack.filter = filter;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}

	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.subheader);
		return;
	}
}
#endif

#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
void CPythonNetworkStream::SendIkarusShopAvarageAndMinMaxPrice(const uint32_t vnum)
{
	TPacketCGNewOfflineShop pack;
	pack.header		= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_AVARAGE_AND_MINMAX_PRICE;
	pack.size		= sizeof(pack) + sizeof(ikashop::TPacketCGShopAvgAndMinMaxPrice);

	ikashop::TPacketCGShopAvgAndMinMaxPrice subpack;
	subpack.vnum = vnum;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}

	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ", pack.subheader);
		return;
	}
}
#endif

void CPythonNetworkStream::SendIkarusShopSearchFillRequest()
{
	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_SHOP_SEARCH_FILL_REQUEST;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopOfferCreate(const ikashop::TOfferInfo& offer)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_OFFER_CREATE;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGOfferCreate);

	ikashop::TSubPacketCGOfferCreate subpack;
	subpack.offer = offer;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopOfferAccept(DWORD dwOfferID)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_OFFER_ACCEPT;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGOfferAccept);

	ikashop::TSubPacketCGOfferAccept subpack;
	subpack.offerid	= dwOfferID;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopOfferCancel(DWORD dwOfferID, DWORD dwOwnerID)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_OFFER_CANCEL;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGOfferCancel);

	ikashop::TSubPacketCGOfferCancel subpack;
	subpack.offerid	= dwOfferID;
	subpack.ownerid	= dwOwnerID;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopOfferListRequest()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_SHOP_REQUEST_OFFER_LIST;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopSafeboxOpen()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_SAFEBOX_OPEN;
	pack.size		= sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopSafeboxGetItem(DWORD dwItemID)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGShopSafeboxGetItem);

	ikashop::TSubPacketCGShopSafeboxGetItem subpack;
	subpack.itemid = dwItemID;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if(!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack ",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopSafeboxGetValutes()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES;
	pack.size = sizeof(pack);

	if (!Send(pack))
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
}

void CPythonNetworkStream::SendIkarusShopSafeboxClose()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_SHOP_SAFEBOX_CLOSE;
	pack.size		= sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopAuctionListRequest()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_AUCTION_LIST_REQUEST;
	pack.size		= sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopAuctionOpen(DWORD dwOwnerID)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_AUCTION_OPEN_REQUEST;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGAuctionOpenRequest);

	ikashop::TSubPacketCGAuctionOpenRequest subpack;
	subpack.ownerid = dwOwnerID;


	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
	
	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopAuctionAddOffer(DWORD dwOwnerID, const ikashop::TPriceInfo& price)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_AUCTION_ADD_OFFER;
	pack.size		= sizeof(pack) + sizeof(ikashop::TSubPacketCGAuctionAddOffer);

	ikashop::TSubPacketCGAuctionAddOffer subpack;
	subpack.ownerid = dwOwnerID;
	subpack.price = price;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopAuctionExitFrom(DWORD dwOwnerID)
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_EXIT_FROM_AUCTION;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGAuctionExitFrom);

	ikashop::TSubPacketCGAuctionExitFrom subpack;
	subpack.ownerid = dwOwnerID;
	
	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopAuctionCreate(const TItemPos& pos, const ikashop::TPriceInfo& price)
{
	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader	= ikashop::SUBHEADER_CG_CREATE_AUCTION;
	pack.size = sizeof(pack) + sizeof(ikashop::TSubPacketCGAuctionCreate);

	ikashop::TSubPacketCGAuctionCreate subpack;
	subpack.pos = pos;
	subpack.startprice = price;

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}

	if (!Send(subpack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - subpack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopMyAuctionOpen()
{
	TPacketCGNewOfflineShop pack{};
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_MY_AUCTION_OPEN_REQUEST;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopMyAuctionClose()
{
	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_MY_AUCTION_CLOSE;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusShopMyAuctionCancel()
{
	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_MY_AUCTION_CANCEL;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusMyShopCloseBoard()
{
	if(m_strPhase != "Game")
		return;

	TPacketCGNewOfflineShop pack;
	pack.header	= HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_CLOSE_MY_SHOP_BOARD;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack",pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusOffersListClose()
{
	if (m_strPhase != "Game")
		return;

	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_CLOSE_OFFER_LIST;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}
}

void CPythonNetworkStream::SendIkarusCloseShopGuestBoard()
{
	if (m_strPhase != "Game")
		return;

	TPacketCGNewOfflineShop pack;
	pack.header = HEADER_CG_NEW_OFFLINESHOP;
	pack.subheader = ikashop::SUBHEADER_CG_CLOSE_SHOP_GUEST;
	pack.size = sizeof(pack);

	if (!Send(pack))
	{
		TraceError("CANNOT SEND OFFLINESHOP PACKET : SUBHEADER %d - pack", pack.subheader);
		return;
	}
}



#endif
