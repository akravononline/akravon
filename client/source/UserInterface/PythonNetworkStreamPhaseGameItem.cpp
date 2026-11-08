#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonSafeBox.h"
#include "PythonCharacterManager.h"
#include "PythonSystem.h"
#include "AbstractPlayer.h"

//////////////////////////////////////////////////////////////////////////
// SafeBox

bool CPythonNetworkStream::SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney)
{
	assert(!"CPythonNetworkStream::SendSafeBoxMoneyPacket - Don't use this function");
	return false;

//	TPacketCGSafeboxMoney kSafeboxMoney;
//	kSafeboxMoney.bHeader = HEADER_CG_SAFEBOX_MONEY;
//	kSafeboxMoney.bState = byState;
//	kSafeboxMoney.dwMoney = dwMoney;
//	if (!Send(sizeof(kSafeboxMoney), &kSafeboxMoney))
//		return false;
//
//	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos)
{
	__PlayInventoryItemDropSound(InventoryPos);

	TPacketCGSafeboxCheckin kSafeboxCheckin;
	kSafeboxCheckin.bHeader = HEADER_CG_SAFEBOX_CHECKIN;
	kSafeboxCheckin.ItemPos = InventoryPos;
	kSafeboxCheckin.bSafePos = bySafeBoxPos;
	if (!Send(sizeof(kSafeboxCheckin), &kSafeboxCheckin))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos)
{
	__PlaySafeBoxItemDropSound(bySafeBoxPos);

	TPacketCGSafeboxCheckout kSafeboxCheckout;
	kSafeboxCheckout.bHeader = HEADER_CG_SAFEBOX_CHECKOUT;
	kSafeboxCheckout.bSafePos = bySafeBoxPos;
	kSafeboxCheckout.ItemPos = InventoryPos;
	if (!Send(sizeof(kSafeboxCheckout), &kSafeboxCheckout))
		return false;

	return SendSequence();
}

#ifdef ENABLE_NEW_MAX_COUNT
bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, short byCount)
#else
bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount)
#endif
{
	__PlaySafeBoxItemDropSound(bySourcePos);

	TPacketCGItemMove kItemMove;
	kItemMove.header = HEADER_CG_SAFEBOX_ITEM_MOVE;
	kItemMove.pos = TItemPos(INVENTORY, bySourcePos);
	kItemMove.num = byCount;
	kItemMove.change_pos = TItemPos(INVENTORY, byTargetPos);
	if (!Send(sizeof(kItemMove), &kItemMove))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvSafeBoxSetPacket()
{
	TPacketGCItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum	= kItemSet.vnum;
	kItemData.count	= kItemSet.count;
	kItemData.flags	= kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;

	for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];

	for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];

	#ifdef ENABLE_EVOLUTION_SYSTEM
	kItemData.iEvolution = kItemSet.iEvolution;
	#endif
	#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.dwTransmutation = kItemSet.dwTransmutation;
	#endif
	#ifdef ENABLE_LOCKED_SYSTEM
	kItemData.bLocked = kItemSet.bLocked;
	#endif

	CPythonSafeBox::Instance().SetItemData(kItemSet.Cell.cell, kItemData);

	__RefreshSafeboxWindow();
	return true;
}

bool CPythonNetworkStream::RecvSafeBoxDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelItemData(kItemDel.pos);

	__RefreshSafeboxWindow();
	return true;
}

bool CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket()
{
	TPacketGCSafeboxWrongPassword kSafeboxWrongPassword;

	if (!Recv(sizeof(kSafeboxWrongPassword), &kSafeboxWrongPassword))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnSafeBoxError", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxSizePacket()
{
	TPacketGCSafeboxSize kSafeBoxSize;
	if (!Recv(sizeof(kSafeBoxSize), &kSafeBoxSize))
		return false;

	CPythonSafeBox::Instance().OpenSafeBox(kSafeBoxSize.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSafeboxWindow", Py_BuildValue("(i)", kSafeBoxSize.bSize));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxMoneyChangePacket()
{
	TPacketGCSafeboxMoneyChange kMoneyChange;
	if (!Recv(sizeof(kMoneyChange), &kMoneyChange))
		return false;

	CPythonSafeBox::Instance().SetMoney(kMoneyChange.dwMoney);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafeboxMoney", Py_BuildValue("()"));

	return true;
}

// SafeBox
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mall
bool CPythonNetworkStream::SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos)
{
	__PlayMallItemDropSound(byMallPos);

	TPacketCGMallCheckout kMallCheckoutPacket;
	kMallCheckoutPacket.bHeader = HEADER_CG_MALL_CHECKOUT;
	kMallCheckoutPacket.bMallPos = byMallPos;
	kMallCheckoutPacket.ItemPos = InventoryPos;
	if (!Send(sizeof(kMallCheckoutPacket), &kMallCheckoutPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvMallOpenPacket()
{
	TPacketGCMallOpen kMallOpen;
	if (!Recv(sizeof(kMallOpen), &kMallOpen))
		return false;

	CPythonSafeBox::Instance().OpenMall(kMallOpen.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMallWindow", Py_BuildValue("(i)", kMallOpen.bSize));

	return true;
}

bool CPythonNetworkStream::RecvMallItemSetPacket()
{
	TPacketGCItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;

	for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];

	for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];

	#ifdef ENABLE_EVOLUTION_SYSTEM
	kItemData.iEvolution = kItemSet.iEvolution;
	#endif
	#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.dwTransmutation = kItemSet.dwTransmutation;
	#endif
	#ifdef ENABLE_LOCKED_SYSTEM
	kItemData.bLocked = kItemSet.bLocked;
	#endif

	CPythonSafeBox::Instance().SetMallItemData(kItemSet.Cell.cell, kItemData);

	__RefreshMallWindow();
	return true;
}

bool CPythonNetworkStream::RecvMallItemDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelMallItemData(kItemDel.pos);

	__RefreshMallWindow();
	Tracef(" >> CPythonNetworkStream::RecvMallItemDelPacket\n");

	return true;
}
// Mall
//////////////////////////////////////////////////////////////////////////

// Item
// Recieve
bool CPythonNetworkStream::RecvItemDelPacket()
{
	TPacketGCItemDelDeprecated packet_item_del;
	if (!Recv(sizeof(TPacketGCItemDelDeprecated), &packet_item_del))
		return false;

	TItemData kItemData;
	kItemData.vnum = 0;
	kItemData.count = 0;
	kItemData.flags = 0;

	memset(kItemData.alSockets, 0, sizeof(kItemData.alSockets));
	memset(kItemData.aAttr, 0, sizeof(kItemData.aAttr));

	#ifdef ENABLE_EVOLUTION_SYSTEM
	kItemData.iEvolution = 0;
	#endif
	#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.dwTransmutation = 0;
	#endif
	#ifdef ENABLE_LOCKED_SYSTEM
	kItemData.bLocked = 0;
	#endif

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_del.Cell, kItemData);

	#ifdef ENABLE_SWITCHBOT
	if (packet_item_del.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
	#endif

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemSetPacket()
{
	TPacketGCItemSet packet_item_set;
	if (!Recv(sizeof(TPacketGCItemSet), &packet_item_set))
		return false;

	TItemData kItemData;
	kItemData.vnum	= packet_item_set.vnum;
	kItemData.count	= packet_item_set.count;
	kItemData.flags	= packet_item_set.flags;
	kItemData.anti_flags = packet_item_set.anti_flags;

	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i] = packet_item_set.alSockets[i];

	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j] = packet_item_set.aAttr[j];

	#ifdef ENABLE_EVOLUTION_SYSTEM
	kItemData.iEvolution = packet_item_set.iEvolution;
	#endif
	#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.dwTransmutation = packet_item_set.dwTransmutation;
	#endif
	#ifdef ENABLE_LOCKED_SYSTEM
	kItemData.bLocked = packet_item_set.bLocked;
	#endif

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_set.Cell, kItemData);

	#ifdef ENABLE_SWITCHBOT
	if (packet_item_set.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
	#endif

	if (CPythonSystem::Instance().IsShowHighlight())
	{
		if (packet_item_set.highlight)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Highlight_Item", Py_BuildValue("(ii)", packet_item_set.Cell.window_type, packet_item_set.Cell.cell));
	}

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemUsePacket()
{
	TPacketGCItemUse packet_item_use;

	if (!Recv(sizeof(TPacketGCItemUse), &packet_item_use))
		return false;

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemUpdatePacket()
{
	TPacketGCItemUpdate packet_item_update;
	if (!Recv(sizeof(TPacketGCItemUpdate), &packet_item_update))
		return false;

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	#ifdef ENABLE_ITEM_UPDATE_OPTIMIZATION
	rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count, false);
	#else
	rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count);
	#endif

#ifdef ENABLE_EVOLUTION_SYSTEM
	#ifdef ENABLE_ITEM_UPDATE_OPTIMIZATION
	rkPlayer.SetItemEvolution(packet_item_update.Cell, packet_item_update.iEvolution, false);
	#else
	rkPlayer.SetItemEvolution(packet_item_update.Cell, packet_item_update.iEvolution);
	#endif
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
	#ifdef ENABLE_ITEM_UPDATE_OPTIMIZATION
	rkPlayer.SetItemTransmutation(packet_item_update.Cell, packet_item_update.dwTransmutation, false);
	#else
	rkPlayer.SetItemTransmutation(packet_item_update.Cell, packet_item_update.dwTransmutation);
	#endif
#endif

#ifdef ENABLE_LOCKED_SYSTEM
	#ifdef ENABLE_ITEM_UPDATE_OPTIMIZATION
	rkPlayer.SetItemLocked(packet_item_update.Cell, packet_item_update.bLocked, false);
	#else
	rkPlayer.SetItemLocked(packet_item_update.Cell, packet_item_update.bLocked);
	#endif
#endif

	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		rkPlayer.SetItemMetinSocket(packet_item_update.Cell, i, packet_item_update.alSockets[i]);

	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		rkPlayer.SetItemAttribute(packet_item_update.Cell, j, packet_item_update.aAttr[j].bType, packet_item_update.aAttr[j].sValue);

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemGroundAddPacket()
{
	TPacketGCItemGroundAdd packet_item_ground_add;
	if (!Recv(sizeof(TPacketGCItemGroundAdd), &packet_item_ground_add))
		return false;

	__GlobalPositionToLocalPosition(packet_item_ground_add.lX, packet_item_ground_add.lY);

	CPythonItem::Instance().CreateItem(packet_item_ground_add.dwVID,
									   packet_item_ground_add.dwVnum,
									   packet_item_ground_add.lX,
									   packet_item_ground_add.lY,
									   packet_item_ground_add.lZ);
	return true;
}

bool CPythonNetworkStream::RecvItemOwnership()
{
	TPacketGCItemOwnership p;
	if (!Recv(sizeof(TPacketGCItemOwnership), &p))
		return false;

	CPythonItem::Instance().SetOwnership(p.dwVID, p.szName);
	return true;
}

bool CPythonNetworkStream::RecvItemGroundDelPacket()
{
	TPacketGCItemGroundDel packet_item_ground_del;
	if (!Recv(sizeof(TPacketGCItemGroundDel), &packet_item_ground_del))
		return false;

	CPythonItem::Instance().DeleteItem(packet_item_ground_del.vid);
	return true;
}

bool CPythonNetworkStream::RecvQuickSlotAddPacket()
{
	TPacketGCQuickSlotAdd packet_quick_slot_add;
	if (!Recv(sizeof(TPacketGCQuickSlotAdd), &packet_quick_slot_add))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.AddQuickSlot(packet_quick_slot_add.pos, packet_quick_slot_add.slot.Type, packet_quick_slot_add.slot.Position);

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvQuickSlotDelPacket()
{
	TPacketGCQuickSlotDel packet_quick_slot_del;
	if (!Recv(sizeof(TPacketGCQuickSlotDel), &packet_quick_slot_del))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.DeleteQuickSlot(packet_quick_slot_del.pos);

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvQuickSlotMovePacket()
{
	TPacketGCQuickSlotSwap packet_quick_slot_swap;
	if (!Recv(sizeof(TPacketGCQuickSlotSwap), &packet_quick_slot_swap))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.MoveQuickSlot(packet_quick_slot_swap.pos, packet_quick_slot_swap.change_pos);

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::SendShopEndPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop packet_shop;
	packet_shop.header = HEADER_CG_SHOP;
	packet_shop.subheader = SHOP_SUBHEADER_CG_END;

	if (!Send(sizeof(packet_shop), &packet_shop))
	{
		Tracef("SendShopEndPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendShopBuyPacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUY;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	BYTE bCount=1;
	if (!Send(sizeof(BYTE), &bCount))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendShopSellPacket(BYTE bySlot)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
	if (!Send(sizeof(BYTE), &bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_SPECIAL_STORAGE
#ifdef ENABLE_NEW_MAX_COUNT
bool CPythonNetworkStream::SendShopSellPacketNew(BYTE bySlot, short byCount, BYTE byType)
#else
bool CPythonNetworkStream::SendShopSellPacketNew(BYTE bySlot, BYTE byCount, BYTE byType)
#endif
#else
#ifdef ENABLE_NEW_MAX_COUNT
bool CPythonNetworkStream::SendShopSellPacketNew(BYTE bySlot, short byCount)
#else
bool CPythonNetworkStream::SendShopSellPacketNew(BYTE bySlot, BYTE byCount)
#endif
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL2;

	TPacketGCShopDetail packetDetail;
#ifdef ENABLE_SPECIAL_STORAGE
	packetDetail.itemPos = TItemPos(byType, bySlot);
#else
	packetDetail.itemPos = TItemPos(INVENTORY, bySlot);
#endif

	packetDetail.byCount = byCount;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}

	if (!Send(sizeof(TPacketGCShopDetail), &packetDetail))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}

	Tracef(" SendShopSellPacketNew(bySlot=%d, byCount=%d)\n", packetDetail.itemPos, packetDetail.byCount);

	return SendSequence();
}

// Send
bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos)
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
			return true;
		}

		if (CPythonShop::Instance().IsOpen())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
			return true;
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemUseSound(pos);

	TPacketCGItemUse itemUsePacket;
	itemUsePacket.header = HEADER_CG_ITEM_USE;
	itemUsePacket.pos = pos;

	if (!Send(sizeof(TPacketCGItemUse), &itemUsePacket))
	{
		Tracen("SendItemUsePacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemUseToItem itemUseToItemPacket;
	itemUseToItemPacket.header = HEADER_CG_ITEM_USE_TO_ITEM;
	itemUseToItemPacket.source_pos = source_pos;
	itemUseToItemPacket.target_pos = target_pos;

	if (!Send(sizeof(TPacketCGItemUseToItem), &itemUseToItemPacket))
	{
		Tracen("SendItemUseToItemPacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemUseToItemPacket(src=%d, dst=%d)\n", source_pos, target_pos);
#endif

	return SendSequence();
}

#ifdef ENABLE_DROP_DIALOG_EXTENDED_SYSTEM
bool CPythonNetworkStream::SendItemDeletePacket(TItemPos item_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDelete itemDeletePacket;
	itemDeletePacket.header = HEADER_CG_ITEM_DELETE;
	itemDeletePacket.item_pos = item_pos;

	if (!Send(sizeof(TPacketCGItemDelete), &itemDeletePacket))
	{
		Tracen("SendItemDeletePacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemDeletePacket(item_pos=%d)\n", item_pos);
#endif

	return true;
}

bool CPythonNetworkStream::SendItemSellPacket(TItemPos item_pos, bool is_auto)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemSell itemSellPacket;
	itemSellPacket.header = HEADER_CG_ITEM_SELL;
	itemSellPacket.item_pos = item_pos;
	itemSellPacket.is_auto = is_auto;

	if (!Send(sizeof(TPacketCGItemSell), &itemSellPacket))
	{
		Tracen("SendItemSellPacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemSellPacket(item_pos=%d)\n", item_pos);
#endif

	return true;
}
#else
bool CPythonNetworkStream::SendItemDropPacket(TItemPos pos, DWORD elk)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP;
	itemDropPacket.pos = pos;
	itemDropPacket.elk = elk;

	if (!Send(sizeof(TPacketCGItemDrop), &itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD count)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop2 itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP2;
	itemDropPacket.pos = pos;
	itemDropPacket.gold = elk;
	itemDropPacket.count = count;

	if (!Send(sizeof(itemDropPacket), &itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::__IsEquipItemInSlot(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	return rkPlayer.IsEquipItemInSlot(uSlotPos);
}

void CPythonNetworkStream::__PlayInventoryItemUseSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	DWORD dwItemID=rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayUseSound(dwItemID);
}

void CPythonNetworkStream::__PlayInventoryItemDropSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	DWORD dwItemID=rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

//void CPythonNetworkStream::__PlayShopItemDropSound(UINT uSlotPos)
//{
//	DWORD dwItemID;
//	CPythonShop& rkShop=CPythonShop::Instance();
//	if (!rkShop.GetSlotItemID(uSlotPos, &dwItemID))
//		return;
//
//	CPythonItem& rkItem=CPythonItem::Instance();
//	rkItem.PlayDropSound(dwItemID);
//}

void CPythonNetworkStream::__PlaySafeBoxItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox=CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayMallItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox=CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotMallItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

#ifdef ENABLE_NEW_MAX_COUNT
bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, short num)
#else
bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num)
#endif
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
				return true;
			}
		}

		if (CPythonShop::Instance().IsOpen())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
				return true;
			}
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemDropSound(pos);

	TPacketCGItemMove	itemMovePacket;
	itemMovePacket.header = HEADER_CG_ITEM_MOVE;
	itemMovePacket.pos = pos;
	itemMovePacket.change_pos = change_pos;
	itemMovePacket.num = num;

	if (!Send(sizeof(TPacketCGItemMove), &itemMovePacket))
	{
		Tracen("SendItemMovePacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemPickUpPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemPickUp	itemPickUpPacket;
	itemPickUpPacket.header = HEADER_CG_ITEM_PICKUP;
	itemPickUpPacket.vid = vid;

	if (!Send(sizeof(TPacketCGItemPickUp), &itemPickUpPacket))
	{
		Tracen("SendItemPickUpPacket Error");
		return false;
	}

	return SendSequence();
}


bool CPythonNetworkStream::SendQuickSlotAddPacket(BYTE wpos, BYTE type, BYTE pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotAdd quickSlotAddPacket;

	quickSlotAddPacket.header		= HEADER_CG_QUICKSLOT_ADD;
	quickSlotAddPacket.pos			= wpos;
	quickSlotAddPacket.slot.Type	= type;
	quickSlotAddPacket.slot.Position = pos;

	if (!Send(sizeof(TPacketCGQuickSlotAdd), &quickSlotAddPacket))
	{
		Tracen("SendQuickSlotAddPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotDelPacket(BYTE pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotDel quickSlotDelPacket;

	quickSlotDelPacket.header = HEADER_CG_QUICKSLOT_DEL;
	quickSlotDelPacket.pos = pos;

	if (!Send(sizeof(TPacketCGQuickSlotDel), &quickSlotDelPacket))
	{
		Tracen("SendQuickSlotDelPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotMovePacket(BYTE pos, BYTE change_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotSwap quickSlotSwapPacket;

	quickSlotSwapPacket.header = HEADER_CG_QUICKSLOT_SWAP;
	quickSlotSwapPacket.pos = pos;
	quickSlotSwapPacket.change_pos = change_pos;

	if (!Send(sizeof(TPacketCGQuickSlotSwap), &quickSlotSwapPacket))
	{
		Tracen("SendQuickSlotSwapPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvSpecialEffect()
{
	TPacketGCSpecialEffect kSpecialEffect;
	if (!Recv(sizeof(kSpecialEffect), &kSpecialEffect))
		return false;

	DWORD effect = -1;
	bool bPlayPotionSound = false;
	bool bAttachEffect = true;
	switch (kSpecialEffect.type)
	{
		case SE_HPUP_RED:
			effect = CInstanceBase::EFFECT_HPUP_RED;
			bPlayPotionSound = true;
			break;
		case SE_SPUP_BLUE:
			effect = CInstanceBase::EFFECT_SPUP_BLUE;
			bPlayPotionSound = true;
			break;
		case SE_SPEEDUP_GREEN:
			effect = CInstanceBase::EFFECT_SPEEDUP_GREEN;
			bPlayPotionSound = true;
			break;
		case SE_DXUP_PURPLE:
			effect = CInstanceBase::EFFECT_DXUP_PURPLE;
			bPlayPotionSound = true;
			break;
		case SE_CRITICAL:
			effect = CInstanceBase::EFFECT_CRITICAL;
			break;
		case SE_PENETRATE:
			effect = CInstanceBase::EFFECT_PENETRATE;
			break;
		case SE_BLOCK:
			effect = CInstanceBase::EFFECT_BLOCK;
			break;
		case SE_DODGE:
			effect = CInstanceBase::EFFECT_DODGE;
			break;
		case SE_CHINA_FIREWORK:
			effect = CInstanceBase::EFFECT_FIRECRACKER;
			bAttachEffect = false;
			break;
		case SE_SPIN_TOP:
			effect = CInstanceBase::EFFECT_SPIN_TOP;
			bAttachEffect = false;
			break;
		case SE_SUCCESS :
			effect = CInstanceBase::EFFECT_SUCCESS ;
			bAttachEffect = false ;
			break ;
		case SE_FAIL :
			effect = CInstanceBase::EFFECT_FAIL ;
			break ;
		case SE_FR_SUCCESS:
			effect = CInstanceBase::EFFECT_FR_SUCCESS;
			bAttachEffect = false ;
			break;
		case SE_LEVELUP_ON_14_FOR_GERMANY:
			effect = CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY;
			bAttachEffect = false ;
			break;
		case SE_LEVELUP_UNDER_15_FOR_GERMANY:
			effect = CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY;
			bAttachEffect = false ;
			break;
		case SE_PERCENT_DAMAGE1:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE1;
			break;
		case SE_PERCENT_DAMAGE2:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE2;
			break;
		case SE_PERCENT_DAMAGE3:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE3;
			break;
		case SE_AUTO_HPUP:
			effect = CInstanceBase::EFFECT_AUTO_HPUP;
			break;
		case SE_AUTO_SPUP:
			effect = CInstanceBase::EFFECT_AUTO_SPUP;
			break;
		case SE_EQUIP_RAMADAN_RING:
			effect = CInstanceBase::EFFECT_RAMADAN_RING_EQUIP;
			break;
		case SE_EQUIP_HALLOWEEN_CANDY:
			effect = CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP;
			break;
		case SE_EQUIP_HAPPINESS_RING:
 			effect = CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP;
			break;
		case SE_EQUIP_LOVE_PENDANT:
			effect = CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP;
			break;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		case SE_EFFECT_ACCE_SUCCEDED:
			effect = CInstanceBase::EFFECT_ACCE_SUCCEDED;
			break;
		case SE_EFFECT_ACCE_EQUIP:
			effect = CInstanceBase::EFFECT_ACCE_EQUIP;
			break;
#endif
#ifdef ENABLE_SMITH_EFFECT_SYSTEM
		case SE_FR_FAIL:
			effect = CInstanceBase::EFFECT_FR_FAIL;
			break;
#endif
		default:
			TraceError("%d is not a special effect number. TPacketGCSpecialEffect",kSpecialEffect.type);
			break;
	}

	if (bPlayPotionSound)
	{
		IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
		if(rkPlayer.IsMainCharacterIndex(kSpecialEffect.vid))
		{
			CPythonItem& rkItem=CPythonItem::Instance();
			rkItem.PlayUsePotionSound();
		}
	}

	if (-1 != effect)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
		if (pInstance)
		{
			if(bAttachEffect)
				pInstance->AttachSpecialEffect(effect);
			else
				pInstance->CreateSpecialEffect(effect);
		}
	}

	return true;
}


bool CPythonNetworkStream::RecvSpecificEffect()
{
	TPacketGCSpecificEffect kSpecificEffect;
	if (!Recv(sizeof(kSpecificEffect), &kSpecificEffect))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecificEffect.vid);
	//EFFECT_TEMP
	if (pInstance)
	{
		CInstanceBase::RegisterEffect(CInstanceBase::EFFECT_TEMP, "", kSpecificEffect.effect_file, false);
		pInstance->AttachSpecialEffect(CInstanceBase::EFFECT_TEMP);
	}

	return true;
}

bool CPythonNetworkStream::RecvDragonSoulRefine()
{
	TPacketGCDragonSoulRefine kDragonSoul;

	if (!Recv(sizeof(kDragonSoul), &kDragonSoul))
		return false;


	switch (kDragonSoul.bSubType)
	{
	case DS_SUB_HEADER_OPEN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("()"));
		break;
	case DS_SUB_HEADER_REFINE_FAIL:
	case DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE:
	case DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineFail", Py_BuildValue("(iii)",
			kDragonSoul.bSubType, kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	case DS_SUB_HEADER_REFINE_SUCCEED:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineSucceed",
				Py_BuildValue("(ii)", kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	}

	return true;
}

#ifdef ENABLE_STYLE_ATTRIBUTE_SYSTEM
bool CPythonNetworkStream::SendItemNewAttributePacket(TItemPos source_pos, TItemPos target_pos, BYTE * bValues)
{
	if (!__CanActMainInstance())
		return true;
	TPacketCGItemNewAttribute itemNewAttributePacket;
	itemNewAttributePacket.header = HEADER_CG_ITEM_USE_NEW_ATTRIBUTE;
	itemNewAttributePacket.source_pos = source_pos;
	itemNewAttributePacket.target_pos = target_pos;
	itemNewAttributePacket.bValues[0] = bValues[0];
	itemNewAttributePacket.bValues[1] = bValues[1];
	itemNewAttributePacket.bValues[2] = bValues[2];
	itemNewAttributePacket.bValues[3] = bValues[3];
	// itemNewAttributePacket.bValues[4] = bValues[4];
	if (!Send(sizeof(TPacketCGItemNewAttribute), &itemNewAttributePacket))
	{
		Tracen("SendItemNewAttributePacket Error");
		return false;
	}
	return true;
}
#endif

#ifdef ENABLE_HIDE_COSTUME_PART
bool CPythonNetworkStream::ChangeCostumeVisible(BYTE type, bool status)
{
	TPacketCGChangeCostumeVisibilityStatus kChange;
	kChange.bHeader = HEADER_CG_CHANGE_COSTUME_VISIBILITY_STATUS;
	kChange.type = type;
	kChange.status = status;
	if (!Send(sizeof(kChange), &kChange))
	{
		return false;
	}
	return true;
}
#endif

#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
bool CPythonNetworkStream::SendItemShopOpenAdminPagePacket()
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess packet;
	packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	packet.subheader = ITEMSHOP_PROCESS_ADMIN_PAGE_OPEN;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("SendItemShopOpenAdminPagePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopCateAddPacket(TGCIShopCateAdmin table)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_CATE_ADD;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopCateAddPacket Error\n");
		return false;
	}

	if (!Send(sizeof(table), &table))
	{
		Tracef("SendItemShopCateAddPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopCateDeletePacket(TGCIShopCateAdmin table)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_CATE_DEL;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopCateDeletePacket Error\n");
		return false;
	}

	if (!Send(sizeof(table), &table))
	{
		Tracef("SendItemShopCateDeletePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopSubCateAddPacket(TGCIShopCateAdmin table)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_SUBCATE_ADD;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopSubCateAddPacket Error\n");
		return false;
	}

	if (!Send(sizeof(table), &table))
	{
		Tracef("SendItemShopSubCateAddPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopSubCateDeletePacket(TGCIShopCateAdmin table)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_SUBCATE_DEL;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopCateDeletePacket Error\n");
		return false;
	}

	if (!Send(sizeof(table), &table))
	{
		Tracef("SendItemShopCateDeletePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopRenamePacket(TGCIShopCateAdmin table)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_RENAME;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopRenamePacket Error\n");
		return false;
	}

	if (!Send(sizeof(table), &table))
	{
		Tracef("SendItemShopRenamePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopItemAddPacket(TGCIShopItemAdmin table)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_ITEM_ADD;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopItemAddPacket Error\n");
		return false;
	}

	if (!Send(sizeof(table), &table))
	{
		Tracef("SendItemShopItemAddPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopItemUpdatePacket(TGCIShopItemAdmin table)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_ITEM_UPDATE;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopItemUpdatePacket Error\n");
		return false;
	}

	if (!Send(sizeof(table), &table))
	{
		Tracef("SendItemShopItemUpdatePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemShopItemDeletePacket(DWORD dwItemID)
{
	if (!__CanActMainInstance())
		return true;

	TCGItemShopAdminProcess Packet;
	Packet.header = HEADER_CG_ITEMSHOP_ADMIN_PROCESS;
	Packet.subheader = ITEMSHOP_PROCESS_ITEM_DEL;

	if (!Send(sizeof(TCGItemShopAdminProcess), &Packet))
	{
		Tracef("SendItemShopItemDeletePacket Error\n");
		return false;
	}

	if (!Send(sizeof(DWORD), &dwItemID))
	{
		Tracef("SendItemShopItemDeletePacket Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_BLACK_MARKET_SYSTEM
bool CPythonNetworkStream::SendOpenBlackMarketPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemBlackMarket blackmarketpacket;
	blackmarketpacket.header = HEADER_CG_BLACKMARKET;
	blackmarketpacket.subHeader = BM_SUBHEADER_OPEN;
	blackmarketpacket.Cell = TItemPos(RESERVED_WINDOW, 0);

	if (!Send(sizeof(blackmarketpacket), &blackmarketpacket))
	{
		Tracen("SendOpenBlackMarketPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemBlackMarketPacket(TItemPos pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemBlackMarket blackmarketpacket;
	blackmarketpacket.header = HEADER_CG_BLACKMARKET;
	blackmarketpacket.subHeader = BM_SUBHEADER_SELL;
	blackmarketpacket.Cell = pos;

	if (!Send(sizeof(blackmarketpacket), &blackmarketpacket))
	{
		Tracen("SendItemBlackMarketPacket Error");
		return false;
	}

	return SendSequence();
}
#endif

#if defined(__BL_LUCKY_BOX__)
bool CPythonNetworkStream::SendLuckyBoxActionPacket(BYTE bAction)
{
	TPacketCGLuckyBox p;
	p.header = HEADER_CG_LUCKY_BOX;
	p.action = bAction;

	if (!Send(sizeof(p), &p))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvLuckyBoxPacket()
{
	TPacketGCLuckyBox p;
	if (!Recv(sizeof(p), &p))
	{
		Tracen("RecvLuckyBox Packet Error");
		return false;
	}

	#ifdef ENABLE_FULL_YANG
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowLuckyBoxWindow", Py_BuildValue("(iiK)", p.vnum, p.count, p.need_money));
	#else
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowLuckyBoxWindow", Py_BuildValue("(iii)", p.vnum, p.count, p.need_money));
	#endif
	return true;
}
#endif

//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
