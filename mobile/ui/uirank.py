# -*- coding: utf-8 -*-
# AKRAVON: siralama penceresi - bizim tasarim dili (lacivert + altin tahta, yatay depo dugmeleri, Marcellus baslik)
# Eski masaustu penceresi (3B model, kaydirma cubugu, 15 satir) kalkti: ilk 10 oyuncu, kategori dugmeleri, tek ekran.
# Arayuz ayni kalir: RankingWindow() / Open / Close / IsShow / Hide / Destroy / LoadRankData / UpdateLastItem
import net
import player
import ui
import uiAkravon
import uiMobileStyle
import renderTarget
import wndMgr

IMG_DIR_EMPIRE = "d:/ymir work/ui/game/rank/"
CLOSE = ("d:/ymir work/ui/akravon_legacy/inv_close_1.png", "d:/ymir work/ui/akravon_legacy/inv_close_1.png", "d:/ymir work/ui/akravon_legacy/inv_close_3.png")

RENDER_TARGET_INDEX = 11
MODEL_WIDTH = 165
MODEL_HEIGHT = 340
TOP = 16
TITLE_Y = 14
CONTENT_TOP = 62
SIDE_X = 20
SIDE_WIDTH = 210
SIDE_STEP = 43
SIDE_HEIGHT = 36
TAB_HEIGHT = 42
LIST_X = SIDE_X + SIDE_WIDTH + 20
LIST_WIDTH = 510
MODEL_X = LIST_X + LIST_WIDTH + 14
PANEL_WIDTH = MODEL_X + MODEL_WIDTH + 20
RACE_HEIGHT = 36
HEAD_Y = CONTENT_TOP + RACE_HEIGHT + 8
ROW_TOP = HEAD_Y + 30
ROW_HEIGHT = 36
ROWS = 10
BOTTOM = 20

FONT_TITLE = "Marcellus:26"
FONT_HEAD = "Marcellus:18"
FONT_ROW = "Lato:20"
FONT_TAB = "Marcellus:18"
FONT_SIDE = "Marcellus:16"
FONT_RACE = "Marcellus:16"

GOLD = uiMobileStyle.Color(236, 206, 140)
SILVER = uiMobileStyle.Color(214, 220, 232)
BRONZE = uiMobileStyle.Color(214, 150, 96)
NORMAL = uiMobileStyle.Color(238, 232, 218)
MINE = uiMobileStyle.Color(255, 214, 120)
ROW_A = uiMobileStyle.Color(18, 28, 52, 150)
ROW_B = uiMobileStyle.Color(30, 44, 76, 150)
ROW_ME = uiMobileStyle.Color(96, 74, 30, 200)

TYPE_NAMES = {0 : "Genel", 1 : "Zindan", 2 : "Patron"}
RACE_NAMES = ("Tümü", "Savaşçı", "Ninja", "Sura", "Şaman")

CATEGORY_NAMES = {
	0 : {},
	1 : {},
	2 : {},
}


def _Names():
	# oyuncu / zindan / patron kategorileri (anahtar = player.RANK_*), yazilar burada
	names = {}
	names[0] = (
		(player.RANK_LEVEL, "Karakter Level"),
		(player.RANK_K_STONE, "Kesilen Metin"),
		(player.RANK_K_BOSS, "Kesilen Patron"),
		(player.RANK_D_STONE, "Metin Hasar"),
		(player.RANK_D_BOSS, "Patron Hasar"),
		(player.RANK_D_SKILL, "Pvp Beceri Hasar"),
		(player.RANK_D_NORMAL, "Pvp Düz Hasar"),
		(player.RANK_DUNGEON, "Tamamlanmış Zindan"),
		(player.RANK_PLAYTIME, "Oyun Süresi"),
		(player.RANK_CHEST, "Açılan Sandık"),
	)
	names[1] = (
		(player.RANK_DUNGEON_LUCIFER, "Lucifer"),
		(player.RANK_DUNGEON_CATACOMB, "Azrail"),
		(player.RANK_DUNGEON_BERAN_SETAOU, "Mavi Ejderha"),
		(player.RANK_DUNGEON_BARONES, "Barones"),
		(player.RANK_DUNGEON_RAZADOR, "Razador"),
		(player.RANK_DUNGEON_NEMERE, "Nemere"),
	)
	names[2] = (
		(player.RANK_BOSS_ORK, "Ork Başkanı"),
		(player.RANK_BOSS_QUEEN_SPIDER, "Kralice Örümcek"),
		(player.RANK_BOSS_DARK_GHOST_LEADER, "Karanlık Hayalet Lideri"),
		(player.RANK_BOSS_GIANT_DESERT_TORTOISE, "Dev Çöl Kaplumbağası"),
		(player.RANK_BOSS_NINE_TAILS, "Dokuz Kuyruk"),
		(player.RANK_BOSS_FLAME_KING, "Alev Kral"),
		(player.RANK_BOSS_YELLOW_TIGER_GHOST, "Sarı Kaplan Hayaleti"),
		(player.RANK_BOSS_ICE_WITCH, "Güçlü Buz Cadısı"),
		(player.RANK_BOSS_GENERAL, "General"),
		(player.RANK_BOSS_KING_VOBA, "Kral Voba"),
	)
	return names


# her kategori icin sag taraftaki 3B model: (model no, olcek); 0 = karakterin kendisi / secili irk
MODELS = {
	"RANK_K_STONE" : (8001, 1.0), "RANK_K_BOSS" : (1093, 0.8), "RANK_D_STONE" : (8027, 1.0), "RANK_D_BOSS" : (2598, 1.0),
	"RANK_DUNGEON" : (20394, 1.0), "RANK_CHEST" : (3965, 0.5),
	"RANK_DUNGEON_LUCIFER" : (1093, 0.8), "RANK_DUNGEON_CATACOMB" : (2598, 1.0), "RANK_DUNGEON_BERAN_SETAOU" : (2493, 0.6),
	"RANK_DUNGEON_BARONES" : (2092, 0.4), "RANK_DUNGEON_RAZADOR" : (6091, 1.0), "RANK_DUNGEON_NEMERE" : (6191, 1.0),
	"RANK_BOSS_ORK" : (691, 0.9), "RANK_BOSS_QUEEN_SPIDER" : (2091, 0.5), "RANK_BOSS_DARK_GHOST_LEADER" : (792, 1.0),
	"RANK_BOSS_GIANT_DESERT_TORTOISE" : (2191, 0.7), "RANK_BOSS_NINE_TAILS" : (1901, 1.0), "RANK_BOSS_FLAME_KING" : (2206, 1.0),
	"RANK_BOSS_YELLOW_TIGER_GHOST" : (1304, 1.0), "RANK_BOSS_ICE_WITCH" : (1192, 1.0), "RANK_BOSS_GENERAL" : (2492, 1.0),
	"RANK_BOSS_KING_VOBA" : (3791, 0.7),
}


def ModelOf(key):
	for name in MODELS:
		if getattr(player, name, None) == key:
			return MODELS[name]
	return (0, 1.0)


def NumberText(value):
	try:
		number = int(value)
	except Exception:
		return str(value)
	if number <= 0:
		return "-"
	text = str(number)
	parts = []
	while len(text) > 3:
		parts.insert(0, text[-3:])
		text = text[:-3]
	parts.insert(0, text)
	return ".".join(parts)


class RankRow(ui.Window):
	def __init__(self, parent, y, index):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.AddFlag("not_pick")
		self.SetSize(LIST_WIDTH, ROW_HEIGHT)
		self.SetPosition(LIST_X, y)
		self.bar = uiMobileStyle.CreateBar(self, ROW_A if index % 2 == 0 else ROW_B, 0, 0, LIST_WIDTH, ROW_HEIGHT)
		self.rank = uiMobileStyle.CreateText(self, "", 38, 5, NORMAL, FONT_ROW, "center")
		self.name = uiMobileStyle.CreateText(self, "", 84, 5, NORMAL, FONT_ROW)
		self.empire = ui.ImageBox()
		self.empire.SetParent(self)
		self.empire.AddFlag("not_pick")
		self.empire.SetPosition(300, 5)
		self.empire.Hide()
		self.value = uiMobileStyle.CreateText(self, "", LIST_WIDTH - 18, 5, NORMAL, FONT_ROW, "right")
		self.empireId = -1
		self.Hide()

	def Destroy(self):
		self.Hide()
		self.bar = None
		self.rank = None
		self.name = None
		self.empire = None
		self.value = None

	def Set(self, rank, name, empire, value, color, background = None):
		if background is not None:
			self.bar.SetColor(background)
		self.rank.SetText(str(rank))
		self.name.SetText(name)
		self.value.SetText(NumberText(value))
		for line in (self.rank, self.name, self.value):
			line.SetPackedFontColor(color)
		if empire != self.empireId:
			self.empireId = empire
			self.empire.LoadImage(IMG_DIR_EMPIRE + "empire_%d.sub" % empire)
		self.empire.Show()
		self.Show()


class RankingWindow(ui.Window):
	def __init__(self):
		import uiMobileOptions
		ui.Window.__init__(self)
		self.AddFlag("float")
		self.typeIndex = 0
		self.rankIndex = -1
		self.raceIndex = 0
		self.names = _Names()
		self.sideButtons = []
		self.tabButtons = []
		self.raceButtons = []
		self.rows = []
		self.keep = []

		height = ROW_TOP + ROWS * (ROW_HEIGHT + 2) + 12 + ROW_HEIGHT + BOTTOM
		self.SetSize(PANEL_WIDTH, height)
		self.panel = uiAkravon.Frame(True, True)
		self.panel.SetParent(self)
		self.panel.SetSize(PANEL_WIDTH, height)
		self.panel.Show()
		uiAkravon.SkinFrame(self.panel, "panel_thin")
		self.title = uiMobileStyle.CreateText(self.panel, "Sıralama", PANEL_WIDTH // 2, TITLE_Y, uiAkravon.HEADER_COLOR, FONT_TITLE, "center")
		self.closeButton = ui.Button()
		self.closeButton.SetParent(self.panel)
		self.closeButton.SetUpVisual(CLOSE[0])
		self.closeButton.SetOverVisual(CLOSE[1])
		self.closeButton.SetDownVisual(CLOSE[2])
		self.closeButton.SetPosition(PANEL_WIDTH - 52, 12)
		self.closeButton.SAFE_SetEvent(self.Close)
		self.closeButton.Show()

		# ust sekmeler: Genel / Zindanlar / Patronlar (solda, kategori listesinin ustunde)
		tabWidth = (SIDE_WIDTH - 8) // 3
		for index in range(3):
			tab = uiMobileOptions.SkinButton(tabWidth, TAB_HEIGHT, TYPE_NAMES[index], "Marcellus:16")
			tab.SetParent(self.panel)
			tab.SetPosition(SIDE_X + index * (tabWidth + 4), CONTENT_TOP)
			tab.SetEvent(ui.__mem_func__(self.SelectType), index)
			tab.Show()
			self.tabButtons.append(tab)

		# sag ust: irk filtresi
		raceWidth = (LIST_WIDTH - 4 * 6) // 5
		for index in range(5):
			button = uiMobileOptions.SkinButton(raceWidth, RACE_HEIGHT, RACE_NAMES[index], FONT_RACE)
			button.SetParent(self.panel)
			button.SetPosition(LIST_X + index * (raceWidth + 6), CONTENT_TOP)
			button.SetEvent(ui.__mem_func__(self.SelectRace), index)
			button.Show()
			self.raceButtons.append(button)

		# tablo basligi
		self.headBar = uiMobileStyle.CreateBar(self.panel, uiMobileStyle.Color(96, 74, 42, 255), LIST_X, HEAD_Y + 24, LIST_WIDTH, 1)
		self.keep.append(uiMobileStyle.CreateText(self.panel, "#", LIST_X + 38, HEAD_Y, uiAkravon.HEADER_COLOR, FONT_HEAD, "center"))
		self.keep.append(uiMobileStyle.CreateText(self.panel, "Oyuncu", LIST_X + 84, HEAD_Y, uiAkravon.HEADER_COLOR, FONT_HEAD))
		self.keep.append(uiMobileStyle.CreateText(self.panel, "Krallık", LIST_X + 300, HEAD_Y, uiAkravon.HEADER_COLOR, FONT_HEAD))
		self.valueHead = uiMobileStyle.CreateText(self.panel, "Değer", LIST_X + LIST_WIDTH - 18, HEAD_Y, uiAkravon.HEADER_COLOR, FONT_HEAD, "right")
		for index in range(ROWS):
			self.rows.append(RankRow(self.panel, ROW_TOP + index * (ROW_HEIGHT + 2), index))
		self.meRow = RankRow(self.panel, ROW_TOP + ROWS * (ROW_HEIGHT + 2) + 12, 0)
		self.empty = uiMobileStyle.CreateText(self.panel, "Henüz kayıt yok", LIST_X + LIST_WIDTH // 2, ROW_TOP + 120, uiAkravon.HINT_COLOR, "Marcellus:20", "center")
		self.empty.Hide()
		self.modelBoard = ui.ImageBox()
		self.modelBoard.SetParent(self.panel)
		self.modelBoard.AddFlag("not_pick")
		self.modelBoard.LoadImage("rank_window/render_boardbase.png")
		self.modelBoard.SetPosition(MODEL_X, CONTENT_TOP)
		self.modelBoard.Show()
		self.modelTarget = ui.RenderTarget()
		self.modelTarget.SetParent(self.modelBoard)
		self.modelTarget.SetPosition(5, 5)
		self.modelTarget.SetSize(MODEL_WIDTH - 10, MODEL_HEIGHT - 10)
		self.modelTarget.SetRenderTarget(RENDER_TARGET_INDEX)
		self.modelTarget.Show()
		cardTop = CONTENT_TOP + MODEL_HEIGHT + 12
		cardHeight = ROW_TOP + ROWS * (ROW_HEIGHT + 2) + 12 + ROW_HEIGHT - cardTop
		self.card = uiMobileStyle.CreateBar(self.panel, ROW_B, MODEL_X, cardTop, MODEL_WIDTH, cardHeight)
		self.keep.append(self.card)
		self.keep.append(uiMobileStyle.CreateBar(self.panel, uiMobileStyle.Color(176, 140, 74), MODEL_X, cardTop, MODEL_WIDTH, 2))
		self.keep.append(uiMobileStyle.CreateText(self.panel, "SIRAN", MODEL_X + MODEL_WIDTH // 2, cardTop + 12, uiAkravon.HEADER_COLOR, FONT_HEAD, "center"))
		self.cardValue = uiMobileStyle.CreateText(self.panel, "-", MODEL_X + MODEL_WIDTH // 2, cardTop + 40, MINE, "Marcellus:44", "center")
		self.cardHint = uiMobileStyle.CreateText(self.panel, "", MODEL_X + MODEL_WIDTH // 2, cardTop + cardHeight - 34, SILVER, "Lato:17", "center")
		self.sideTop = CONTENT_TOP + TAB_HEIGHT + 12
		self.loaded = False

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		for group in (self.sideButtons, self.tabButtons, self.raceButtons):
			for button in group:
				button.Destroy()
		for row in self.rows + [self.meRow]:
			row.Destroy()
		self.sideButtons = []
		self.tabButtons = []
		self.raceButtons = []
		self.rows = []
		self.keep = []
		self.__HideModel()
		self.Hide()

	# ------------------------------------------------------------ kategoriler
	def __BuildSide(self):
		for button in self.sideButtons:
			button.Destroy()
		self.sideButtons = []
		import uiMobileOptions
		for (position, (key, name)) in enumerate(self.names[self.typeIndex]):
			button = uiMobileOptions.SkinButton(SIDE_WIDTH, SIDE_HEIGHT, name, FONT_SIDE)
			button.SetParent(self.panel)
			button.SetPosition(SIDE_X, self.sideTop + position * SIDE_STEP)
			button.SetEvent(ui.__mem_func__(self.SelectRank), key)
			button.rankKey = key
			button.Show()
			self.sideButtons.append(button)

	def SelectType(self, index):
		self.typeIndex = index
		for (position, tab) in enumerate(self.tabButtons):
			tab.SetSelected(position == index)
		self.__BuildSide()
		self.SelectRank(self.names[index][0][0])

	def SelectRace(self, index):
		self.raceIndex = index
		self.ChangeCategory(self.typeIndex, self.rankIndex)

	def SelectRank(self, key):
		self.ChangeCategory(self.typeIndex, key)

	def ChangeCategory(self, categoryId, subCategoryId):
		self.typeIndex = categoryId
		self.rankIndex = subCategoryId
		for button in self.sideButtons:
			button.SetSelected(button.rankKey == subCategoryId)
		for (position, button) in enumerate(self.raceButtons):
			button.SetSelected(position == self.raceIndex)
		self.__Clear()
		self.__ShowModel(subCategoryId)
		(name, empire, value) = player.GetRankInfo(categoryId, self.raceIndex, subCategoryId, 0)
		if name == "" or empire == 0 or value == 0:
			net.SendChatPacket("/rank_update %d %d %d" % (int(categoryId), int(self.raceIndex), int(subCategoryId)))
		else:
			self.LoadRankData(categoryId, self.raceIndex, subCategoryId, False)

	def __ShowModel(self, key):
		# eski siralamadaki gibi: metin tasi / patron / zindan canavari ya da karakter modeli
		try:
			(model, scale) = ModelOf(key)
			if model == 0:
				model = (self.raceIndex - 1) if self.raceIndex > 0 else player.GetRace()
			else:
				renderTarget.SetScale(RENDER_TARGET_INDEX, scale)
			renderTarget.SelectModel(RENDER_TARGET_INDEX, model)
		except Exception:
			pass

	def __SetCard(self, rank):
		if rank:
			self.cardValue.SetText("#%d" % rank)
			self.cardHint.SetText("ilk 10 i\xe7indesin")
		else:
			self.cardValue.SetText("-")
			self.cardHint.SetText("ilk 10 d\xfd\xfe\xfdnda")

	def __Clear(self):
		self.__SetCard(0)
		for row in self.rows:
			row.Hide()
		self.meRow.Hide()
		self.empty.Hide()

	# ------------------------------------------------------------ veri (sunucu cevabi)
	def LoadRankData(self, cate, race, index, isFirstUpdate):
		if (cate, race, index) != (self.typeIndex, self.raceIndex, self.rankIndex):
			return
		self.__Clear()
		myName = player.GetName()
		shown = 0
		myRank = 0
		for position in range(ROWS):
			(name, empire, value) = player.GetRankInfo(cate, race, index, position + 1)
			if name == "" or empire == 0 or value == 0:
				continue
			color = NORMAL
			if position == 0:
				color = GOLD
			elif position == 1:
				color = SILVER
			elif position == 2:
				color = BRONZE
			background = None
			if name == myName:
				myRank = position + 1
				color = MINE
				background = ROW_ME
			elif position % 2 == 0:
				background = ROW_A
			else:
				background = ROW_B
			self.rows[position].Set(position + 1, name, empire, value, color, background)
			shown += 1
		self.__SetCard(myRank)
		if not shown:
			self.empty.Show()
		if not myRank:
			self.UpdateLastItem(cate, race, index)

	def UpdateLastItem(self, cate, race, index):
		if (cate, race, index) != (self.typeIndex, self.raceIndex, self.rankIndex):
			return
		(name, empire, value) = player.GetRankInfo(cate, race, index, 11)
		if name != "" and name == player.GetName() and empire != 0:
			self.meRow.Set("-", name, empire, value, MINE, ROW_ME)

	# ------------------------------------------------------------ pencere
	def Open(self):
		if not self.loaded:
			self.loaded = True
			self.SelectType(0)
		self.SetPosition((wndMgr.GetScreenWidth() - PANEL_WIDTH) // 2, max(8, (wndMgr.GetScreenHeight() - self.GetHeight()) // 2))
		self.Show()
		self.SetTop()
		try:
			renderTarget.SetBackground(RENDER_TARGET_INDEX, "rank_window/render.png")
			renderTarget.SetVisibility(RENDER_TARGET_INDEX, True)
		except Exception:
			pass
		self.ChangeCategory(self.typeIndex, self.rankIndex)

	def __HideModel(self):
		try:
			renderTarget.SetVisibility(RENDER_TARGET_INDEX, False)
		except Exception:
			pass

	def Close(self):
		self.__HideModel()
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True
