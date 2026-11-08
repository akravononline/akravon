#include "stdafx.h"
#include "PythonPrivateItemShop.h"
#include "PythonNetworkStream.h"

#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
CPythonPrivateItemShop::CPythonPrivateItemShop()
{
}

CPythonPrivateItemShop::~CPythonPrivateItemShop()
{
}

void CPythonPrivateItemShop::AddItemData(const TItemShopData & rItemData)
{
	m_ItemInstanceVector.push_back(rItemData);
}

void CPythonPrivateItemShop::ClearItemData()
{
	m_ItemInstanceVector.clear();
}

void CPythonPrivateItemShop::CreateItemData()
{
	TItemShopData rItemData;
	AddItemData(rItemData);
}

DWORD CPythonPrivateItemShop::GetItemDataPtr(DWORD index, TItemShopData ** ppInstance)
{
	*ppInstance = &m_ItemInstanceVector.at(index);
	return 1;
}

PyObject * privateItemShopClear(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPrivateItemShop::Instance().ClearItemData();
	return Py_BuildNone();
}

PyObject * privateItemShopCreate(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPrivateItemShop::Instance().CreateItemData();
	return Py_BuildNone();
}

PyObject * privateItemShopInitialize(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPrivateItemShop::Instance().ClearItemData();
	CPythonPrivateItemShop::Instance().CreateItemData();
	return Py_BuildNone();
}

PyObject * privateItemShopSetItemID(PyObject * poSelf, PyObject * poArgs)
{
	int iItemID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemID))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->itemID = iItemID;
	return Py_BuildNone();
}

PyObject * privateItemShopSetCateID(PyObject * poSelf, PyObject * poArgs)
{
	int iCateID;
	if (!PyTuple_GetInteger(poArgs, 0, &iCateID))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->cateID = iCateID;
	return Py_BuildNone();
}

PyObject * privateItemShopSetSubCateID(PyObject * poSelf, PyObject * poArgs)
{
	int iSubCateID;
	if (!PyTuple_GetInteger(poArgs, 0, &iSubCateID))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->subCateID = iSubCateID;
	return Py_BuildNone();
}

PyObject * privateItemShopSetSellTypeID(PyObject * poSelf, PyObject * poArgs)
{
	int iSellTypeID;
	if (!PyTuple_GetInteger(poArgs, 0, &iSellTypeID))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->sellType = iSellTypeID;
	return Py_BuildNone();
}

PyObject * privateItemShopSetVnum(PyObject * poSelf, PyObject * poArgs)
{
	int iVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVnum))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->vnum = iVnum;
	return Py_BuildNone();
}

PyObject * privateItemShopSetCount(PyObject * poSelf, PyObject * poArgs)
{
	#ifdef ENABLE_NEW_MAX_COUNT
	short iCount;
	#else
	BYTE iCount;
	#endif
	if (!PyTuple_GetInteger(poArgs, 0, &iCount))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->count = iCount;
	return Py_BuildNone();
}

PyObject * privateItemShopSetPrice(PyObject * poSelf, PyObject * poArgs)
{
	#ifdef ENABLE_FULL_YANG
	int64_t iPrice;
	if (!PyTuple_GetLongLong(poArgs, 0, &iPrice))
		return Py_BadArgument();
	#else
	int iPrice;
	if (!PyTuple_GetInteger(poArgs, 0, &iPrice))
		return Py_BadArgument();
	#endif

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->price = iPrice;
	return Py_BuildNone();
}

PyObject * privateItemShopSetDiscount(PyObject * poSelf, PyObject * poArgs)
{
	int iDiscount;
	if (!PyTuple_GetInteger(poArgs, 0, &iDiscount))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->discount = iDiscount;
	return Py_BuildNone();
}

PyObject * privateItemShopSetTimeType(PyObject * poSelf, PyObject * poArgs)
{
	int iTimeType;
	if (!PyTuple_GetInteger(poArgs, 0, &iTimeType))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->timeType = (BYTE)(iTimeType);
	return Py_BuildNone();
}

PyObject * privateItemShopSetStatus(PyObject * poSelf, PyObject * poArgs)
{
	int iStatus;
	if (!PyTuple_GetInteger(poArgs, 0, &iStatus))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->status = (BYTE)(iStatus);
	return Py_BuildNone();
}

PyObject * privateItemShopSetStock(PyObject * poSelf, PyObject * poArgs)
{
	int iStock;
	if (!PyTuple_GetInteger(poArgs, 0, &iStock))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->stock = iStock;
	return Py_BuildNone();
}

PyObject * privateItemShopSetTotalSold(PyObject * poSelf, PyObject * poArgs)
{
	int iSold;
	if (!PyTuple_GetInteger(poArgs, 0, &iSold))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->totalSold = iSold;
	return Py_BuildNone();
}

PyObject * privateItemShopSetAddTime(PyObject* poSelf, PyObject* poArgs)
{
	char* szAddTime;
	if (!PyTuple_GetString(poArgs, 0, &szAddTime))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	strncpy(pItemData->addTime, szAddTime, sizeof(pItemData->addTime));
	return Py_BuildNone();
}

PyObject * privateItemShopSetOfferTime(PyObject* poSelf, PyObject* poArgs)
{
	char* szOfferTime;
	if (!PyTuple_GetString(poArgs, 0, &szOfferTime))
		return Py_BadArgument();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	strncpy(pItemData->offerTime, szOfferTime, sizeof(pItemData->offerTime));
	return Py_BuildNone();
}

PyObject * privateItemShopSetSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSocketIndex))
		return Py_BadArgument();

	int iSocketValue;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketValue))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->alSockets[iSocketIndex] = iSocketValue;
	return Py_BuildNone();
}

PyObject * privateItemShopSetAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iAttrSlotIndex))
		return Py_BuildException();

	int iAttrType;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrType))
		return Py_BuildException();

	int iAttrValue;
	if (!PyTuple_GetInteger(poArgs, 2, &iAttrValue))
		return Py_BuildException();

	if (iAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return Py_BuildException();

	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	pItemData->aAttr[iAttrSlotIndex].bType = iAttrType;
	pItemData->aAttr[iAttrSlotIndex].sValue = iAttrValue;
	return Py_BuildNone();
}

///###########################################################################################################///
PyObject * privateItemShopAddCate(PyObject * poSelf, PyObject * poArgs)
{
	int iCateType;
	if (!PyTuple_GetInteger(poArgs, 0, &iCateType))
		return Py_BuildException();

	int iCateID;
	if (!PyTuple_GetInteger(poArgs, 1, &iCateID))
		return Py_BuildException();

	char* szCateName;
	if (!PyTuple_GetString(poArgs, 2, &szCateName))
		return Py_BadArgument();

	TGCIShopCateAdmin table;
	table.iCateID = iCateID;
	strncpy(table.szCategory, szCateName, sizeof(table.szCategory)-1);

	if (iCateType == 1)
		CPythonNetworkStream::Instance().SendItemShopCateAddPacket(table);
	else
		CPythonNetworkStream::Instance().SendItemShopSubCateAddPacket(table);

	return Py_BuildNone();
}

PyObject * privateItemShopDeleteCate(PyObject * poSelf, PyObject * poArgs)
{
	int iCateType;
	if (!PyTuple_GetInteger(poArgs, 0, &iCateType))
		return Py_BuildException();

	int iCateID;
	if (!PyTuple_GetInteger(poArgs, 1, &iCateID))
		return Py_BuildException();

	TGCIShopCateAdmin table;
	table.iCateID = iCateID;
	strncpy(table.szCategory, "NULL", sizeof(table.szCategory)-1);

	if (iCateType == 1)
		CPythonNetworkStream::Instance().SendItemShopCateDeletePacket(table);
	else
		CPythonNetworkStream::Instance().SendItemShopSubCateDeletePacket(table);

	return Py_BuildNone();
}

PyObject * privateItemShopRenameCate(PyObject * poSelf, PyObject * poArgs)
{
	int iCateType;
	if (!PyTuple_GetInteger(poArgs, 0, &iCateType))
		return Py_BuildException();

	int iCateID;
	if (!PyTuple_GetInteger(poArgs, 1, &iCateID))
		return Py_BuildException();

	char* szRename;
	if (!PyTuple_GetString(poArgs, 2, &szRename))
		return Py_BadArgument();

	TGCIShopCateAdmin table;
	table.iCateID = iCateID;
	strncpy(table.szCategory, szRename, sizeof(table.szCategory)-1);
	CPythonNetworkStream::Instance().SendItemShopRenamePacket(table);
	return Py_BuildNone();
}
///###########################################################################################################///
PyObject * privateItemShopUpdateItem(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	// TraceError("Cate: %d", pItemData->cateID);
	// TraceError("SubCate: %d", pItemData->subCateID);

	TGCIShopItemAdmin table;
	table.iID = pItemData->itemID;
	table.iVnum = pItemData->vnum;
	table.iCount = pItemData->count;
	table.iPrice = pItemData->price;
	table.iDiscount = pItemData->discount;
	table.categoryID = pItemData->subCateID;
	strncpy(table.szAddTime, pItemData->addTime, sizeof(table.szAddTime)-1);
	strncpy(table.szOfferTime, pItemData->offerTime, sizeof(table.szOfferTime)-1);
	table.bTimeType = pItemData->timeType;
	table.bStatus = pItemData->status;
	table.iStock = pItemData->stock;
	table.wSellCount = pItemData->totalSold;
	table.bSellType = pItemData->sellType;

	for (int s = 0; s < ITEM_SOCKET_SLOT_MAX_NUM; s++)
	{
		table.alSockets[s] = pItemData->alSockets[s];
	}

	for (int at = 0; at < ITEM_ATTRIBUTE_SLOT_MAX_NUM; at++)
	{
		table.aAttr[at].bType = pItemData->aAttr[at].bType;
		table.aAttr[at].sValue = pItemData->aAttr[at].sValue;
	}

	CPythonNetworkStream::Instance().SendItemShopItemUpdatePacket(table);
	return Py_BuildNone();
}

PyObject * privateItemShopAddItem(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPrivateItemShop::TItemShopData * pItemData;
	if (!CPythonPrivateItemShop::Instance().GetItemDataPtr(0, &pItemData))
		return Py_BuildException();

	// TraceError("Cate: %d", pItemData->cateID);
	// TraceError("SubCate: %d", pItemData->subCateID);

	TGCIShopItemAdmin table;
	table.iID = pItemData->itemID;
	table.iVnum = pItemData->vnum;
	table.iCount = pItemData->count;
	table.iPrice = pItemData->price;
	table.iDiscount = pItemData->discount;
	table.categoryID = pItemData->subCateID;
	strncpy(table.szAddTime, pItemData->addTime, sizeof(table.szAddTime)-1);
	strncpy(table.szOfferTime, pItemData->offerTime, sizeof(table.szOfferTime)-1);
	table.bTimeType = pItemData->timeType;
	table.bStatus = pItemData->status;
	table.iStock = pItemData->stock;
	table.wSellCount = pItemData->totalSold;
	table.bSellType = pItemData->sellType;

	for (int s = 0; s < ITEM_SOCKET_SLOT_MAX_NUM; s++)
	{
		table.alSockets[s] = pItemData->alSockets[s];
	}

	for (int at = 0; at < ITEM_ATTRIBUTE_SLOT_MAX_NUM; at++)
	{
		table.aAttr[at].bType = pItemData->aAttr[at].bType;
		table.aAttr[at].sValue = pItemData->aAttr[at].sValue;
	}

	CPythonNetworkStream::Instance().SendItemShopItemAddPacket(table);
	return Py_BuildNone();
}

PyObject * privateItemShopDeleteItem(PyObject * poSelf, PyObject * poArgs)
{
	int iItemID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemID))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendItemShopItemDeletePacket((DWORD)(iItemID));
	return Py_BuildNone();
}
///###########################################################################################################///

void initprivateItemShop()
{
	static PyMethodDef s_methods[] =
	{
		{ "Clear",				privateItemShopClear,			METH_VARARGS },
		{ "Create",				privateItemShopCreate,			METH_VARARGS },
		{ "Initialize",			privateItemShopInitialize,		METH_VARARGS },
		{ "SetItemID",			privateItemShopSetItemID,		METH_VARARGS },
		{ "SetCateID",			privateItemShopSetCateID,		METH_VARARGS },
		{ "SetSubCateID",		privateItemShopSetSubCateID,	METH_VARARGS },
		{ "SetSellTypeID",		privateItemShopSetSellTypeID,	METH_VARARGS },
		{ "SetVnum",			privateItemShopSetVnum,			METH_VARARGS },
		{ "SetCount",			privateItemShopSetCount,		METH_VARARGS },
		{ "SetPrice",			privateItemShopSetPrice,		METH_VARARGS },
		{ "SetDiscount",		privateItemShopSetDiscount,		METH_VARARGS },
		{ "SetTimeType",		privateItemShopSetTimeType,		METH_VARARGS },
		{ "SetStatus",			privateItemShopSetStatus,		METH_VARARGS },
		{ "SetStock",			privateItemShopSetStock,		METH_VARARGS },
		{ "SetTotalSold",		privateItemShopSetTotalSold,	METH_VARARGS },
		{ "SetAddTime",			privateItemShopSetAddTime,		METH_VARARGS },
		{ "SetOfferTime",		privateItemShopSetOfferTime,	METH_VARARGS },
		{ "SetSocket",			privateItemShopSetSocket,		METH_VARARGS },
		{ "SetAttribute",		privateItemShopSetAttribute,	METH_VARARGS },

		{ "SendAddCate",		privateItemShopAddCate,			METH_VARARGS },
		{ "SendDeleteCate",		privateItemShopDeleteCate,		METH_VARARGS },
		{ "SendRenameCate",		privateItemShopRenameCate,		METH_VARARGS },

		{ "SendUpdateItem",		privateItemShopUpdateItem,		METH_VARARGS },
		{ "SendAddItem",		privateItemShopAddItem,			METH_VARARGS },
		{ "SendDeleteItem",		privateItemShopDeleteItem,		METH_VARARGS },

		{ NULL,					NULL,							NULL },
	};

	PyObject * poModule = Py_InitModule("privateItemShop", s_methods);

	PyModule_AddIntConstant(poModule, "EDIT_ITEM",		1);
	PyModule_AddIntConstant(poModule, "NEW_ITEM",		2);
	PyModule_AddIntConstant(poModule, "DELETE_ITEM",	3);

	PyModule_AddIntConstant(poModule, "ADD_CATE",		1);
	PyModule_AddIntConstant(poModule, "ADD_SUBCATE",	2);

	PyModule_AddIntConstant(poModule, "DELETE_CATE",	1);
	PyModule_AddIntConstant(poModule, "DELETE_SUBCATE",	2);

	PyModule_AddIntConstant(poModule, "RENAME_CATE",	1);
	PyModule_AddIntConstant(poModule, "RENAME_SUBCATE",	2);
}
#endif
