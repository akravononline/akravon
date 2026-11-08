EMOTION_IDS = tuple(range(1, 19)) + (51, 52, 53)
SUPPORT_IDS = (122, 131, 130, 121, 163, 167, 180, 179, 164, 165, 166, 178)
PATH = "mobile_ui/akravon/actions/"


def GetEmotionIcon(index, size = 64):
	if index in EMOTION_IDS:
		return PATH + "emotion_%d_%d.png" % (index, size)
	return None


def GetSupportIcon(index):
	if index in SUPPORT_IDS:
		return PATH + "support_%d_64.png" % index
	return None


def GetCharacterEmotionIcon(index):
	if index in EMOTION_IDS:
		return PATH + "emotion_%d_square_40.png" % index
	return None
