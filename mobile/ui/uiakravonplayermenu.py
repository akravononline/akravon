# -*- coding: utf-8 -*-
# AKRAVON: oyuncuya dokununca acilan menu (Fisilda, Ticaret, Duello, Grup, Arkadas, Lonca, Engelle).
# Mobil: iki sutun, 46 px yuksek buyuk dugmeler. Dugmeler bir kez olusturulan sabit havuzdan (MAX) yeniden kullanilir (cokme guvenligi).
# Kosullar oyunun orijinal hedef tahtasiyla ayni (uitarget.RefreshButton): krallik, mesafe, grup / lonca yetkisi, engel.
from __future__ import division
import app
import ui
import net
import chr
import player
import guild
import messenger
import constInfo
import wndMgr
import uiMobileStyle
import uiAkravonQuest
import uiAkravonInventory
import uiAkravonOffer
import uiAkravon

W = 440
PAD = 22
TITLE_H = 50
INNER_W = W - PAD * 2
BTN_W = (INNER_W - 10) // 2
ICON = 28
IMG = "d:/ymir work/ui/akravon_legacy/"
HUD_ICON = "mobile_ui/akravon/hud/btn_%s_flat_normal_xs.png"
ICONS = {"whisper" : "chat", "exchange" : "market", "pvp" : "swords", "friend" : "messenger", "party_invite" : "character", "party_request" : "character", "party_exit" : "send", "party_remove" : "send", "guild" : "guild", "block" : "shield", "unblock" : "shield"}
BTN_H = 44
GAP = 10
MAX = 8
EXCHANGE_RANGE = 3000
CLOSE_DISTANCE = 6000	# hedeften bu kadar uzaklasinca menu kapanir

menu = None


class MenuButton(uiMobileStyle.Pressable):
	# ikonlu dugme: sistem menusundeki satirlarla ayni cerceve (picker_row); ikon + yazi dugmenin ortasinda
	def __init__(self):
		uiMobileStyle.Pressable.__init__(self, BTN_W, BTN_H)
		self.panel = ui.ExpandedImageBox()
		self.panel.SetParent(self)
		self.panel.AddFlag("not_pick")
		self.panel.LoadImage(IMG + "pm_btn%d_1.png" % BTN_W)
		self.panel.Show()
		self.icon = ui.ExpandedImageBox()
		self.icon.SetParent(self)
		self.icon.AddFlag("not_pick")
		self.icon.Show()
		self.label = uiMobileStyle.CreateText(self, "", 0, BTN_H // 2 - 11, uiAkravonOffer.CREAM, "Marcellus:17", "left")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.panel = None
		self.icon = None
		self.label = None

	def Setup(self, text, iconName):
		# sola dayali: ikon + yazi; yazi sigmazsa kucuk punto
		self.icon.LoadImage(HUD_ICON % iconName)
		self.icon.SetScale(float(ICON) / max(1, self.icon.GetWidth()), float(ICON) / max(1, self.icon.GetHeight()))
		self.icon.SetPosition(12, (BTN_H - ICON) // 2)
		textX = 12 + ICON + 8
		limit = BTN_W - textX - 10
		self.label.SetText(uiMobileStyle.TrUpper(text))
		self.label.SetPosition(textX, BTN_H // 2 - 11)
		self.RefreshVisual()

	def RefreshVisual(self):
		if not getattr(self, "panel", None):
			return
		self.panel.LoadImage(IMG + "pm_btn%d_%d.png" % (BTN_W, 3 if self.pressed else 1))
		self.label.SetPackedFontColor(uiMobileStyle.GOLD_COLOR if self.pressed else uiMobileStyle.BRIGHT_COLOR)


class PlayerMenu(ui.Window):
	def __init__(self):
		ui.Window.__init__(self, "UI")
		self.AddFlag("float")
		self.frame = uiAkravonQuest.Board9()
		self.frame.SetParent(self)
		self.frame.AddFlag("not_pick")
		self.frame.Show()
		self.title = uiMobileStyle.CreateText(self, "", W // 2, 13, uiMobileStyle.Color(222, 184, 110), "Marcellus:21", "center")
		self.divider = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(150, 118, 58), PAD, TITLE_H, W - PAD * 2, 1)
		self.closeButton = uiAkravonInventory.CloseButton(self, W - PAD - 34, 8, ui.__mem_func__(self.Close))
		self.buttons = []
		for index in range(MAX):
			button = MenuButton()
			button.SetParent(self)
			button.SetEvent(ui.__mem_func__(getattr(self, "OnButton%d" % index)))
			button.Hide()
			self.buttons.append(button)
		self.actions = [None] * MAX
		self.vid = 0
		self.name = ""
		self.interface = None
		self.SetSize(W, 100)
		self.frame.SetSize(W, 100)
		self.Hide()

	def __del__(self):
		ui.Window.__del__(self)

	def OnButton0(self):
		self.Do(0)

	def OnButton1(self):
		self.Do(1)

	def OnButton2(self):
		self.Do(2)

	def OnButton3(self):
		self.Do(3)

	def OnButton4(self):
		self.Do(4)

	def OnButton5(self):
		self.Do(5)

	def OnButton6(self):
		self.Do(6)

	def OnButton7(self):
		self.Do(7)

	def Do(self, index):
		action = self.actions[index]
		vid = self.vid
		name = self.name
		self.Close()
		if action == "whisper":
			if self.interface:
				self.interface.OpenWhisperDialog(name)
		elif action == "exchange":
			net.SendExchangeStartPacket(vid)
		elif action == "pvp":
			net.SendChatPacket("/pvp %d" % vid)
		elif action == "friend":
			net.SendMessengerAddByVIDPacket(vid)
		elif action == "party_invite":
			net.SendPartyInvitePacket(vid)
		elif action == "party_exit":
			net.SendPartyExitPacket()
		elif action == "party_remove":
			net.SendPartyRemovePacketVID(vid)
		elif action == "party_request":
			net.SendChatPacket("/party_request " + str(vid))
		elif action == "guild":
			net.SendGuildAddMemberPacket(vid)
		elif action == "block":
			net.SendMessengerAddBlockByVIDPacket(vid)
		elif action == "unblock":
			messenger.RemoveBlock(constInfo.ME_KEY)
			net.SendMessengerRemoveBlockPacket(constInfo.ME_KEY, name)

	def Entries(self, vid, name):
		entries = [("Fısılda", "whisper")]
		sameEmpire = bool(player.IsSameEmpire(vid))
		if player.GetCharacterDistance(vid) <= EXCHANGE_RANGE:
			entries.append(("Ticaret", "exchange"))
		if player.IsPartyMember(vid):
			if player.IsPartyLeader(vid):
				entries.append(("Gruptan Ayrıl", "party_exit"))
			elif player.IsPartyLeader(player.GetMainCharacterIndex()):
				entries.append(("Gruptan At", "party_remove"))
		else:
			if sameEmpire and not player.IsCantFightInstance(vid):
				if player.IsRevengeInstance(vid):
					entries.append(("İntikam Al", "pvp"))
				elif player.IsChallengeInstance(vid):
					entries.append(("Düello Kabul", "pvp"))
				else:
					entries.append(("Düello", "pvp"))
			if sameEmpire:
				if player.IsPartyMember(player.GetMainCharacterIndex()):
					if player.IsPartyLeader(player.GetMainCharacterIndex()):
						entries.append(("Grup Daveti", "party_invite"))
				elif chr.IsPartyMember(vid):
					entries.append(("Gruba Katıl", "party_request"))
				else:
					entries.append(("Grup Daveti", "party_invite"))
		if sameEmpire and not messenger.IsFriendByName(name):
			entries.append(("Arkadaş Ekle", "friend"))
		if guild.MainPlayerHasAuthority(guild.AUTH_ADD_MEMBER) and not guild.IsMemberByName(name) and 0 == chr.GetGuildID(vid):
			entries.append(("Lonca Daveti", "guild"))
		if app.ENABLE_MESSENGER_BLOCK and name and not str(name)[0] == "[":
			if messenger.IsBlockByName(name):
				entries.append(("Engeli Kaldır", "unblock"))
			else:
				entries.append(("Engelle", "block"))
		return entries[:MAX]

	def Open(self, interface, vid, name):
		self.interface = interface
		self.vid = vid
		self.name = str(name)
		entries = self.Entries(vid, self.name)
		for button in self.buttons:
			button.Hide()
		self.actions = [None] * MAX
		for (index, (label, action)) in enumerate(entries):
			button = self.buttons[index]
			button.Setup(label, ICONS.get(action, "whisper"))
			self.actions[index] = action
		count = len(entries)
		width = W
		y = TITLE_H + 16
		for index in range(count):
			button = self.buttons[index]
			button.SetPosition(PAD + (index % 2) * (BTN_W + GAP), y + (index // 2) * (BTN_H + GAP))	# tek kalan dugme solda
			button.Show()
		rows = (count + 1) // 2
		height = y + rows * (BTN_H + GAP) - GAP + PAD
		self.SetSize(width, height)
		self.frame.SetSize(width, height)
		self.title.SetPosition(width // 2, 13)
		self.divider.SetSize(width - PAD * 2, 1)
		self.closeButton.SetPosition(width - PAD - 34, 8)
		self.title.SetText(self.name)
		self.SetPosition((wndMgr.GetScreenWidth() - width) // 2, 56)
		self.Show()
		self.SetTop()

	def Close(self):
		self.vid = 0
		self.Hide()

	def OnUpdate(self):
		if self.vid:
			try:
				distance = player.GetCharacterDistance(self.vid)
			except Exception:
				distance = -1
			if distance < 0 or distance > CLOSE_DISTANCE:
				self.Close()	# hedef gitti / cok uzaklasti

	def OnPressEscapeKey(self):
		self.Close()
		return True


def Show(interface, vid, name):
	# game.SetPCTargetBoard'dan cagrilir; menu acildiysa True (orijinal tahta gizlenir)
	global menu
	try:
		if not vid or player.IsMainCharacterIndex(vid):
			return False
		if chr.INSTANCE_TYPE_BUILDING == chr.GetInstanceType(vid):
			return False
		if player.IsPVPInstance(vid) or player.IsObserverMode():
			return False
		if menu is None:
			menu = PlayerMenu()
		menu.Open(interface, vid, name)
		return True
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonPlayerMenu: " + traceback.format_exc())
		return False


def Hide():
	if menu is not None:
		menu.Close()


def Destroy():
	global menu
	if menu is not None:
		menu.Hide()
	menu = None
