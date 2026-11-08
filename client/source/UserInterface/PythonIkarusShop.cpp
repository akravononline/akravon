#include "StdAfx.h"
#ifdef ENABLE_IKASHOP_RENEWAL
#include <fstream>
#include <ranges>
#include <msl/msl.h>

#include "../GameLib/ItemManager.h"
#include "../EterPack/EterPackManager.h"

#include "Packet.h"
#include "PythonNetworkStream.h"
#include "PythonTextTail.h"
#include "PythonPlayer.h"
#include "PythonIkarusShop.h"
#include "PythonCharacterManager.h"
#include "PythonSystem.h"

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
#include "PythonMiniMap.h"
#endif

#ifdef min
#undef min
#undef max
#endif

#include <rapidjson/rapidjson.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <fmt\fmt.h>

#include "JsonHelper.h"

using namespace ikashop::python;

DWORD ikashop::GetItemHash(const TShopPlayerItem& item)
{
	// checking type - skipping non equip items
	CItemData* table = nullptr;
	if (!CItemManager::instance().GetItemDataPointer(item.vnum, &table))
		return 0;

	switch(table->GetType())
	{
	case CItemData::ITEM_TYPE_ARMOR:
	case CItemData::ITEM_TYPE_WEAPON:
	case CItemData::ITEM_TYPE_COSTUME:
	case CItemData::ITEM_TYPE_BELT:
		break;

	default:
		return 0;
	}

	// copying values
	auto copy {item};

	// resetting non hashable values
	copy.expiration = {};
	copy.id = {};
	copy.owner = {};
	copy.pos = {};
	copy.window = {};

	// returning memory hash
	return GetCRC32(reinterpret_cast<const char*>(&copy), sizeof(copy));
}

#ifdef ENABLE_IKASHOP_ENTITIES
namespace ikashop
{
	ShopInstance::ShopInstance(SkinType Skin)
	{
		m_VirtualID = 0;
		m_Skin = Skin;
	}

	void ShopInstance::SetName(const SignString& str)
	{
		static CGraphicText* font = nullptr;
		if (!font)
			font = static_cast<CGraphicText*>(DefaultFont_GetResource());
		m_Text.SetValue(str.c_str());
		m_Text.SetTextPointer(font);
		m_Text.SetOutline(true);
		m_Text.SetHorizonalAlign(CGraphicTextInstance::EHorizontalAlign::HORIZONTAL_ALIGN_CENTER);
		m_Text.SetColor(0xFF7AE75D);
		// m_Text.Update();//Pazar Isimleri
	}

	const ShopInstance::SignString& ShopInstance::GetName()
	{
		return m_Text.GetValueStringReference();
	}

	void ShopInstance::Show(float x, float y, float z)
	{
		std::string model = "ikashop/lite/shop.gr2";

		m_GraphicThing.Clear();
		m_GraphicThing.ReserveModelThing(1);
		m_GraphicThing.ReserveModelInstance(1);
		m_GraphicThing.RegisterModelThing(0, (CGraphicThing*)CResourceManager::Instance().GetResourcePointer(model.c_str()));
		m_GraphicThing.SetModelInstance(0, 0, 0);

		m_GraphicThing.SetPosition(x, -y, z);
		m_GraphicThing.Show();
		m_GraphicThing.Update();
		m_GraphicThing.Transform();
		m_GraphicThing.Deform();

		m_TextHeight = m_GraphicThing.GetHeight() + 10;
		if (!IsSkin()) { m_TextHeight -= 50; }

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		m_pos.x = x, m_pos.y = y, m_pos.z = z;
		#endif
	}

	void ShopInstance::Clear()
	{
		m_GraphicThing.Clear();
		m_VirtualID = 0;
		m_Skin = 0;
	}

	void ShopInstance::Render()
	{
		m_GraphicThing.Render();
	}

	void ShopInstance::RenderText()
	{
		if (m_GraphicThing.isShow())
			m_Text.Render();
	}

	void ShopInstance::BlendRender()
	{
		m_GraphicThing.BlendRender();
	}

	void ShopInstance::Update()
	{
		m_GraphicThing.Deform();
		m_GraphicThing.Update();

		// update and render text tail
		CPythonGraphic& rpyGraphic = CPythonGraphic::Instance();
		rpyGraphic.Identity();
		float x = 0, y = 0, z = 0;
		const D3DXVECTOR3& c_rv3Position = m_GraphicThing.GetPosition();
		rpyGraphic.ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z + m_TextHeight, &x, &y, &z);
		x = floorf(x);
		y = floorf(y);
		z *= CPythonGraphic::Instance().GetOrthoDepth() * -1.0f;
		z += 15.0f;
		m_Text.SetPosition(x, y, z);
	}

	void ShopInstance::UpdateTextTail()
	{
		// update and render text tail
		CPythonGraphic& rpyGraphic = CPythonGraphic::Instance();
		rpyGraphic.Identity();
		float x = 0, y = 0, z = 0;
		const D3DXVECTOR3& c_rv3Position = m_GraphicThing.GetPosition();
		rpyGraphic.ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z + m_TextHeight, &x, &y, &z);

		x = floorf(x);
		y = floorf(y);
		z *= CPythonGraphic::Instance().GetOrthoDepth() * -1.0f;
		z += 15.0f;
		m_Text.SetPosition(x, y, z);
	}

	bool ShopInstance::IsPickable()
	{
		return m_GraphicThing.isShow();
	}

	bool ShopInstance::IsPicked()
	{
		float u, v, t;
		return m_GraphicThing.Intersect(&u, &v, &t);
	}
}
#endif

CPythonIkarusShop::CPythonIkarusShop()
{
	m_businessBoard = nullptr;
	#ifdef ENABLE_IKASHOP_ENTITIES
	m_bIsShowName = false;
	#endif

	// checking folder
	DWORD attr = ::GetFileAttributesA("shop");
	if (attr == INVALID_FILE_ATTRIBUTES || attr != FILE_ATTRIBUTE_DIRECTORY)
		::CreateDirectoryA("shop", 0);

	LoadFilterHistory();
}

void CPythonIkarusShop::SetSearchShopBoard(PyObject* poWindow)
{
	m_searchShopBoard = poWindow;
}

//starts
void CPythonIkarusShop::SetBusinessBoard(PyObject* poWindow)
{
	m_businessBoard = poWindow;
}

PyObject* CPythonIkarusShop::GetBusinessBoard()
{
	return m_businessBoard;
}

bool CPythonIkarusShop::CallOnBusinessBoard(const std::string& func, PyObject* args)
{
	if (args == nullptr)
		args = PyTuple_New(0);

	bool ret = false;
	if (m_businessBoard != nullptr)
		PyCallClassMemberFunc(m_businessBoard, func.c_str(), args, &ret);
	return ret;
}

bool CPythonIkarusShop::CallOnSearchShopBoard(const std::string& func, PyObject* args)
{
	if (args == nullptr)
		args = PyTuple_New(0);

	bool ret = false;
	if (m_searchShopBoard != nullptr)
		PyCallClassMemberFunc(m_searchShopBoard, func.c_str(), args, &ret);
	return ret;
}

void CPythonIkarusShop::OpenShop(const ikashop::TShopInfo& shop, const std::vector<ikashop::TShopItem>& vec)
{
	// making shop dict
	PythonSafeDict shopDict;
	shopDict.set("id", shop.ownerid);
	shopDict.set("duration", shop.duration);
	shopDict.set("name", shop.name);
	shopDict.set("items", ExtractPythonShopItemList(vec));

	CallOnSearchShopBoard("OpenShopGuest", Py_BuildValue("(O)", shopDict.weakRef()));
}

void CPythonIkarusShop::OpenShopOwner(const ikashop::TShopInfo& shop, const std::vector<ikashop::TShopItem>& vec)
{
	// making shop dict
	PythonSafeDict shopDict;
	shopDict.set("duration", shop.duration);
	shopDict.set("name", shop.name);
	shopDict.set("items", ExtractPythonShopItemList(vec));

	CallOnBusinessBoard("OpenShopOwner", Py_BuildValue("(O)", shopDict.weakRef()));
}

void CPythonIkarusShop::OpenShopOwnerNoShop()
{
	CallOnBusinessBoard("OpenShopOwnerEmpty" );
}

void CPythonIkarusShop::ShopClose()
{
	CallOnBusinessBoard("ShopClose" );
}

void CPythonIkarusShop::ShopExpiredGuesting(DWORD ownerid)
{
	CallOnSearchShopBoard("ShopExpiredGuesting", Py_BuildValue("(i)", ownerid));
}

void CPythonIkarusShop::ShopRemoveItem(bool owner, DWORD itemid)
{
	if (owner)
		CallOnBusinessBoard("ShopOwnerRemoveItem", Py_BuildValue("(i)", itemid));
	else
		CallOnSearchShopBoard("ShopGuestRemoveItem", Py_BuildValue("(i)", itemid));
}

void CPythonIkarusShop::ShopGuestEditItem(DWORD itemid, const ikashop::TPriceInfo& price)
{
	ikashop::python::PythonSafeDict dict;
	dict.set("price", price.yang);
	#ifdef ENABLE_CHEQUE_SYSTEM
	dict.set("cheque", price.cheque);
	#endif

	CallOnSearchShopBoard("ShopGuestEditItem", Py_BuildValue("(iO)", itemid, dict.weakRef()));
}

void CPythonIkarusShop::ShopOwnerEditItem(DWORD itemid, const ikashop::TPriceInfo& price)
{
	ikashop::python::PythonSafeDict dict;
	dict.set("price", price.yang);
	#ifdef ENABLE_CHEQUE_SYSTEM
	dict.set("cheque", price.cheque);
	#endif

	CallOnBusinessBoard("ShopOwnerEditItem", Py_BuildValue("(iO)", itemid, dict.weakRef()));
}

void CPythonIkarusShop::ShopFilterResult(const std::vector<ikashop::TSearchResultInfo>& vec)
{
	static auto extractOne = [](const ikashop::TSearchResultInfo& data)
	{
		auto dict = ExtractPythonDictShopItem(data);
		dict.set("is_auction", data.auction);
		dict.set("seller_name", data.seller_name);
		dict.set("duration", data.duration);
		return dict;
	};

	auto itemList = ExtractPythonListFromContainer(vec, extractOne);
	CallOnSearchShopBoard("SetSearchResultItems", Py_BuildValue("(O)", itemList.weakRef()));
}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
void CPythonIkarusShop::ShopFilterNameResult(const std::vector<ikashop::TSearchNameResultInfo>& vec)
{
	if (m_shopInstances.empty() || vec.empty())
		return;

	CPythonMiniMap::Instance().ClearAllSignalPoint();
	m_shopSearchList.clear();

	for (const auto& iter : vec)
	{
		for (auto it = m_shopInstances.begin(); it != m_shopInstances.end(); it++)
		{
			ikashop::ShopInstance& shop = *(*it);
			if (shop.GetVID() != iter.dwShopVID) { continue; }

			m_shopSearchList.insert(shop.GetVID());

			ikashop::PIXEL_POSITION pos = shop.GetXYZ();

			CPythonBackground::TSearchPos info;
			info.x = pos.x; info.y = pos.y; info.z = pos.z;
			CPythonBackground::Instance().CreatePrivateShopPos(shop.GetVID(), &info);

			CPythonMiniMap::Instance().AddSignalPointNew(shop.GetVID(), pos.x, pos.y);
		}
	}

	CPythonMiniMap::Instance().OpenAtlasWindow();
}

void CPythonIkarusShop::ShopFilterNameClear()
{
	CPythonMiniMap::Instance().ClearAllSignalPoint();

	m_shopSearchList.clear();
	for (auto itModel = m_shopInstances.begin(); itModel != m_shopInstances.end(); itModel++)
	{
		ikashop::ShopInstance& shop = *(*itModel);

		auto pModel = shop.GetThingInstancePtr();
		if (pModel)
		{
			if (!pModel->isShow())
			{
				pModel->Show();
				pModel->Update();
				pModel->Transform();
				pModel->Deform();
			}
		}
	}
}
#endif

void CPythonIkarusShop::OfferListReceived(const std::vector<ikashop::TOfferListElement>& offers)
{
	static auto extractOffer = [](auto&& offer)
	{
		auto item = ExtractPythonDictShopItem(offer.item);

		PythonSafeDict offerDict;
		offerDict.set("name", offer.shopname);
		offerDict.set("id", offer.offerid);
		offerDict.set("buyer_name", offer.buyername);
		offerDict.set("ownerid", offer.ownerid);
		offerDict.set("itemid", offer.itemid);
		offerDict.set("price", offer.price.yang);
		offerDict.set("notified", offer.notified);
		offerDict.set("accepted", offer.accepted);
		offerDict.set("item", item);
		offerDict.set("incoming", offer.incoming);
		offerDict.set("datetime", offer.datetime);
		return offerDict;
	};

	auto offerList = ExtractPythonListFromContainer(offers, extractOffer);
	CallOnBusinessBoard("SetOffers", Py_BuildValue("(O)", offerList.weakRef()));
}

void CPythonIkarusShop::SafeboxRefresh(const ikashop::TValutesInfo& valute, const std::vector<ikashop::TShopPlayerItem>& items)
{
	auto itemList = ExtractPythonPlayerItemList(items);
	CallOnBusinessBoard("SetupSafebox", Py_BuildValue("(KO)", valute.yang, itemList.weakRef()));
}

void CPythonIkarusShop::SafeboxRemoveItem(DWORD itemid)
{
	CallOnBusinessBoard("SafeboxRemoveItem", Py_BuildValue("(i)", itemid));
}

void CPythonIkarusShop::SafeboxAddItem(const ikashop::TShopPlayerItem& item)
{
	auto itemDict = ikashop::python::ExtractPythonDictPlayerItem(item);
	CallOnBusinessBoard("SafeboxAddItem", Py_BuildValue("(O)", itemDict.weakRef()));
}

void CPythonIkarusShop::AuctionList(const std::vector<ikashop::TAuctionListElement>& auctions)
{
	static auto extractAuctionListElement = [](auto&& element)
	{
		auto auctionDict = ExtractPythonDictShopItem(element.auction);
		auctionDict.set("seller_name", element.auction.ownername);
		auctionDict.set("duration", element.auction.duration);
		auctionDict.set("price", element.actual_best.yang);
		auctionDict.set("is_auction", true);
		return auctionDict;
	};

	auto auctionList = ExtractPythonListFromContainer(auctions, extractAuctionListElement);
	CallOnSearchShopBoard("SetAuctionList", Py_BuildValue("(O)", auctionList.weakRef()));
}

void CPythonIkarusShop::MyAuctionOpen(const ikashop::TAuctionInfo& auction, const std::vector<ikashop::TAuctionOfferInfo>& offers)
{
	static auto extractAutionOffer = [](auto&& offer)
	{
		PythonSafeDict offerDict;
		offerDict.set("buyerid", offer.buyerid);
		offerDict.set("buyername", offer.buyername);
		offerDict.set("offerid", offer.ownerid);
		offerDict.set("price", offer.price.yang);
		offerDict.set("datetime", offer.datetime);
		return offerDict;
	};

	auto auctionDict = ExtractPythonDictShopItem(auction);
	auctionDict.set("duration", auction.duration);
	auctionDict.set("offers", ExtractPythonListFromContainer(offers, extractAutionOffer));

	CallOnBusinessBoard("SetupAcutionOwner", Py_BuildValue("(O)", auctionDict.weakRef()));
}

void CPythonIkarusShop::AuctionOpen(const ikashop::TAuctionInfo& auction, const std::vector<ikashop::TAuctionOfferInfo>& offers)
{
	static auto extractAutionOffer = [](auto&& offer)
	{
		PythonSafeDict offerDict;
		offerDict.set("buyerid", offer.buyerid);
		offerDict.set("buyername", offer.buyername);
		offerDict.set("offerid", offer.ownerid);
		offerDict.set("price", offer.price.yang);
		offerDict.set("datetime", offer.datetime);
		return offerDict;
	};

	auto auctionDict = ExtractPythonDictShopItem(auction);
	auctionDict.set("ownername", auction.ownername);
	auctionDict.set("duration", auction.duration);
	auctionDict.set("offers", ExtractPythonListFromContainer(offers, extractAutionOffer));

	CallOnSearchShopBoard("OpenAuctionGuest", Py_BuildValue("(O)", auctionDict.weakRef()));
}

void CPythonIkarusShop::OpenMyAuctionNoAuction()
{
	CallOnBusinessBoard("OpenCreateAuctionQuestion");
}

//filter caching methods
void CPythonIkarusShop::LoadFilterHistory()
{
	// opening stream
	std::ifstream stream("data/shop/search_history.json", std::ios::in | std::ios::ate | std::ios::binary);
	if (stream.is_open() == false)
		return;

	// reading size
	auto fsize = stream.tellg();

	// resetting reading point
	stream.seekg(0);

	// making string receiving file content
	std::string jsonString;
	jsonString.resize(fsize);

	// reading file content inside string
	stream.read(jsonString.data(), fsize);
	stream.close();

	// making document
	rapidjson::Document jsonDoc;
	jsonDoc.Parse(jsonString.c_str());

	// checking for parser errors
	if (jsonDoc.HasParseError())
	{
		TraceError("PARSER ERROR PARSING SEARCH HISTORY FILE: %d", (int) jsonDoc.GetParseError());
		return;
	}

	// checking list object
	if (jsonDoc.IsArray() == false)
	{
		TraceError("INVALID SEARCH HISTORY FILE: document.IsArray() == false");
		return;
	}

	// reserving dimension
	m_filterHistory.clear();
	m_filterHistory.reserve(jsonDoc.Size());

	// iterating over array
	for (auto& member : jsonDoc.GetArray())
	{
		auto& history = m_filterHistory.emplace_back();
		// decoding info
		history.count = jsonHelper::getValue<int>(member, "count");

		// decoding datetime
		history.datetime = jsonHelper::getValue<int64_t>(member, "datetime");

		// decoding filter name
		auto name = jsonHelper::getValue<std::string>(member, "item_name");
		snprintf(history.name, sizeof(history.name), "%s", name.c_str());

		// decoding filter
		history.type = jsonHelper::getValue<int>(member, "type");
		history.subtype = jsonHelper::getValue<int>(member, "subtype");
		history.levelstart = jsonHelper::getValue<int>(member, "level_start");
		history.levelend = jsonHelper::getValue<int>(member, "level_end");
		history.countstart = jsonHelper::getValue<int>(member, "count_start");
		history.countend = jsonHelper::getValue<int>(member, "count_end");
		history.pricestart.yang = jsonHelper::getValue<int64_t>(member, "price_start");
		history.pricend.yang = jsonHelper::getValue<int64_t>(member, "price_end");
		history.sashGrade = jsonHelper::getValue<int>(member, "sash");
		history.alchemyGrade = jsonHelper::getValue<int>(member, "alchemy");

		for (int i = 0; i < ITEM_ATTRIBUTE_SLOT_NORM_NUM; i++)
		{
			auto& attr = history.attrs[i];
			attr.bType = jsonHelper::getValue<int>(member, fmt::format("attrtype{}", i));
			attr.sValue = jsonHelper::getValue<int>(member, fmt::format("attrvalue{}", i));
		}
	}
}

void CPythonIkarusShop::SaveFilterHistory()
{
	// creating document
	rapidjson::Document doc;
	doc.SetArray();

	// reserving array size
	doc.GetArray().Reserve(m_filterHistory.size(), doc.GetAllocator());

	// populating array
	for (auto& history : m_filterHistory)
	{
		rapidjson::Value member;
		member.SetObject();

		// encoding info
		jsonHelper::insertValue(member, "count", static_cast<int>(history.count), doc.GetAllocator());
		// encoding datetime
		jsonHelper::insertValue(member, "datetime", static_cast<int64_t>(history.datetime), doc.GetAllocator());
		// encoding filter
		jsonHelper::insertValue(member, "item_name", std::string(history.name), doc.GetAllocator());
		jsonHelper::insertValue(member, "type", static_cast<int>(history.type), doc.GetAllocator());
		jsonHelper::insertValue(member, "subtype", static_cast<int>(history.subtype), doc.GetAllocator());
		jsonHelper::insertValue(member, "level_start", static_cast<int>(history.levelstart), doc.GetAllocator());
		jsonHelper::insertValue(member, "level_end", static_cast<int>(history.levelend), doc.GetAllocator());
		jsonHelper::insertValue(member, "count_start", static_cast<int>(history.countstart), doc.GetAllocator());
		jsonHelper::insertValue(member, "count_end", static_cast<int>(history.countend), doc.GetAllocator());
		jsonHelper::insertValue(member, "price_start", static_cast<int64_t>(history.pricestart.yang), doc.GetAllocator());
		jsonHelper::insertValue(member, "price_end", static_cast<int64_t>(history.pricend.yang), doc.GetAllocator());
		jsonHelper::insertValue(member, "alchemy", static_cast<int>(history.alchemyGrade), doc.GetAllocator());
		jsonHelper::insertValue(member, "sash", static_cast<int>(history.sashGrade), doc.GetAllocator());

		for (int i =0; i < ITEM_ATTRIBUTE_SLOT_NORM_NUM; i++)
		{
			auto& attr = history.attrs[i];
			jsonHelper::insertValue(member, fmt::format("attrtype{}", i), static_cast<int>(attr.bType), doc.GetAllocator());
			jsonHelper::insertValue(member, fmt::format("attrvalue{}", i), static_cast<int>(attr.sValue), doc.GetAllocator());
		}

		// pushing element inside the array
		doc.PushBack(member, doc.GetAllocator());
	}

	// making json string
	rapidjson::StringBuffer buffer;
	buffer.Clear();

	rapidjson::Writer<rapidjson::StringBuffer> StringWriter(buffer);
	doc.Accept(StringWriter);

	// opening stream
	std::ofstream stream("data/shop/search_history.json", std::ios::out | std::ios::binary);
	if (!stream.is_open())
	{
		TraceError("CANNOT SAVE SEARCH HISTORY FILE!");
		return;
	}

	// completing stream data
	stream.write(buffer.GetString(), buffer.GetSize());
	stream.close();
}

void CPythonIkarusShop::AddNewFilterToHistory(TFilterHistoryElement& element)
{
	m_filterHistory.emplace_back(element);
	while (m_filterHistory.size() > ikashop::OFFLINESHOP_MAX_FILTER_HISTORY_SIZE)
		PopOldestFilterHistoryElement();
	SaveFilterHistory();
}

void CPythonIkarusShop::PopOldestFilterHistoryElement()
{
	if (!m_filterHistory.empty())
		m_filterHistory.erase(std::min_element(m_filterHistory.begin(), m_filterHistory.end()));
}

PythonSafeList CPythonIkarusShop::GetFilterHistory()
{
	static auto extractFilter = [](const TFilterHistoryElement& filter){
		PythonGarbageCollector collector;
		PythonSafeList attrlist;
		for (auto& attr : filter.attrs)
			attrlist.append(collector.makeValue("(ii)", attr.bType, attr.sValue));

		PythonSafeDict filterDict;
		filterDict.set("datetime", filter.datetime);
		filterDict.set("count", filter.count);
		filterDict.set("type", filter.type);
		filterDict.set("subtype", filter.subtype);
		filterDict.set("name", filter.name);
		filterDict.set("price_min", filter.pricestart.yang);
		filterDict.set("price_max", filter.pricend.yang);
		filterDict.set("level_min", filter.levelstart);
		filterDict.set("level_max", filter.levelend);
		filterDict.set("count_min", filter.countstart);
		filterDict.set("count_max", filter.countend);
		filterDict.set("alchemy", filter.alchemyGrade);
		filterDict.set("sash", filter.sashGrade);
		filterDict.set("level_max", filter.levelend);
		filterDict.set("attrs", attrlist);
		return filterDict;
	};

	return ExtractPythonListFromContainer(m_filterHistory, extractFilter);
}

std::vector<std::string> CPythonIkarusShop::GetItemNames()
{
	// getting item map
	std::map<DWORD, std::string> nameMap;
	CItemManager::instance().GetItemsNameMap(nameMap);
	// making vector
	std::vector<std::string> vec;
	vec.reserve(nameMap.size());
	for (auto& [vnum, name] : nameMap)
		vec.emplace_back(name);
	return vec;
}

void CPythonIkarusShop::ShopBuilding_AddItem(int win, int slot)
{
	CallOnBusinessBoard("InsertItemInSlot" , Py_BuildValue("(ii)", win, slot));
}

void CPythonIkarusShop::AuctionBuilding_AddItem(int win, int slot)
{
	CallOnBusinessBoard("AuctionInsertItemInSlot" , Py_BuildValue("(ii)", win, slot));
}

#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
void CPythonIkarusShop::UpdateAvarageAndMinMaxPrice(const long long avgPrice, const long long minPrice, const long long maxPrice)
{
	CallOnBusinessBoard("UpdateAvgAndMinMaxPrice", Py_BuildValue("(LLL)", avgPrice, minPrice, maxPrice));
}
#endif

void CPythonIkarusShop::RefreshBoardCounters(int safebox, int auction, int offers)
{
	CallOnBusinessBoard("SetSafeboxCounter", Py_BuildValue("(i)", safebox));
	CallOnBusinessBoard("SetAuctionCounter", Py_BuildValue("(i)", auction));
	CallOnBusinessBoard("SetOffersCounter", Py_BuildValue("(i)", offers));
}

#ifdef ENABLE_IKASHOP_ENTITIES
std::string GetEntityNameFormat(const char* name)
{
	// getting localeinfo module
	auto pyModule = PythonReference::steal(PyImport_ImportModule("localeInfo"));
	if (!pyModule)
		return name;

	// getting attr name
	const std::string attr = "IKASHOP_SHOP_NAME_FORMAT";

	// checking module has attr
	if (!PyObject_HasAttrString(pyModule, attr.c_str()))
		return name;

	// getting module attribute
	auto pyAttr = PythonReference::steal(PyObject_GetAttrString(pyModule, attr.c_str()));
	if (!pyAttr)
		return name;

	// checking attribute is string
	if (!PyString_Check(pyAttr))
		return name;

	// getting string from attr
	const auto* format = PyString_AsString(pyAttr);
	return fmt::format(fmt::runtime(format), name);
}

void CPythonIkarusShop::InsertEntity(DWORD vid, int type, const char* name, LONG x, LONG y, LONG z)
{
	std::string shoptitle = name;
	std::string delimiter = "@";

	size_t pos = 0;
	std::string shopname;
	if ((pos = shoptitle.find(delimiter)) != std::string::npos)
	{
		shopname = shoptitle.substr(0, pos);
		shoptitle.erase(0, pos+delimiter.length());
	}

	// auto namestr = GetEntityNameFormat(name);
	auto namestr = GetEntityNameFormat(shopname.c_str());
	auto& shop = m_shopInstances.emplace_back(std::make_unique<ikashop::ShopInstance>(type));
	shop->SetVID(vid);
	shop->SetShopType(type);
	shop->SetName(namestr);

	CPythonBackground&rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.GlobalPositionToLocalPosition(x, y);

	z = CPythonBackground::Instance().GetHeight(x, y) + 10.0f;
	shop->Show(x,y,z);

	//making shop texttail
	CPythonTextTail::instance().RegisterShopInstanceTextTail(vid, shoptitle.c_str(), shop->GetThingInstancePtr());
}

void CPythonIkarusShop::RemoveEntity(DWORD vid)
{
	for (auto it = m_shopInstances.begin(); it != m_shopInstances.end(); it++)
	{
		ikashop::ShopInstance& shop = *(*it);
		if (shop.GetVID() == vid)
		{
			shop.Clear();
			//deleting texttail
			CPythonTextTail::Instance().DeleteShopTextTail(vid);
			m_shopInstances.erase(it);
			return;
		}
	}
}

void CPythonIkarusShop::RenderEntities()
{
	CPythonGraphic::Instance().SetDiffuseOperation();
	for (auto & iter : m_shopInstances)
	{
		iter->Render();
		iter->BlendRender();
	}

	if (CPythonSystem::Instance().IsShowSalesText())
	{
		auto& graphic = CPythonGraphic::Instance();
		graphic.PushState();
		graphic.SetInterfaceRenderState();
		for (auto& instance : m_shopInstances)
		{
			instance->RenderText();
		}
		graphic.PopState();
		graphic.SetGameRenderState();
	}
}

void CPythonIkarusShop::UpdateEntities()
{
	for (auto & iter : m_shopInstances)
	{
		iter->Update();

		auto pModel = iter->GetThingInstancePtr();
		if (!pModel) { continue; }

		if (!m_shopSearchList.empty())
		{
			if (m_shopSearchList.find(iter->GetVID()) != m_shopSearchList.end())
			{
				if (!pModel->isShow())
				{
					pModel->Show();
					pModel->Update();
					pModel->Transform();
					pModel->Deform();
				}
			}
			else
			{
				if (pModel->isShow())
				{
					pModel->Hide();
					pModel->Update();
					pModel->Transform();
					pModel->Deform();
				}
			}
		}
	}
}

void CPythonIkarusShop::UpdateShowingTextTail()
{
	for (auto& iter : m_shopInstances)
		iter->UpdateTextTail();
}

bool CPythonIkarusShop::GetShowNameFlag()
{
	return m_bIsShowName;
}

void CPythonIkarusShop::SetShowNameFlag(bool flag)
{
	m_bIsShowName = flag;
}

void CPythonIkarusShop::DeleteEntities()
{
	for (auto& iter : m_shopInstances)
	{
		//deleting texttail
		CPythonTextTail::instance().DeleteShopTextTail(iter->GetVID());
		iter->Clear();
	}

	m_shopInstances.clear();
}

bool CPythonIkarusShop::CheckPickedShopEntity(DWORD* ret)
{
	for (auto& instance : m_shopInstances)
	{
		if (instance->IsPickable())
		{
			if (instance->IsPicked())
			{
				if (ret)
					*ret = instance->GetVID();
				return true;
			}
		}
	}
	return false;
}
#endif

void CPythonIkarusShop::DeleteSearchResult(DWORD itemid)
{
	CallOnSearchShopBoard("DeleteSearchResultItem", Py_BuildValue("(i)", itemid));
}

void CPythonIkarusShop::PopupMessage(const std::string& message)
{
	CallOnBusinessBoard("ServerPopupMessage", Py_BuildValue("(s)", message.c_str()));
}

CPythonIkarusShop::~CPythonIkarusShop()
{
}

PyObject* ikashopSendShopCreate(PyObject* poSelf, PyObject* poArgs)
{
	char* pszName = nullptr;
	if (!PyTuple_GetString(poArgs, 0, &pszName))
		return Py_BadArgument();

	ikashop::TShopInfo shopInfo{};
	strncpy(shopInfo.name, pszName, sizeof(shopInfo.name));
	CPythonNetworkStream::instance().SendIkarusShopCreateShop(shopInfo);
	return Py_BuildNone();
}

PyObject* ikashopSendForceCloseShop(PyObject* poSelf , PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopForceCloseShop();
	return Py_BuildNone();
}

PyObject* ikashopSendOpenShop(PyObject* poSelf , PyObject* poArgs)
{
	uint32_t id = 0;
	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &id))
		return Py_BadArgument();

	CPythonNetworkStream::Instance().SendIkarusShopOpenShop(id);
	return Py_BuildNone();
}

PyObject* ikashopSendOpenShopOwner(PyObject* poSelf , PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendIkarusShopOpenShopOwner();
	return Py_BuildNone();
}

PyObject* ikashopSendBuyItem(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t ownerid = 0;
	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &ownerid))
		return Py_BadArgument();

	uint32_t itemid = 0;
	if (!PyTuple_GetUnsignedInteger(poArgs, 1, &itemid))
		return Py_BadArgument();

	long long seenprice = 0;
	if (!PyTuple_GetLongLong(poArgs, 2, &seenprice))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopBuyItem(ownerid, itemid, false, seenprice);
	return Py_BuildNone();
}

PyObject* ikashopSendAddItem(PyObject* poSelf , PyObject* poArgs)
{
	BYTE window = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &window))
		return Py_BadArgument();

	WORD slot = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &slot))
		return Py_BadArgument();

	int destpos = 0;
	if (!PyTuple_GetInteger(poArgs, 2, &destpos))
		return Py_BadArgument();

	long long yang=0;
	if (!PyTuple_GetLongLong(poArgs, 3, &yang))
		return Py_BadArgument();

	#ifdef ENABLE_CHEQUE_SYSTEM
	int cheque = 0;
	if (!PyTuple_GetInteger(poArgs, 4, &cheque))
		return Py_BadArgument();
	#endif

	ikashop::TShopItemInfo info;
	info.pos = {window, slot};
	info.price.yang = yang;
	info.destpos = destpos;
	#ifdef ENABLE_CHEQUE_SYSTEM
	info.price.cheque = cheque;
	#endif

	CPythonNetworkStream::instance().SendIkarusShopAddItem(info);
	return Py_BuildNone();
}

PyObject* ikashopSendRemoveItem(PyObject* poSelf , PyObject* poArgs)
{
	uint32_t itemid = 0;
	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &itemid))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopRemoveItem(itemid);
	return Py_BuildNone();
}

PyObject* ikashopSendEditItem(PyObject* poSelf , PyObject* poArgs)
{
	uint32_t itemid=0;
	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &itemid))
		return Py_BadArgument();

	long long yang=0;
	if (!PyTuple_GetLongLong(poArgs, 1, &yang))
		return Py_BadArgument();

	#ifdef ENABLE_CHEQUE_SYSTEM
	int cheque = 0;
	if (!PyTuple_GetInteger(poArgs, 2, &cheque))
		return Py_BadArgument();
	#endif

	ikashop::TPriceInfo price;
	price.yang = yang;
	#ifdef ENABLE_CHEQUE_SYSTEM
	price.cheque = cheque;
	#endif

	CPythonNetworkStream::instance().SendOfflineShopEditItem(itemid, price);
	return Py_BuildNone();
}

PyObject* ikashopSendFilterRequest(PyObject* poSelf , PyObject* poArgs)
{
	char* name = nullptr;
	ikashop::TFilterInfo info;

	if (!PyTuple_GetString(poArgs, 0, &name))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 1, &info.type))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 2, &info.subtype))
		return Py_BadArgument();

	PyObject* prices = nullptr;
	if (!PyTuple_GetObject(poArgs, 3, &prices))
		return Py_BadArgument();

	PyObject* levels = nullptr;
	if (!PyTuple_GetObject(poArgs, 4, &levels))
		return Py_BadArgument();

	PyObject* counts = nullptr;
	if (!PyTuple_GetObject(poArgs, 5, &counts))
		return Py_BadArgument();

	PyObject* attrs = nullptr;
	if (!PyTuple_GetObject(poArgs, 6, &attrs))
		return Py_BadArgument();

	PyObject* specials = nullptr;
	if (!PyTuple_GetObject(poArgs, 7, &specials))
		return Py_BadArgument();

	// extracting prices
	if (!PyTuple_GetLongLong(prices, 0, &info.pricestart.yang))
		return Py_BadArgument();

	if (!PyTuple_GetLongLong(prices, 1, &info.pricend.yang))
		return Py_BadArgument();

	// extracting levels
	if (!PyTuple_GetInteger(levels, 0, &info.levelstart))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(levels, 1, &info.levelend))
		return Py_BadArgument();

	// extracting counts
	if (!PyTuple_GetInteger(counts, 0, &info.countstart))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(counts, 1, &info.countend))
		return Py_BadArgument();

	// extracting attrs
	for (int i = 0; i < ITEM_ATTRIBUTE_SLOT_NORM_NUM; i++)
	{
		PyObject* attribute = nullptr;
		if (!PyTuple_GetObject(attrs, i, &attribute))
			return Py_BadArgument();

		if (!PyTuple_GetInteger(attribute, 0, &info.attrs[i].bType))
			return Py_BadArgument();

		int value = 0;
		if (!PyTuple_GetInteger(attribute, 1, &value))
			return Py_BadArgument();

		info.attrs[i].sValue = static_cast<short>(value);
	}

	// extracting specials
	if (!PyTuple_GetInteger(specials, 0, &info.sashGrade))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(specials, 1, &info.alchemyGrade))
		return Py_BadArgument();

	// copying name
	strncpy(info.name, name, sizeof(info.name));

	CPythonNetworkStream::instance().SendIkarusShopFilterRequest(info);
	return Py_BuildNone();
}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
PyObject* ikashopSendFilterNameRequest(PyObject* poSelf, PyObject* poArgs)
{
	char* szName = nullptr;
	ikashop::TFilterName info;

	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BadArgument();

	if (!PyTuple_GetUnsignedLong(poArgs, 1, &info.dwVnum))
		return Py_BadArgument();

	if (!PyTuple_GetLong(poArgs, 2, &info.lSocket))
		return Py_BadArgument();

	// copying name
	strncpy(info.szName, szName, sizeof(info.szName));

	CPythonNetworkStream::instance().SendIkarusShopFilterNameRequest(info);
	return Py_BuildNone();
}

PyObject* ikashopSendFilterNameClear(PyObject* poSelf, PyObject* poArgs)
{
	CPythonIkarusShop::instance().ShopFilterNameClear();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
PyObject* ikashopSendAvarageAndMinMaxPricePacket(PyObject* poSelf , PyObject* poArgs)
{
	DWORD vnum;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &vnum))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopAvarageAndMinMaxPrice(static_cast<uint32_t>(vnum));
	return Py_BuildNone();
}
#endif

PyObject* ikashopSendRandomSearchFillRequest(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopSearchFillRequest();
	return Py_BuildNone();
}

PyObject* ikashopSendOfferCreate(PyObject* poSelf , PyObject* poArgs)
{
	ikashop::TOfferInfo offer;

	if (!PyTuple_GetUnsignedLong(poArgs, 0, &offer.ownerid))
		return Py_BadArgument();

	if (!PyTuple_GetUnsignedLong(poArgs, 1, &offer.itemid))
		return Py_BadArgument();

	if (!PyTuple_GetLongLong(poArgs, 2, &offer.price.yang))
		return Py_BadArgument();

	#ifdef ENABLE_CHEQUE_SYSTEM
	if (!PyTuple_GetInteger(poArgs, 3, &offer.price.cheque))
		return Py_BadArgument();
	#endif

	CPythonNetworkStream::instance().SendIkarusShopOfferCreate(offer);
	return Py_BuildNone();
}

PyObject* ikashopSendOfferAccept(PyObject* poSelf , PyObject* poArgs)
{
	DWORD offerid=0;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &offerid))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopOfferAccept(offerid);
	return Py_BuildNone();
}

PyObject* ikashopSendOfferCancel(PyObject* poSelf , PyObject* poArgs)
{
	DWORD offerid = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &offerid))
		return Py_BadArgument();

	DWORD ownerid = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 1, &ownerid))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopOfferCancel(offerid, ownerid);
	return Py_BuildNone();
}

PyObject* ikashopSendOfferListRequest(PyObject* poSelf , PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopOfferListRequest();
	return Py_BuildNone();
}

PyObject* ikashopSendSafeboxOpen(PyObject* poSelf , PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopSafeboxOpen();
	return Py_BuildNone();
}

PyObject* ikashopSendSafeboxGetItem(PyObject* poSelf , PyObject* poArgs)
{
	DWORD itemid = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &itemid))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopSafeboxGetItem(itemid);
	return Py_BuildNone();
}

PyObject* ikashopSendSafeboxGetValutes(PyObject* poSelf , PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopSafeboxGetValutes();
	return Py_BuildNone();
}

PyObject* ikashopSendSafeboxClose(PyObject* poSelf , PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopSafeboxClose();
	return Py_BuildNone();
}

PyObject* ikashopSendOffersListClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusOffersListClose();
	return Py_BuildNone();
}

PyObject* ikashopSendCloseMyShopBoard(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusMyShopCloseBoard();
	return Py_BuildNone();
}

PyObject* ikashopSendCloseShopGuestBoard(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusCloseShopGuestBoard();
	return Py_BuildNone();
}

PyObject* ikashopSendAuctionListRequest(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopAuctionListRequest();
	return Py_BuildNone();
}

PyObject* ikashopSendAuctionOpenAuction(PyObject* poSelf, PyObject* poArgs)
{
	DWORD ownerid = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &ownerid))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopAuctionOpen(ownerid);
	return Py_BuildNone();
}

PyObject* ikashopSendAuctionAddOffer(PyObject* poSelf, PyObject* poArgs)
{
	DWORD ownerid = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &ownerid))
		return Py_BadArgument();

	ikashop::TPriceInfo price;
	if (!PyTuple_GetLongLong(poArgs, 1, &price.yang))
		return Py_BadArgument();

#ifdef ENABLE_CHEQUE_SYSTEM
	if (!PyTuple_GetInteger(poArgs, 2, &price.cheque))
		return Py_BadArgument();
#endif

	CPythonNetworkStream::instance().SendIkarusShopAuctionAddOffer(ownerid, price);
	return Py_BuildNone();
}

PyObject* ikashopSendAuctionExitFrom(PyObject* poSelf, PyObject* poArgs)
{
	DWORD ownerid = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &ownerid))
		return Py_BadArgument();
	CPythonNetworkStream::instance().SendIkarusShopAuctionExitFrom(ownerid);
	return Py_BuildNone();
}

PyObject* ikashopSendAuctionCreate(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos pos;
	if (!PyTuple_GetInteger(poArgs, 0, &pos.window_type))
		return Py_BadArgument();
	
	if (!PyTuple_GetInteger(poArgs, 1, &pos.cell))
		return Py_BadArgument();

	ikashop::TPriceInfo price;
	if (!PyTuple_GetLongLong(poArgs, 2, &price.yang))
		return Py_BadArgument();

	CPythonNetworkStream::instance().SendIkarusShopAuctionCreate(pos, price);
	return Py_BuildNone();
}

PyObject* ikashopSendMyAuctionOpen(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopMyAuctionOpen();
	return Py_BuildNone();
}

PyObject* ikashopSendMyAuctionClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopMyAuctionClose();
	return Py_BuildNone();
}

PyObject* ikashopSendMyAuctionCancel(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusShopMyAuctionCancel();
	return Py_BuildNone();
}

PyObject* SendCloseMyShopBoard(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::instance().SendIkarusMyShopCloseBoard();
	return Py_BuildNone();
}

PyObject* ikashopGetFilterHistory(PyObject* poSelf , PyObject* poArgs)
{
	auto list = CPythonIkarusShop::instance().GetFilterHistory();
	return list.incRef();
}

PyObject* ikashopRegisterFilterHistory(PyObject* poSelf , PyObject* poArgs)
{
	CPythonIkarusShop::TFilterHistoryElement element;
	CPythonIkarusShop::NowDatetime(element.datetime);

	if (!PyTuple_GetUnsignedLong(poArgs, 0, &element.count))
		return Py_BadArgument();

	char* name = nullptr;
	if (!PyTuple_GetString(poArgs, 1, &name))
		return Py_BadArgument();

	strncpy(element.name, name, sizeof(element.name));

	if (!PyTuple_GetInteger(poArgs, 2, &element.type))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 3, &element.subtype))
		return Py_BadArgument();

	PyObject* prices = nullptr;
	if (!PyTuple_GetObject(poArgs, 4, &prices))
		return Py_BadArgument();

	PyObject* levels = nullptr;
	if (!PyTuple_GetObject(poArgs, 5, &levels))
		return Py_BadArgument();

	PyObject* counts = nullptr;
	if (!PyTuple_GetObject(poArgs, 6, &counts))
		return Py_BadArgument();

	PyObject* attrs = nullptr;
	if (!PyTuple_GetObject(poArgs, 7, &attrs))
		return Py_BadArgument();

	PyObject* specials = nullptr;
	if (!PyTuple_GetObject(poArgs, 8, &specials))
		return Py_BadArgument();

	// extracting prices
	if (!PyTuple_GetLongLong(prices, 0, &element.pricestart.yang))
		return Py_BadArgument();

	if (!PyTuple_GetLongLong(prices, 1, &element.pricend.yang))
		return Py_BadArgument();

	// extracting levels
	if (!PyTuple_GetInteger(levels, 0, &element.levelstart))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(levels, 1, &element.levelend))
		return Py_BadArgument();

	// extracting counts
	if (!PyTuple_GetInteger(counts, 0, &element.countstart))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(counts, 1, &element.countend))
		return Py_BadArgument();

	// extracting attributes
	for (int i = 0; i < ITEM_ATTRIBUTE_SLOT_NORM_NUM; i++)
	{
		PyObject* attribute = nullptr;
		if (!PyTuple_GetObject(attrs, i, &attribute))
			return Py_BadArgument();

		int value = 0;
		if (!PyTuple_GetInteger(attribute, 0, &element.attrs[i].bType))
			return Py_BadArgument();

		if (!PyTuple_GetInteger(attribute, 1, &value))
			return Py_BadArgument();

		element.attrs[i].sValue = static_cast<short>(value);
	}

	// extracting specials
	if (!PyTuple_GetInteger(specials, 0, &element.sashGrade))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(specials, 1, &element.alchemyGrade))
		return Py_BadArgument();

	CPythonIkarusShop::instance().AddNewFilterToHistory(element);
	return Py_BuildNone();
}

PyObject* ikashopGetItemNames(PyObject* poSelf, PyObject* poArgs)
{
	auto vec = CPythonIkarusShop::instance().GetItemNames();
	// converting map to dict
	PythonSafeList list;
	for (auto& name : vec)
		list.append(name);
	return list.incRef();
}

PyObject* ikashopSetBusinessBoard(PyObject* poSelf , PyObject* poArgs)
{
	PyObject* board = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &board))
		return Py_BadArgument();
	if (board == Py_None)
		board = nullptr;
	CPythonIkarusShop::instance().SetBusinessBoard(board);
	return Py_BuildNone();
}

PyObject* ikashopSetSearchShopBoard(PyObject* poSelf , PyObject* poArgs)
{
	PyObject* board = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &board))
		return Py_BadArgument();
	
	if (board == Py_None)
		board = nullptr;

	CPythonIkarusShop::instance().SetSearchShopBoard(board);
	return Py_BuildNone();
}

PyObject* ikashopGetBusinessBoard(PyObject* poSelf , PyObject* poArgs)
{
	PyObject* poInterface = CPythonIkarusShop::instance().GetBusinessBoard();
	
	if (!poInterface)
		return Py_BuildNone();

	Py_IncRef(poInterface);
	return poInterface;
}

PyObject* ikashopShopBuilding_AddItem(PyObject* poSelf, PyObject* poArgs)
{
	int win = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &win))
		return Py_BadArgument();

	int slot = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &slot))
		return Py_BadArgument();

	CPythonIkarusShop::instance().ShopBuilding_AddItem(win, slot);
	return Py_BuildNone();
}

PyObject* ikashopAuctionBuilding_AddItem(PyObject* poSelf, PyObject* poArgs)
{
	int win = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &win))
		return Py_BadArgument();

	int slot = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &slot))
		return Py_BadArgument();

	CPythonIkarusShop::instance().AuctionBuilding_AddItem(win, slot);
	return Py_BuildNone();
}

#ifdef ENABLE_IKASHOP_ENTITIES
PyObject* ikashopHideShopNames(PyObject* poSelf, PyObject* poArgs)
{
	CPythonIkarusShop::instance().SetShowNameFlag(false);
	return Py_BuildNone();
}

PyObject* ikashopShowShopNames(PyObject* poSelf, PyObject* poArgs)
{
	CPythonIkarusShop::instance().SetShowNameFlag(true);
	return Py_BuildNone();
}
#endif

PyObject* ikashopGetFirstEmptyCell(PyObject* poSelf, PyObject* poArgs)
{
	int size = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &size))
		return Py_BadArgument();

	PyObject* items = nullptr;
	if (!PyTuple_GetObject(poArgs, 1, &items) || !PyList_Check(items))
		return Py_BadArgument();

	constexpr auto width = ikashop::OFFLINESHOP_GRID_WIDTH;
	constexpr auto height = ikashop::OFFLINESHOP_GRID_HEIGHT;

	ikashop::CShopGrid<width, height> grid{};

	for (int i = 0; i < PyList_Size(items); i++)
	{
		auto info = PyList_GetItem(items, i);
		if (!PyTuple_Check(info) || PyTuple_Size(info) != 2)
			return Py_BadArgument();

		DWORD vnum = 0;
		if (!PyTuple_GetUnsignedLong(info, 0, &vnum))
			return Py_BadArgument();
		int cell = 0;
		if (!PyTuple_GetInteger(info, 1, &cell))
			return Py_BadArgument();

		CItemData* data = nullptr;
		if (!CItemManager::instance().GetItemDataPointer(vnum, &data))
			return Py_BadArgument();

		grid.RegisterItem(cell, data->GetSize());
	}

	const auto cell = grid.FindSpace(size);
	if (!cell)
		return Py_BuildNone();

	return PyInt_FromLong(*cell);
}

PyObject* ikashopCheckShopSpace(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* items = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &items) || !PyList_Check(items))
		return Py_BadArgument();

	DWORD vnum = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 1, &vnum))
		return Py_BadArgument();

	int cell = 0;
	if (!PyTuple_GetInteger(poArgs, 2, &cell))
		return Py_BadArgument();

	CItemData* itemData = nullptr;
	if (!CItemManager::instance().GetItemDataPointer(vnum, &itemData))
		return PyBool_FromLong(false);

	constexpr auto width = ikashop::OFFLINESHOP_GRID_WIDTH;
	constexpr auto height = ikashop::OFFLINESHOP_GRID_HEIGHT;

	ikashop::CShopGrid<width, height> grid{};

	// filling grids with items
	for (int i = 0; i < PyList_Size(items); i++)
	{
		auto info = PyList_GetItem(items, i);
		if (!PyTuple_Check(info) || PyTuple_Size(info) != 2)
			return Py_BadArgument();

		DWORD vnum = 0;
		if (!PyTuple_GetUnsignedLong(info, 0, &vnum))
			return Py_BadArgument();
		int cell = 0;
		if (!PyTuple_GetInteger(info, 1, &cell))
			return Py_BadArgument();

		CItemData* data = nullptr;
		if (!CItemManager::instance().GetItemDataPointer(vnum, &data))
			return Py_BadArgument();

		grid.RegisterItem(cell, data->GetSize());
	}

	return PyBool_FromLong(grid.CheckSpace(cell, itemData->GetSize()));
}

PyObject* ikashopGetItemHash(PyObject* poSelf, PyObject* poArgs)
{
	// extracting data
	DWORD vnum = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &vnum))
		return Py_BadArgument();

	DWORD count = 0;
	if (!PyTuple_GetUnsignedLong(poArgs, 1, &count))
		return Py_BadArgument();

	PyObject* sockets = nullptr;
	if (!PyTuple_GetObject(poArgs, 2, &sockets))
		return Py_BadArgument();

	PyObject* attrs = nullptr;
	if (!PyTuple_GetObject(poArgs, 3, &attrs))
		return Py_BadArgument();

	PyObject* attribute = nullptr;
	int attrvalue = 0;

	ikashop::TShopPlayerItem item{};
	item.vnum = vnum;
	item.count = count;

	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; i++)
		if (!PyTuple_GetLong(sockets, i, &item.alSockets[i]))
			return Py_BadArgument();

	for (int i = 0; i < ITEM_ATTRIBUTE_SLOT_MAX_NUM; i++)
	{
		if (!PyTuple_GetObject(attrs, i, &attribute) || !PyTuple_GetInteger(attribute, 0, &item.aAttr[i].bType) || !PyTuple_GetInteger(attribute, 1, &attrvalue))
			return Py_BadArgument();
		item.aAttr[i].sValue = static_cast<short>(attrvalue);
	}

	return PyInt_FromSize_t(static_cast<size_t>(ikashop::GetItemHash(item)));
}

void initikashop()
{
	static PyMethodDef s_methods[] =
	{
		//packets exchanging
		{ "SendShopCreate", ikashopSendShopCreate, METH_VARARGS },
		{ "SendForceCloseShop", ikashopSendForceCloseShop, METH_VARARGS },
		{ "SendOpenShop", ikashopSendOpenShop, METH_VARARGS },
		{ "SendOpenShopOwner", ikashopSendOpenShopOwner, METH_VARARGS },
		{ "SendBuyItem", ikashopSendBuyItem, METH_VARARGS },
		{ "SendAddItem", ikashopSendAddItem, METH_VARARGS },
		{ "SendRemoveItem", ikashopSendRemoveItem, METH_VARARGS },
		{ "SendEditItem", ikashopSendEditItem, METH_VARARGS },
		{ "SendFilterRequest", ikashopSendFilterRequest, METH_VARARGS },

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		{ "SendFilterNameRequest", ikashopSendFilterNameRequest, METH_VARARGS },
		{ "SendFilterNameClear", ikashopSendFilterNameClear, METH_VARARGS },
		#endif

		#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
		{ "SendAvarageAndMinMaxPricePacket", ikashopSendAvarageAndMinMaxPricePacket, METH_VARARGS },
		#endif

		{ "SendRandomSearchFillRequest", ikashopSendRandomSearchFillRequest, METH_VARARGS },
		{ "SendOfferCreate", ikashopSendOfferCreate, METH_VARARGS },
		{ "SendOfferAccept", ikashopSendOfferAccept, METH_VARARGS },
		{ "SendOfferCancel", ikashopSendOfferCancel, METH_VARARGS },
		{ "SendOfferListRequest", ikashopSendOfferListRequest, METH_VARARGS },
		{ "SendSafeboxOpen", ikashopSendSafeboxOpen, METH_VARARGS },
		{ "SendSafeboxGetItem", ikashopSendSafeboxGetItem, METH_VARARGS },
		{ "SendSafeboxGetValutes", ikashopSendSafeboxGetValutes, METH_VARARGS },
		{ "SendSafeboxClose", ikashopSendSafeboxClose, METH_VARARGS },
		{ "SendOffersListClose", ikashopSendOffersListClose, METH_VARARGS },
		{ "SendCloseMyShopBoard", ikashopSendCloseMyShopBoard, METH_VARARGS },
		{ "SendCloseShopGuestBoard", ikashopSendCloseShopGuestBoard, METH_VARARGS },

		//AUCTION
		{ "SendAuctionListRequest", ikashopSendAuctionListRequest, METH_VARARGS },
		{ "SendAuctionOpenAuction", ikashopSendAuctionOpenAuction, METH_VARARGS },
		{ "SendAuctionAddOffer", ikashopSendAuctionAddOffer, METH_VARARGS },
		{ "SendAuctionExitFrom", ikashopSendAuctionExitFrom, METH_VARARGS },
		{ "SendAuctionCreate", ikashopSendAuctionCreate, METH_VARARGS },
		{ "SendMyAuctionOpen", ikashopSendMyAuctionOpen, METH_VARARGS },
		{ "SendMyAuctionClose", ikashopSendMyAuctionClose, METH_VARARGS },
		{ "SendMyAuctionCancel", ikashopSendMyAuctionCancel, METH_VARARGS },

		//interfaces methods
		{ "GetFilterHistory", ikashopGetFilterHistory, METH_VARARGS },
		{ "RegisterFilterHistory", ikashopRegisterFilterHistory, METH_VARARGS },

		{ "GetItemNames", ikashopGetItemNames, METH_VARARGS },

		{ "SetSearchShopBoard", ikashopSetSearchShopBoard, METH_VARARGS },
		{ "SetBusinessBoard", ikashopSetBusinessBoard, METH_VARARGS },
		{ "GetBusinessBoard", ikashopGetBusinessBoard, METH_VARARGS },

		//create shop methods
		{ "AddItemOnShop", ikashopShopBuilding_AddItem, METH_VARARGS },
		{ "AddItemOnAuction", ikashopAuctionBuilding_AddItem, METH_VARARGS },

		#ifdef ENABLE_IKASHOP_ENTITIES
		{ "HideShopNames", ikashopHideShopNames, METH_VARARGS },
		{ "ShowShopNames", ikashopShowShopNames, METH_VARARGS },
		#endif

		// helpers
		{ "GetFirstEmptyCell", ikashopGetFirstEmptyCell, METH_VARARGS },
		{ "CheckShopSpace", ikashopCheckShopSpace, METH_VARARGS },

		{ "GetItemHash", ikashopGetItemHash, METH_VARARGS },

		{ NULL, NULL, NULL },
	};

	PyObject * poModule = Py_InitModule("ikashop", s_methods);
	PyModule_AddIntConstant(poModule, "FILTER_ATTRIBUTE_NUM", ITEM_ATTRIBUTE_SLOT_NORM_NUM);
	PyModule_AddIntConstant(poModule, "FILTER_NAME_MAX_LEN", CItemData::ITEM_NAME_MAX_LEN);

	PyModule_AddIntConstant(poModule, "SHOP_GRID_WIDTH", ikashop::OFFLINESHOP_GRID_WIDTH);
	PyModule_AddIntConstant(poModule, "SHOP_GRID_HEIGHT", ikashop::OFFLINESHOP_GRID_HEIGHT);
	PyModule_AddIntConstant(poModule, "SAFEBOX_GRID_WIDTH", ikashop::SAFEBOX_GRID_WIDTH);
	PyModule_AddIntConstant(poModule, "SAFEBOX_GRID_HEIGHT", ikashop::SAFEBOX_GRID_HEIGHT);
	PyModule_AddIntConstant(poModule, "SAFEBOX_PAGE_COUNT", ikashop::SAFEBOX_PAGE_COUNT);
	#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "YANG_PER_CHEQUE", YANG_PER_CHEQUE);
	#endif
}
#endif


