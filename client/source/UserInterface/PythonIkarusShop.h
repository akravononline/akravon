#ifndef __INCLUDE_HEADER_PYTHON_OFFLINESHO__
#define __INCLUDE_HEADER_PYTHON_OFFLINESHO__

#ifdef ENABLE_IKASHOP_RENEWAL
#include "PythonBackground.h"
#include "Packet.h"
#include <msl/msl.h>

namespace ikashop
{
	template <class T ,template <class> class S, typename S<T>::iterator >
	void ForEach(S<T>& container, std::function<void(T&)> func)
	{
		std::for_each(container.begin(), container.end(), func);
	}

	template <class T , class K, template <class,class> class S, typename S<K,T>::iterator >
	void ForEach(S<K,T>& container, std::function<void(T&)> func)
	{
		std::for_each(container.begin(), container.end(), [&](auto&& iter){ func(iter.second); });
	}

	template <class T>
	void DeletePointersContainer(T& obj)
	{
		ForEach(obj, [](auto&& iter){ delete(iter); });
	}

	enum eConstOfflineshop
	{
		OFFLINESHOP_DURATION_MAX_DAYS	= 99,
		OFFLINESHOP_DURATION_MAX_HOURS	= 23,
		OFFLINESHOP_DURATION_MAX_MINUTES = OFFLINESHOP_DURATION_MAX_DAYS * 24 * 60,
		OFFLINESHOP_MAX_FILTER_HISTORY_SIZE = 25,

		OFFLINESHOP_GRID_WIDTH = 10,//15
		OFFLINESHOP_GRID_HEIGHT = 10,
		OFFLINESHOP_GRID_CELL_COUNT = OFFLINESHOP_GRID_WIDTH * OFFLINESHOP_GRID_HEIGHT,

		SAFEBOX_GRID_WIDTH = 9,
		SAFEBOX_GRID_HEIGHT = 11,
		SAFEBOX_PAGE_COUNT = 3,
	};

	template<int WIDTH, int HEIGHT>
	class CShopGrid
	{
		static constexpr auto CELL_COUNT = WIDTH * HEIGHT;

	public:
		void Clear()
		{
			m_cells = {};
		}

		void RegisterItem(int cell, int size)
		{
			const auto [col, row] = _DecomposeCell(cell);
			for (int i = 0; i < size; i++)
				_SetCellState(_ComposeCell(col, row + i), true);
		}

		void UnregisterItem(int cell, int size)
		{
			const auto [col, row] = _DecomposeCell(cell);
			for (int i = 0; i < size; i++)
				_SetCellState(_ComposeCell(col, row + i), false);
		}

		bool CheckSpace(int cell, int size)
		{
			const auto [col, row] = _DecomposeCell(cell);
			auto range = msl::range(row, row + size);
			return std::all_of(range.begin(), range.end(), [&](auto&& value) { return !_GetCellState(_ComposeCell(col, value)); });
		}

		std::optional<int> FindSpace(int size)
		{
			for (int cell = 0; cell < CELL_COUNT; cell++)
				if (CheckSpace(cell, size))
					return cell;
			return std::nullopt;
		}

		constexpr int GetCellCount()
		{
			return WIDTH * HEIGHT;
		}

	private:
		std::pair<int, int> _DecomposeCell(int cell) { return { cell % WIDTH, cell / WIDTH }; }
		int _ComposeCell(int col, int row) { return row * WIDTH + col; }
		void _SetCellState(int cell, bool state) { if (cell >= 0 && cell < CELL_COUNT) m_cells[cell] = state; }
		bool _GetCellState(int cell) { return cell >= 0 && cell < CELL_COUNT ? m_cells[cell] : true; }

	private:
		std::array<bool, WIDTH*HEIGHT> m_cells{};
	};


	DWORD GetItemHash(const TShopPlayerItem& item);

	namespace python 
	{
		struct PythonReference
		{
		public:
			static PythonReference steal(PyObject* obj){
				PythonReference ref;
				ref.m_obj = obj;
				return ref;
			}

		private:
			PythonReference() = default;
		
		public:
			PythonReference(PyObject* obj) : m_obj{ obj } { Py_XINCREF(obj); }
			PythonReference(const PythonReference& r) : m_obj{ r.m_obj } { Py_XINCREF(r.m_obj); }
			PythonReference(PythonReference&& r) : m_obj{ r.m_obj } { r.m_obj = nullptr; }
			~PythonReference() { Py_XDECREF(m_obj); }

			operator PyObject* () { return m_obj; }
			PyObject* weakRef() const { return m_obj; }

			operator bool() const { return m_obj != nullptr && m_obj != Py_None && PyObject_IsTrue(m_obj); }

		private:
			PyObject* m_obj = nullptr;
		};

		struct PythonGarbageCollector
		{
			// integer values
			PyObject* makeInteger(int val) { return _Register(PyInt_FromLong(static_cast<LONG>(val))); }
#ifdef CYNEX_MOBILE
			// Cynex: android'de LONG = int, int64_t = long - long long ayrica lazim
			PyObject* makeInteger(long long val) { return _Register(PyLong_FromLongLong(val)); }
			PyObject* makeInteger(unsigned long long val) { return _Register(PyLong_FromUnsignedLongLong(val)); }
#else
			PyObject* makeInteger(LONG val) { return _Register(PyInt_FromLong(val)); }
#endif
			PyObject* makeInteger(DWORD val) { return _Register(PyInt_FromSize_t(val)); }
			PyObject* makeInteger(int64_t val) { return _Register(PyLong_FromLongLong(val)); }
			PyObject* makeInteger(uint64_t val) { return _Register(PyLong_FromUnsignedLongLong(val)); }
			// floating values
			PyObject* makeFloat(float value) { return _Register(PyFloat_FromDouble(static_cast<double>(value))); }
			PyObject* makeFloat(double value) { return _Register(PyFloat_FromDouble(value)); }
			// boolean
			PyObject* makeBool(bool value) { return _Register(PyBool_FromLong(value ? 1 : 0)); }
			// string
			PyObject* makeString(const std::string& str) { return _Register(PyString_FromString(str.c_str())); }
			PyObject* makeString(const char* str) { return _Register(PyString_FromString(str)); }
			// objects
			PyObject* makeDict() { return _Register(PyDict_New()); }
			PyObject* makeList(size_t count = 0) { return _Register(PyList_New(count)); }
			PyObject* makeTuple(size_t count = 0) { return _Register(PyTuple_New(count)); }
			
			template<class ...ArgTypes>
			PyObject* makeValue(const std::string& str, const ArgTypes& ...arg) {
				return _Register(Py_BuildValue(str.c_str(), arg...));
			}

			PyObject* registerObject(PyObject* obj){ objects.emplace_back(obj); return obj; }

			~PythonGarbageCollector(){ std::for_each(objects.begin(), objects.end(), [](auto&& obj){ Py_XDECREF(obj); }); }

		private:
			PyObject* _Register(PyObject* ptr) { objects.emplace_back(ptr); return ptr; }

		private:
			std::vector<PyObject*> objects;
		};

		struct PythonSafeDict;
		struct PythonSafeList;

		template<class Type>
		constexpr bool IsPythonSafeType = std::is_same_v<Type, PythonSafeDict> || std::is_same_v<Type, PythonSafeList> || std::is_same_v<Type, PythonReference>;

		struct PythonSafeDict
		{
			PythonSafeDict() { dict = collector.makeDict(); }
			PythonSafeDict(const PythonSafeDict& r) { dict = collector.registerObject(r.incRef()); }

			// integers
			void set(const std::string& key, bool value) { PyDict_SetItemString(dict, key.c_str(), collector.makeBool(value)); }
			void set(const std::string& key, int value) { PyDict_SetItemString(dict, key.c_str(), collector.makeInteger(value)); }
#ifdef CYNEX_MOBILE
			void set(const std::string& key, long long value) { PyDict_SetItemString(dict, key.c_str(), collector.makeInteger(value)); }
			void set(const std::string& key, unsigned long long value) { PyDict_SetItemString(dict, key.c_str(), collector.makeInteger(value)); }
#else
			void set(const std::string& key, LONG value) { PyDict_SetItemString(dict, key.c_str(), collector.makeInteger(value)); }
#endif
			void set(const std::string& key, DWORD value) { PyDict_SetItemString(dict, key.c_str(), collector.makeInteger(value)); }
			void set(const std::string& key, int64_t value) { PyDict_SetItemString(dict, key.c_str(), collector.makeInteger(value)); }
			void set(const std::string& key, uint64_t value) { PyDict_SetItemString(dict, key.c_str(), collector.makeInteger(value)); }
			// floats
			void set(const std::string& key, float value) { PyDict_SetItemString(dict, key.c_str(), collector.makeFloat(value)); }
			void set(const std::string& key, double value) { PyDict_SetItemString(dict, key.c_str(), collector.makeFloat(value)); }
			// strings
			void set(const std::string& key, const std::string& value) { PyDict_SetItemString(dict, key.c_str(), collector.makeString(value)); }
			void set(const std::string& key, const char* value) { PyDict_SetItemString(dict, key.c_str(), collector.makeString(value)); }
			// objects
			void set(const std::string& key, PyObject* obj) { PyDict_SetItemString(dict, key.c_str(), obj); }

			template<class Type, std::enable_if_t< IsPythonSafeType<Type>, int> = 0>
			void set(const std::string& key, const Type& value) { PyDict_SetItemString(dict, key.c_str(), value.weakRef()); }

			// references
			PyObject* weakRef() const { return dict; }
			PythonReference ref() const { return { dict }; }
			PyObject* incRef() const { Py_XINCREF(dict); return dict; }

		private:
			PyObject* dict = nullptr;
			PythonGarbageCollector collector;
		};

		struct PythonSafeList
		{
			PythonSafeList() { list = collector.makeList(); }
			PythonSafeList(const PythonSafeList& r) { list = collector.registerObject(r.incRef()); }

			// integers
			void append(bool value) { PyList_Append(list, collector.makeBool(value)); }
			void append(int value) { PyList_Append(list, collector.makeInteger(value)); }
#ifdef CYNEX_MOBILE
			void append(long long value) { PyList_Append(list, collector.makeInteger(value)); }
			void append(unsigned long long value) { PyList_Append(list, collector.makeInteger(value)); }
#else
			void append(LONG value) { PyList_Append(list, collector.makeInteger(value)); }
#endif
			void append(DWORD value) { PyList_Append(list, collector.makeInteger(value)); }
			void append(int64_t value) { PyList_Append(list, collector.makeInteger(value)); }
			void append(uint64_t value) { PyList_Append(list, collector.makeInteger(value)); }
			// floats
			void append(float value) { PyList_Append(list, collector.makeFloat(value)); }
			void append(double value) { PyList_Append(list, collector.makeFloat(value)); }
			// strings
			void append(const std::string& value) { PyList_Append(list, collector.makeString(value)); }
			void append(const char* value) { PyList_Append(list, collector.makeString(value)); }
			// objects
			void append(PyObject* obj) { PyList_Append(list, obj); }

			template<class Type, std::enable_if_t<IsPythonSafeType<Type>, int> = 0>
			void append(const Type& value) { PyList_Append(list, value.weakRef()); }

			// references
			PyObject* weakRef() const { return list; }
			PythonReference ref() const { return { list }; }
			PyObject* incRef() const { Py_XINCREF(list); return list; }

		private:
			PyObject* list = nullptr;
			PythonGarbageCollector collector;
		};

		template<class Type>
		inline PythonSafeDict ExtractPythonDictPlayerItem(const Type& info) {
			// appending item attributes
			PythonGarbageCollector collector;

			PythonSafeList attrList;
			for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; j++)
				attrList.append(collector.makeValue("(ii)", info.aAttr[j].bType, info.aAttr[j].sValue));

			// appending item sockets
			PythonSafeList socketList;
			for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; j++)
				socketList.append(info.alSockets[j]);

			// making item dict
			PythonSafeDict itemDict;
			itemDict.set("id", info.id);
			itemDict.set("owner", info.owner);
			itemDict.set("vnum", info.vnum);
			itemDict.set("count", info.count);
			#ifdef ENABLE_EVOLUTION_SYSTEM
			itemDict.set("evolution", info.evolution);
			#endif
			#ifdef ENABLE_CHANGELOOK_SYSTEM
			itemDict.set("transmutation", info.transmutation);
			#endif
			#ifdef ENABLE_LOCKED_SYSTEM
			itemDict.set("locked", info.locked);
			#endif
			itemDict.set("attrs", attrList.weakRef());
			itemDict.set("sockets", socketList.weakRef());
			itemDict.set("window", info.window);
			itemDict.set("cell", info.pos);
			return itemDict;
		}

		template<class Type>
		inline PythonSafeDict ExtractPythonDictShopItem(const Type& info) {
			auto itemDict = ExtractPythonDictPlayerItem(info);
			itemDict.set("price", info.price.yang);
			#ifdef ENABLE_CHEQUE_SYSTEM
			itemDict.set("cheque", info.price.cheque);
			#endif
			return itemDict;
		}


		template<class Type, class FuncionType>
		inline PythonSafeList ExtractPythonListFromContainer(const Type& container, FuncionType func) {
			PythonSafeList list;
			for(auto& element : container)
				list.append(func(element));
			return list;
		}

		template<class Type>
		inline PythonSafeList ExtractPythonPlayerItemList(const Type& container) {
			static auto func = [](auto&& value) { return ExtractPythonDictPlayerItem(value); };
			return ExtractPythonListFromContainer(container, func);
		}

		template<class Type>
		inline PythonSafeList ExtractPythonShopItemList(const Type& container) {
			static auto func = [](auto&& value) { return ExtractPythonDictShopItem(value); };
			return ExtractPythonListFromContainer(container, func);
		}
	}
}

#ifdef ENABLE_IKASHOP_ENTITIES
namespace ikashop
{
	#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	struct PIXEL_POSITION { float x, y, z; };
	#endif

	class ShopInstance {
		using SkinType = int;
		using ShopID = uint32_t;
		using SignString = std::string;

	public:
		ShopInstance(SkinType Skin);

		void SetVID(ShopID dwVID) { m_VirtualID = dwVID; }
		ShopID GetVID() const { return m_VirtualID; }
		void SetShopType(SkinType iType) { m_Skin = iType; }
		SkinType GetType() const { return m_Skin; }
		bool IsSkin() const { return m_Skin != 0; }
		CGraphicThingInstance* GetThingInstancePtr() { return &m_GraphicThing; }

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		const PIXEL_POSITION & GetXYZ() const { return m_pos; }
		#endif

		void SetName(const SignString& str);
		const SignString& GetName();
		void Show(float x, float y, float z);
		void Clear();
		void Render();
		void RenderText();
		void BlendRender();
		void Update();
		void UpdateTextTail();

		bool IsPickable();
		bool IsPicked();

	private:
		CGraphicThingInstance m_GraphicThing{};
		CGraphicTextInstance m_Text;
		float m_TextHeight;
		ShopID m_VirtualID = 0;
		SkinType m_Skin = 0;
		bool m_shownEffects = true;
		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		PIXEL_POSITION m_pos = {0, 0, 0};
		#endif
	};
}
#endif


class CPythonIkarusShop : public CSingleton<CPythonIkarusShop>
{

public:
	struct TFilterHistoryElement : public ikashop::TFilterInfo
	{
		int64_t datetime;
		DWORD count;

		friend auto operator < (const TFilterHistoryElement& left, const TFilterHistoryElement& right) {
			return left.datetime < right.datetime;
		}

		friend auto operator > (const TFilterHistoryElement& left, const TFilterHistoryElement& right) {
			return left.datetime > right.datetime;
		}

		friend auto operator == (const TFilterHistoryElement& left, const TFilterHistoryElement& right) {
			return left.datetime == right.datetime;
		}
	};


	typedef std::vector<TFilterHistoryElement> FILTERHISTORY;
	typedef FILTERHISTORY::iterator FILTERHISTORY_ITER;

	static void NowDatetime(int64_t& datetime)
	{
		datetime = static_cast<int64_t>(std::time(nullptr));
	}

	static constexpr auto PRICE_HISTORY_EXPIRATION = 7 * 24 * 60 * 60;
	static constexpr auto NOTIFICATION_EXPIRATION = 10 * 24 * 60 * 60;


public:
	CPythonIkarusShop();
	~CPythonIkarusShop();

	//starts
	void SetSearchShopBoard(PyObject* poWindow);
	void SetBusinessBoard(PyObject* poWindow);
	PyObject* GetBusinessBoard();


	void OpenShop(const ikashop::TShopInfo& shop, const std::vector<ikashop::TShopItem>& vec);
	void OpenShopOwner(const ikashop::TShopInfo& shop, const std::vector<ikashop::TShopItem>& vec);
	void OpenShopOwnerNoShop();

	void ShopClose();
	void ShopExpiredGuesting(DWORD ownerid);
	void ShopRemoveItem(bool owner, DWORD itemid);

	void ShopGuestEditItem(DWORD itemid, const ikashop::TPriceInfo& price);
	void ShopOwnerEditItem(DWORD itemid, const ikashop::TPriceInfo& price);

	void ShopFilterResult(const std::vector<ikashop::TSearchResultInfo>& vec);

	#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	void ShopFilterNameResult(const std::vector<ikashop::TSearchNameResultInfo>& vec);
	void ShopFilterNameClear();
	#endif

	#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
	void UpdateAvarageAndMinMaxPrice(const long long avgPrice, const long long minPrice, const long long maxPrice);
	#endif

	void OfferListReceived(const std::vector<ikashop::TOfferListElement>& offers);
	void SafeboxRefresh(const ikashop::TValutesInfo& valute, const std::vector<ikashop::TShopPlayerItem>& item);
	void SafeboxRemoveItem(DWORD itemid);
	void SafeboxAddItem(const ikashop::TShopPlayerItem& item);

	void AuctionList(const std::vector<ikashop::TAuctionListElement>& auctions);
	void MyAuctionOpen(const ikashop::TAuctionInfo& auction, const std::vector<ikashop::TAuctionOfferInfo>& offers);
	void AuctionOpen(const ikashop::TAuctionInfo& auction, const std::vector<ikashop::TAuctionOfferInfo>& offers);
	void OpenMyAuctionNoAuction();

	//filter caching
	void LoadFilterHistory();
	void SaveFilterHistory();
	void AddNewFilterToHistory(TFilterHistoryElement& element);
	void PopOldestFilterHistoryElement();


	//interfaces methods
	ikashop::python::PythonSafeList GetFilterHistory();
	std::vector<std::string> GetItemNames();

	//create shop methods
	void ShopBuilding_AddItem(int win, int slot);
	void AuctionBuilding_AddItem(int win, int slot);

	void RefreshBoardCounters(int safebox, int auction, int offers);

	#ifdef ENABLE_IKASHOP_ENTITIES
	void InsertEntity(DWORD vid , int type, const char* name, LONG x, LONG y, LONG z);
	void RemoveEntity(DWORD vid);

	void RenderEntities();
	void UpdateEntities();
	void UpdateShowingTextTail();

	bool GetShowNameFlag();
	void SetShowNameFlag(bool flag);

	void DeleteEntities();
	bool CheckPickedShopEntity(DWORD* ret);
	#endif

	void PopupMessage(const std::string& message);
	void DeleteSearchResult(DWORD itemid);

	#ifdef ENABLE_IKASHOP_ENTITIES
	std::vector<std::unique_ptr<ikashop::ShopInstance>> m_shopInstances;
	#endif

	#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	std::set<DWORD> m_shopSearchList;
	#endif

private:
	bool CallOnBusinessBoard(const std::string& func, PyObject* args = nullptr);
	bool CallOnSearchShopBoard(const std::string& func, PyObject* args = nullptr);

private:
	PyObject* m_businessBoard = nullptr;
	PyObject* m_searchShopBoard = nullptr;

	FILTERHISTORY m_filterHistory;

	// price history

	#ifdef ENABLE_IKASHOP_ENTITIES
	// std::vector<std::unique_ptr<ikashop::ShopInstance>> m_shopInstances;
	bool m_bIsShowName;
	#endif
};

extern void initikashop();
#endif

#endif //__INCLUDE_HEADER_PYTHON_OFFLINESHO__
