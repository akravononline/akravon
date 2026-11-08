from __future__ import division
# AKRAVON: oyuna girerken / isinlanirken yukleme ekrani - sahne, logo, altin ilerleme cubugu, asama yazisi, ipucu
import app
import dbg
import wndMgr
import ui
import introLoading
import uiScriptLocale
import uiMobileStyle
import uiAkravon
import uiMobileCreate

from uiMobileCreate import HDImage, HDSize, CenterText, CREAM, MUTED_LABEL

BAR_WIDTH = 640
BAR_INSET_X = 14
BAR_INSET_Y = 9
BAR_Y_FROM_BOTTOM = 92
LOGO_SCALE = 0.62
TIP_INTERVAL = 4.5
FONT_STAGE = "Marcellus:22"
FONT_PERCENT = "Marcellus:22"
FONT_TIP = "Lato:17"
GOLD = uiMobileStyle.Color(236, 200, 128)

# ilerleme yuzdesi -> asama yazisi
STAGES = (
	(0, "MOBILE_LOAD_CONNECT"),
	(10, "MOBILE_LOAD_MAP"),
	(40, "MOBILE_LOAD_EFFECT"),
	(50, "MOBILE_LOAD_CHARACTERS"),
	(90, "MOBILE_LOAD_WORLD"),
	(100, "MOBILE_LOAD_ENTER"),
)
TIPS = ("MOBILE_TIP_1", "MOBILE_TIP_2", "MOBILE_TIP_3", "MOBILE_TIP_4", "MOBILE_TIP_5", "MOBILE_TIP_6")


def Locale(name):
	return getattr(uiScriptLocale, name, name)


class MobileLoadingWindow(introLoading.LoadingWindow):
	def __init__(self, stream, isWarp = False):
		self.mobileItems = []
		self.progress = 0.0
		self.shownProgress = 0.0
		self.fill = None
		self.spark = None
		self.percent = None
		self.stage = None
		self.tip = None
		self.tipIndex = 0
		self.nextTipTime = 0.0
		self.fillSpan = (0, 0, 0)
		self.sparkY = 0
		introLoading.LoadingWindow.__init__(self, stream, isWarp)

	def Open(self):
		introLoading.LoadingWindow.Open(self)
		try:
			self.__BuildMobile()
		except Exception as error:
			import traceback
			dbg.TraceError("MobileLoadingWindow.Open - layout error: %s" % traceback.format_exc())

	def Close(self):
		for item in self.mobileItems:
			item.Hide()
		self.mobileItems = []
		self.fill = None
		self.spark = None
		self.percent = None
		self.stage = None
		self.tip = None
		introLoading.LoadingWindow.Close(self)

	def __Keep(self, item):
		self.mobileItems.append(item)
		return item

	def __BuildMobile(self):
		# orijinal Open'in olmayan yukleme resmi hatasi bir sonraki cagrida yuzeye cikar - once temizle
		try:
			wndMgr.GetScreenWidth()
			if self.loadingImage:
				self.loadingImage.SetPosition(0, 0)
		except Exception:
			pass
		width = wndMgr.GetScreenWidth()
		height = wndMgr.GetScreenHeight()
		try:
			if self.loadingImage:
				self.loadingImage.Hide()
		except Exception:
			pass

		bg = self.__Keep(uiAkravon.CreateImage(self, "loading_bg"))
		bg.SetScale(float(width) / 1280.0, float(height) / 720.0)
		for item in uiMobileCreate.BuildScreenFrame(self, width, height, height):
			self.__Keep(item)

		logo = self.__Keep(uiAkravon.CreateImage(self, "akravon_logo"))
		logo.SetScale(LOGO_SCALE, LOGO_SCALE)
		logoW = int(logo.GetWidth() * LOGO_SCALE)
		logo.SetPosition((width - logoW) // 2, 38)

		(barW, barH) = HDSize("loading_bar")
		barX = (width - barW) // 2
		barY = height - BAR_Y_FROM_BOTTOM
		self.stage = self.__Keep(CenterText(self, Locale(STAGES[0][1]), width // 2, barY - 22, CREAM, FONT_STAGE))
		fillX = barX + BAR_INSET_X
		fillY = barY + BAR_INSET_Y
		fillW = barW - BAR_INSET_X * 2
		fillH = barH - BAR_INSET_Y * 2
		self.fillSpan = (fillX, fillW, fillH)
		self.__Keep(uiMobileStyle.CreateBar(self, uiMobileStyle.Color(4, 6, 12, 230), fillX, fillY, fillW, fillH))
		self.fill = self.__Keep(uiAkravon.CreateImage(self, "loading_fill", fillX, fillY))
		self.fill.SetScale(0.0, fillH / 28.0)
		self.__Keep(HDImage(self, "loading_bar", barX, barY))
		self.sparkY = fillY + fillH // 2 - 12
		self.spark = self.__Keep(HDImage(self, "loading_spark", fillX - 12, self.sparkY))
		self.percent = self.__Keep(uiMobileStyle.CreateText(self, "0%", barX + barW + 14, barY + barH // 2 - 12, GOLD, FONT_PERCENT))

		self.tipIndex = app.GetRandom(0, len(TIPS) - 1)
		self.tip = self.__Keep(CenterText(self, "", width // 2, barY + barH + 28, MUTED_LABEL, FONT_TIP))
		self.__ShowTip()
		self.__ApplyProgress(0.0)

	def __ShowTip(self):
		if self.tip:
			self.tip.SetText(Locale(TIPS[self.tipIndex % len(TIPS)]))
		self.tipIndex += 1
		self.nextTipTime = app.GetTime() + TIP_INTERVAL

	def __ApplyProgress(self, value):
		if not self.fill:
			return
		(fillX, fillW, fillH) = self.fillSpan
		value = max(0.0, min(100.0, value))
		pixels = fillW * value / 100.0
		self.fill.SetScale(pixels / 8.0, fillH / 28.0)
		self.spark.SetPosition(int(fillX + pixels) - 12, self.sparkY)
		self.percent.SetText("%d%%" % int(value + 0.5))
		label = STAGES[0][1]
		for (threshold, name) in STAGES:
			if value >= threshold:
				label = name
		self.stage.SetText(Locale(label))

	def _LoadingWindow__SetProgress(self, p):
		self.progress = float(p)

	def OnUpdate(self):
		introLoading.LoadingWindow.OnUpdate(self)
		if not self.fill:
			return
		# cubuk hedefe yumusak yaklasir
		if self.shownProgress < self.progress:
			self.shownProgress = min(self.progress, self.shownProgress + max(0.6, (self.progress - self.shownProgress) * 0.35))
			self.__ApplyProgress(self.shownProgress)
		if app.GetTime() >= self.nextTipTime:
			self.__ShowTip()
