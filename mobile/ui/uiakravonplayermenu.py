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

W = 320
PAD = 14
TITLE_H = 50
BTN_W = (W - PAD * 2 - 10) // 2
BTN_H = 46
GAP = 10
MAX = 8
EXCHANGE_RANGE = 3000
CLOSE_DISTANCE = 6000	# hedeften bu kadar uzaklasinca menu kapanir

menu = None


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
			button = uiAkravonOffer.FlatButton(BTN_W, BTN_H, "", False, "Marcellus:17")
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
					entries.append(("Düelloyu Kabul Et", "pvp"))
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
			button.label.SetText(uiMobileStyle.TrUpper(label))
			row = index // 2
			column = index % 2
			button.SetPosition(PAD + column * (BTN_W + 10), TITLE_H + 14 + row * (BTN_H + GAP))
			button.Show()
			self.actions[index] = action
		rows = (len(entries) + 1) // 2
		height = TITLE_H + 14 + rows * (BTN_H + GAP) + PAD - GAP + 4
		self.SetSize(W, height)
		self.frame.SetSize(W, height)
		self.title.SetText(self.name)
		self.SetPosition((wndMgr.GetScreenWidth() - W) // 2, 56)
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
