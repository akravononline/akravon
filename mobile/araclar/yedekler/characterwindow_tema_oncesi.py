# AKRAVON: otomatik uretildi (skin/build_akravon_charwin.py)
import uiScriptLocale

QUEST_ICON_BACKGROUND = 'd:/ymir work/ui/game/quest/slot_base.sub'

SMALL_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_00.sub"
MIDDLE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_01.sub"
LARGE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_03.sub"
ICON_SLOT_FILE = "d:/ymir work/ui/public/Slot_Base.sub"
FACE_SLOT_FILE = "d:/ymir work/ui/game/windows/box_face.sub"
ROOT_PATH = "d:/ymir work/ui/game/windows/"

LOCALE_PATH = uiScriptLocale.WINDOWS_PATH

window = {
	"name" : "CharacterWindow",
	"style" : ("movable", "float",),

	"x" : 24,
	"y" : (SCREEN_HEIGHT - 37 - 520) / 2,

	"width" : 620,
	"height" : 520,

	"children" :
	(
		{
			"name" : "board",
			"type" : "expanded_image",
			"image" : "d:/ymir work/ui/akravon_legacy/cw_board2.png",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width" : 620,
			"height" : 520,

			"children" :
			(
				{ "name" : "Skill_TitleBar", "type" : "titlebar", "style" : ("attach",), "x" : 542, "y" : 9, "width" : 64, "color" : "red", "children" : (
{ "name":"TitleName", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:12", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
				{ "name" : "Emoticon_TitleBar", "type" : "titlebar", "style" : ("attach",), "x" : 542, "y" : 9, "width" : 64, "color" : "red", "children" : (
{ "name":"TitleName", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:12", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
				{ "name" : "Quest_TitleBar", "type" : "titlebar", "style" : ("attach",), "x" : 542, "y" : 9, "width" : 64, "color" : "red", "children" : (
{ "name":"TitleName", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:12", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },

				## Tab Area
				{ "name":"Window_Title", "type":"text", "x":310, "y":11, "text":"Karakter", "fontname":"Marcellus:26", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"center" },
{ "name":"Header_Line", "type":"image", "x":16, "y":50, "image":"d:/ymir work/ui/akravon_legacy/cw_hline.png" },
{ "name":"Side_Line", "type":"image", "x":150, "y":68, "image":"d:/ymir work/ui/akravon_legacy/cw_vline.png" },
{ "name":"Face_Slot", "type":"image", "x":28, "y":68, "image":"d:/ymir work/ui/akravon_legacy/cw_face.png" },
{ "name" : "Face_Image", "type" : "expanded_image", "x" : 32, "y" : 72, "x_scale" : 2.0, "y_scale" : 2.0, "image" : "d:/ymir work/ui/game/windows/face_warrior.sub" },
{ "name":"Character_Name_Slot", "type":"image", "x":14, "y":172, "image":"d:/ymir work/ui/akravon_legacy/cw_clear.png", "children" : (
{ "name":"Character_Name", "type":"text", "x":61, "y":0, "text":"", "fontname":"Marcellus:20", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"center" },
) },
{ "name":"Guild_Name_Slot", "type":"image", "x":14, "y":196, "image":"d:/ymir work/ui/akravon_legacy/cw_clear.png", "children" : (
{ "name":"Guild_Name", "type":"text", "x":61, "y":0, "text":"", "fontname":"Lato:15", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"center" },
) },
{ "name":"Align_Card", "type":"image", "x":14, "y":216, "image":"d:/ymir work/ui/akravon_legacy/cw_aligncard.png", "children" : (
{ "name":"Align_Name", "type":"text", "x":10, "y":8, "text":"-", "fontname":"Marcellus:19", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Align_Value", "type":"text", "x":10, "y":32, "text":"Puan : 0", "fontname":"Lato:14", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name" : "Character_TitleBar", "type" : "titlebar", "style" : ("attach",), "x" : 542, "y" : 9, "width" : 64, "color" : "red", "children" : (
{ "name":"TitleName", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:12", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name" : "TabControl", "type" : "window", "x" : 14, "y" : 288, "width" : 122, "height" : 216, "children" : (
{ "name" : "Tab_01", "type" : "image", "x" : 0, "y" : 0, "image" : "d:/ymir work/ui/akravon_legacy/cw_nav_0.png" },
{ "name" : "Tab_02", "type" : "image", "x" : 0, "y" : 0, "image" : "d:/ymir work/ui/akravon_legacy/cw_nav_1.png" },
{ "name" : "Tab_03", "type" : "image", "x" : 0, "y" : 0, "image" : "d:/ymir work/ui/akravon_legacy/cw_nav_2.png" },
{ "name" : "Tab_04", "type" : "image", "x" : 0, "y" : 0, "image" : "d:/ymir work/ui/akravon_legacy/cw_nav_3.png" },
{ "name" : "Akravon_Nav_Bonus", "type" : "image", "x" : 0, "y" : 0, "image" : "d:/ymir work/ui/akravon_legacy/cw_nav_4.png" },
{ "name" : "Tab_Button_01", "type" : "radio_button", "x" : 0, "y" : 0, "width" : 122, "height" : 40 },
{ "name" : "Tab_Button_02", "type" : "radio_button", "x" : 0, "y" : 44, "width" : 122, "height" : 40 },
{ "name" : "Tab_Button_03", "type" : "radio_button", "x" : 0, "y" : 88, "width" : 122, "height" : 40 },
{ "name" : "Tab_Button_04", "type" : "radio_button", "x" : 0, "y" : 132, "width" : 122, "height" : 40 },
{ "name" : "Tab_Button_05", "type" : "radio_button", "x" : 0, "y" : 176, "width" : 122, "height" : 40 },
) },

				## Page Area
				{ "name" : "Character_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : 44, "width" : 620, "height" : 476, "children" : (
{ "name":"Akravon_Hidden", "type":"window", "x":-4000, "y":-4000, "width":10, "height":10, "children" : (
{ "name":"Status_Lv", "type":"window", "x":0, "y":0, "width":10, "height":10, "children" : (
{ "name":"Level_Value", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Status_CurExp", "type":"window", "x":0, "y":0, "width":10, "height":10, "children" : (
{ "name":"Exp_Value", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Status_RestExp", "type":"window", "x":0, "y":0, "width":10, "height":10, "children" : (
{ "name":"RestExp_Value", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"HEL_Label", "type":"window", "x":0, "y":0, "width":10, "height":10, "children" : (
{ "name":"HP_Value", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"SP_Label", "type":"window", "x":0, "y":0, "width":10, "height":10, "children" : (
{ "name":"SP_Value", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
) },
) },
{ "name":"Status_Standard", "type":"window", "x":170, "y":22, "width":426, "height":250, "children" : (
{ "name":"Character_Bar_01_Text", "type":"text", "x":0, "y":0, "text":"Karakter Durumu", "fontname":"Marcellus:20", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Status_Plus_Label", "type":"window", "x":276, "y":2, "width":150, "height":18, "children" : (
{ "name":"Status_Plus_Label_Text", "type":"text", "x":118, "y":0, "text":"Kalan Puan:", "fontname":"Lato:15", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"right" },
{ "name":"Status_Plus_Value", "type":"text", "x":150, "y":0, "text":"0", "fontname":"Lato:14", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"right" },
) },
{ "name":"HTH_Card", "type":"image", "x":0, "y":28, "image":"d:/ymir work/ui/akravon_legacy/cw_statrow.png", "children" : (
{ "name":"HTH_Name", "type":"text", "x":16, "y":5, "text":"VIT", "fontname":"Marcellus:22", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name":"HTH_Hint", "type":"text", "x":16, "y":29, "text":"Can ve savunma", "fontname":"Lato:16", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"HTH_Minus", "type":"button", "x":288, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_minus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_minus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_minus_down.png" },
{ "name":"HTH_Label", "type":"window", "x":322, "y":0, "width":60, "height":50, "children" : (
{ "name":"HTH_Value", "type":"text", "x":30, "y":11, "text":"0", "fontname":"Marcellus:24", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"center" },
) },
{ "name":"HTH_Plus", "type":"button", "x":382, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_plus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_plus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_plus_down.png" },
) },
{ "name":"INT_Card", "type":"image", "x":0, "y":82, "image":"d:/ymir work/ui/akravon_legacy/cw_statrow.png", "children" : (
{ "name":"INT_Name", "type":"text", "x":16, "y":5, "text":"INT", "fontname":"Marcellus:22", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name":"INT_Hint", "type":"text", "x":16, "y":29, "text":"Büyü ve SP", "fontname":"Lato:16", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"INT_Minus", "type":"button", "x":288, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_minus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_minus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_minus_down.png" },
{ "name":"INT_Label", "type":"window", "x":322, "y":0, "width":60, "height":50, "children" : (
{ "name":"INT_Value", "type":"text", "x":30, "y":11, "text":"0", "fontname":"Marcellus:24", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"center" },
) },
{ "name":"INT_Plus", "type":"button", "x":382, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_plus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_plus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_plus_down.png" },
) },
{ "name":"STR_Card", "type":"image", "x":0, "y":136, "image":"d:/ymir work/ui/akravon_legacy/cw_statrow.png", "children" : (
{ "name":"STR_Name", "type":"text", "x":16, "y":5, "text":"STR", "fontname":"Marcellus:22", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name":"STR_Hint", "type":"text", "x":16, "y":29, "text":"Saldýrý gücü", "fontname":"Lato:16", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"STR_Minus", "type":"button", "x":288, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_minus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_minus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_minus_down.png" },
{ "name":"STR_Label", "type":"window", "x":322, "y":0, "width":60, "height":50, "children" : (
{ "name":"STR_Value", "type":"text", "x":30, "y":11, "text":"0", "fontname":"Marcellus:24", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"center" },
) },
{ "name":"STR_Plus", "type":"button", "x":382, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_plus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_plus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_plus_down.png" },
) },
{ "name":"DEX_Card", "type":"image", "x":0, "y":190, "image":"d:/ymir work/ui/akravon_legacy/cw_statrow.png", "children" : (
{ "name":"DEX_Name", "type":"text", "x":16, "y":5, "text":"DEX", "fontname":"Marcellus:22", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name":"DEX_Hint", "type":"text", "x":16, "y":29, "text":"Ýsabet, kaçýnma", "fontname":"Lato:16", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"DEX_Minus", "type":"button", "x":288, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_minus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_minus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_minus_down.png" },
{ "name":"DEX_Label", "type":"window", "x":322, "y":0, "width":60, "height":50, "children" : (
{ "name":"DEX_Value", "type":"text", "x":30, "y":11, "text":"0", "fontname":"Marcellus:24", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"center" },
) },
{ "name":"DEX_Plus", "type":"button", "x":382, "y":10, "default_image":"d:/ymir work/ui/akravon_legacy/cw_plus_up.png", "over_image":"d:/ymir work/ui/akravon_legacy/cw_plus_over.png", "down_image":"d:/ymir work/ui/akravon_legacy/cw_plus_down.png" },
) },
) },
{ "name":"Status_Extent", "type":"window", "x":170, "y":288, "width":426, "height":150, "children" : (
{ "name":"Status_Extent_Label", "type":"text", "x":0, "y":0, "text":"Savaþ Deðerleri", "fontname":"Marcellus:20", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name":"ATT_Row", "type":"window", "x":0, "y":32, "width":207, "height":28, "children" : (
{ "name":"ATT_Name", "type":"text", "x":0, "y":1, "text":"Saldýrý", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"ATT_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"ATT_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
{ "name":"DEF_Row", "type":"window", "x":219, "y":32, "width":207, "height":28, "children" : (
{ "name":"DEF_Name", "type":"text", "x":0, "y":1, "text":"Savunma", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"DEF_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"DEF_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
{ "name":"MATT_Row", "type":"window", "x":0, "y":62, "width":207, "height":28, "children" : (
{ "name":"MATT_Name", "type":"text", "x":0, "y":1, "text":"Büyü Hasarý", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"MATT_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"MATT_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
{ "name":"MDEF_Row", "type":"window", "x":219, "y":62, "width":207, "height":28, "children" : (
{ "name":"MDEF_Name", "type":"text", "x":0, "y":1, "text":"Büyü Savunmasý", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"MDEF_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"MDEF_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
{ "name":"MSPD_Row", "type":"window", "x":0, "y":92, "width":207, "height":28, "children" : (
{ "name":"MSPD_Name", "type":"text", "x":0, "y":1, "text":"Hareket Hýzý", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"MSPD_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"MSPD_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
{ "name":"ASPD_Row", "type":"window", "x":219, "y":92, "width":207, "height":28, "children" : (
{ "name":"ASPD_Name", "type":"text", "x":0, "y":1, "text":"Saldýrý Hýzý", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"ASPD_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"ASPD_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
{ "name":"CSPD_Row", "type":"window", "x":0, "y":122, "width":207, "height":28, "children" : (
{ "name":"CSPD_Name", "type":"text", "x":0, "y":1, "text":"Büyü Hýzý", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"CSPD_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"CSPD_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
{ "name":"ER_Row", "type":"window", "x":219, "y":122, "width":207, "height":28, "children" : (
{ "name":"ER_Name", "type":"text", "x":0, "y":1, "text":"Savuþturma", "fontname":"Lato:17", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"ER_Value", "type":"text", "x":207, "y":1, "text":"0", "fontname":"Lato:18", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"right" },
{ "name":"ER_Line", "type":"image", "x":0, "y":26, "image":"d:/ymir work/ui/akravon_legacy/cw_line.png" },
) },
) },
) },
				{ "name" : "Skill_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : 44, "width" : 620, "height" : 476, "children" : (
{ "name":"Active_Skill_Point_Label", "type":"window", "x":170, "y":28, "width":160, "height":20, "children" : (
{ "name":"Active_Skill_Point_Text", "type":"text", "x":0, "y":0, "text":"Kalan Puan:", "fontname":"Lato:16", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Active_Skill_Point_Value", "type":"text", "x":92, "y":0, "text":"0", "fontname":"Lato:17", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Skill_Group_Bar", "type":"window", "x":422, "y":22, "width":174, "height":30, "children" : (
{ "name" : "Skill_Group_Button_1", "type" : "radio_button", "x" : 0, "y" : 0, "text" : "", "text_color" : 0xFFE8E2D2, "default_image" : "d:/ymir work/ui/akravon_legacy/cw_group_normal.png", "over_image" : "d:/ymir work/ui/akravon_legacy/cw_group_over.png", "down_image" : "d:/ymir work/ui/akravon_legacy/cw_group_down.png" },
{ "name" : "Skill_Group_Button_2", "type" : "radio_button", "x" : 90, "y" : 0, "text" : "", "text_color" : 0xFFE8E2D2, "default_image" : "d:/ymir work/ui/akravon_legacy/cw_group_normal.png", "over_image" : "d:/ymir work/ui/akravon_legacy/cw_group_over.png", "down_image" : "d:/ymir work/ui/akravon_legacy/cw_group_down.png" },
{ "name":"Active_Skill_Group_Name", "type":"text", "x":0, "y":6, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Skill_Cards", "type":"window", "x":170, "y":66, "width":426, "height":162, "children" : (
{ "name":"Skill_Card_0", "type":"image", "x":0, "y":0, "image":"d:/ymir work/ui/akravon_legacy/cw_skillcard2.png", "children" : (
{ "name":"Skill_Ring_0", "type":"image", "x":6, "y":4, "image":"d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name":"Skill_Name_0", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Skill_Grade_0", "type":"text", "x":58, "y":27, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Skill_Card_1", "type":"image", "x":219, "y":0, "image":"d:/ymir work/ui/akravon_legacy/cw_skillcard2.png", "children" : (
{ "name":"Skill_Ring_1", "type":"image", "x":6, "y":4, "image":"d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name":"Skill_Name_1", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Skill_Grade_1", "type":"text", "x":58, "y":27, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Skill_Card_2", "type":"image", "x":0, "y":54, "image":"d:/ymir work/ui/akravon_legacy/cw_skillcard2.png", "children" : (
{ "name":"Skill_Ring_2", "type":"image", "x":6, "y":4, "image":"d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name":"Skill_Name_2", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Skill_Grade_2", "type":"text", "x":58, "y":27, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Skill_Card_3", "type":"image", "x":219, "y":54, "image":"d:/ymir work/ui/akravon_legacy/cw_skillcard2.png", "children" : (
{ "name":"Skill_Ring_3", "type":"image", "x":6, "y":4, "image":"d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name":"Skill_Name_3", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Skill_Grade_3", "type":"text", "x":58, "y":27, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Skill_Card_4", "type":"image", "x":0, "y":108, "image":"d:/ymir work/ui/akravon_legacy/cw_skillcard2.png", "children" : (
{ "name":"Skill_Ring_4", "type":"image", "x":6, "y":4, "image":"d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name":"Skill_Name_4", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Skill_Grade_4", "type":"text", "x":58, "y":27, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Skill_Card_5", "type":"image", "x":219, "y":108, "image":"d:/ymir work/ui/akravon_legacy/cw_skillcard2.png", "children" : (
{ "name":"Skill_Ring_5", "type":"image", "x":6, "y":4, "image":"d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name":"Skill_Name_5", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:15", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Skill_Grade_5", "type":"text", "x":58, "y":27, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
) },
{ "name" : "Skill_Active_Slot", "type" : "slot", "x" : 170, "y" : 66, "width" : 426, "height" : 162, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "slot" : ({"index":1, "x":4, "y":3, "width":44, "height":44}, {"index":2, "x":223, "y":3, "width":44, "height":44}, {"index":3, "x":4, "y":57, "width":44, "height":44}, {"index":4, "x":223, "y":57, "width":44, "height":44}, {"index":5, "x":4, "y":111, "width":44, "height":44}, {"index":6, "x":223, "y":111, "width":44, "height":44}) },
{ "name":"Skill_HD_Layer", "type":"window", "x":170, "y":66, "width":426, "height":162, "children" : (
{ "name" : "Skill_HD_0", "type" : "expanded_image", "style" : ("not_pick",), "x" : 4, "y" : 3, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Skill_HD_1", "type" : "expanded_image", "style" : ("not_pick",), "x" : 223, "y" : 3, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Skill_HD_2", "type" : "expanded_image", "style" : ("not_pick",), "x" : 4, "y" : 57, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Skill_HD_3", "type" : "expanded_image", "style" : ("not_pick",), "x" : 223, "y" : 57, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Skill_HD_4", "type" : "expanded_image", "style" : ("not_pick",), "x" : 4, "y" : 111, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Skill_HD_5", "type" : "expanded_image", "style" : ("not_pick",), "x" : 223, "y" : 111, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
) },
{ "name":"Support_Skill_Group_Name", "type":"text", "x":170, "y":234, "text":"Destek Yetenekleri", "fontname":"Lato:15", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Cards", "type":"window", "x":170, "y":262, "width":426, "height":174, "children" : (
{ "name":"Support_Card_0", "type":"window", "x":0, "y":0, "width":207, "height":54, "children" : (
{ "name":"Support_Name_0", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Grade_0", "type":"text", "x":58, "y":28, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Support_Card_1", "type":"window", "x":219, "y":0, "width":207, "height":54, "children" : (
{ "name":"Support_Name_1", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Grade_1", "type":"text", "x":58, "y":28, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Support_Card_2", "type":"window", "x":0, "y":58, "width":207, "height":54, "children" : (
{ "name":"Support_Name_2", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Grade_2", "type":"text", "x":58, "y":28, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Support_Card_3", "type":"window", "x":219, "y":58, "width":207, "height":54, "children" : (
{ "name":"Support_Name_3", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Grade_3", "type":"text", "x":58, "y":28, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Support_Card_4", "type":"window", "x":0, "y":116, "width":207, "height":54, "children" : (
{ "name":"Support_Name_4", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Grade_4", "type":"text", "x":58, "y":28, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
{ "name":"Support_Card_5", "type":"window", "x":219, "y":116, "width":207, "height":54, "children" : (
{ "name":"Support_Name_5", "type":"text", "x":58, "y":6, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Grade_5", "type":"text", "x":58, "y":28, "text":"", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
) },
{ "name" : "Skill_ETC_Slot", "type" : "slot", "x" : 170, "y" : 262, "width" : 426, "height" : 174, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "slot" : ({"index":101, "x":4, "y":5, "width":44, "height":44}, {"index":102, "x":223, "y":5, "width":44, "height":44}, {"index":103, "x":4, "y":63, "width":44, "height":44}, {"index":104, "x":223, "y":63, "width":44, "height":44}, {"index":105, "x":4, "y":121, "width":44, "height":44}, {"index":106, "x":223, "y":121, "width":44, "height":44}) },
{ "name":"Support_HD_Layer", "type":"window", "x":170, "y":262, "width":426, "height":174, "children" : (
{ "name" : "Support_HD_0", "type" : "expanded_image", "style" : ("not_pick",), "x" : 2, "y" : 3, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Support_HD_1", "type" : "expanded_image", "style" : ("not_pick",), "x" : 221, "y" : 3, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Support_HD_2", "type" : "expanded_image", "style" : ("not_pick",), "x" : 2, "y" : 61, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Support_HD_3", "type" : "expanded_image", "style" : ("not_pick",), "x" : 221, "y" : 61, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Support_HD_4", "type" : "expanded_image", "style" : ("not_pick",), "x" : 2, "y" : 119, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
{ "name" : "Support_HD_5", "type" : "expanded_image", "style" : ("not_pick",), "x" : 221, "y" : 119, "image" : "d:/ymir work/ui/akravon_legacy/cw_clear32.png" },
) },
{ "name":"Support_Prev", "type":"button", "x":454, "y":228, "default_image":"d:/ymir work/ui/akravon_legacy/qs_lt_1.png", "over_image":"d:/ymir work/ui/akravon_legacy/qs_lt_1.png", "down_image":"d:/ymir work/ui/akravon_legacy/qs_lt_3.png" },
{ "name":"Support_Page_Text", "type":"text", "x":524, "y":235, "text":"1/1", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"center" },
{ "name":"Support_Next", "type":"button", "x":550, "y":228, "default_image":"d:/ymir work/ui/akravon_legacy/qs_rt_1.png", "over_image":"d:/ymir work/ui/akravon_legacy/qs_rt_1.png", "down_image":"d:/ymir work/ui/akravon_legacy/qs_rt_3.png" },
{ "name":"Akravon_Hidden_Skill", "type":"window", "x":-4000, "y":-4000, "width":10, "height":10, "children" : (
{ "name":"Support_Skill_Point_Label", "type":"window", "x":0, "y":0, "width":10, "height":10, "children" : (
{ "name":"Support_Skill_Point_Text", "type":"text", "x":0, "y":0, "text":"", "fontname":"Lato:12", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Support_Skill_Point_Value", "type":"text", "x":0, "y":0, "text":"0", "fontname":"Lato:12", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
) },
) },
) },
				{ "name" : "Emoticon_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : 44, "width" : 620, "height" : 476, "children" : (
{ "name":"Action_Bar_Text", "type":"text", "x":170, "y":22, "text":"Tekli Hareketler", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name" : "SoloEmotionSlot", "type" : "grid_table", "x" : 170, "y" : 46, "start_index" : 1, "x_count" : 6, "y_count" : 3, "x_step" : 40, "y_step" : 40, "x_blank" : 37, "y_blank" : 8, "image" : "d:/ymir work/ui/akravon_legacy/cw_slot40.png" },
{ "name":"Reaction_Bar_Text", "type":"text", "x":170, "y":202, "text":"Karþýlýklý Hareketler", "fontname":"Lato:16", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name" : "DualEmotionSlot", "type" : "grid_table", "x" : 170, "y" : 226, "start_index" : 51, "x_count" : 6, "y_count" : 3, "x_step" : 40, "y_step" : 40, "x_blank" : 37, "y_blank" : 8, "image" : "d:/ymir work/ui/akravon_legacy/cw_slot40.png" },
) },
				{ "name" : "Quest_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : 44, "width" : 620, "height" : 476, "children" : (
{ "name":"Quest_Title", "type":"text", "x":170, "y":22, "text":"Görevler", "fontname":"Marcellus:20", "r":0.89, "g":0.76, "b":0.50, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_Card_0", "type":"image", "x":170, "y":54, "image":"d:/ymir work/ui/akravon_legacy/cw_questcard.png" },
{ "name":"Quest_Card_1", "type":"image", "x":170, "y":124, "image":"d:/ymir work/ui/akravon_legacy/cw_questcard.png" },
{ "name":"Quest_Card_2", "type":"image", "x":170, "y":194, "image":"d:/ymir work/ui/akravon_legacy/cw_questcard.png" },
{ "name":"Quest_Card_3", "type":"image", "x":170, "y":264, "image":"d:/ymir work/ui/akravon_legacy/cw_questcard.png" },
{ "name":"Quest_Card_4", "type":"image", "x":170, "y":334, "image":"d:/ymir work/ui/akravon_legacy/cw_questcard.png" },
{ "name" : "Quest_Slot", "type" : "grid_table", "x" : 182, "y" : 69, "start_index" : 0, "x_count" : 1, "y_count" : 5, "x_step" : 32, "y_step" : 32, "y_blank" : 38, "image" : "d:/ymir work/ui/game/quest/slot_base.sub" },
{ "name" : "Quest_ScrollBar", "type" : "scrollbar", "x" : 582, "y" : 54, "size" : 342 },
{ "name":"Quest_Name_00", "type":"text", "x":226, "y":63, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastTime_00", "type":"text", "x":226, "y":84, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastCount_00", "type":"text", "x":226, "y":100, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_Name_01", "type":"text", "x":226, "y":133, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastTime_01", "type":"text", "x":226, "y":154, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastCount_01", "type":"text", "x":226, "y":170, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_Name_02", "type":"text", "x":226, "y":203, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastTime_02", "type":"text", "x":226, "y":224, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastCount_02", "type":"text", "x":226, "y":240, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_Name_03", "type":"text", "x":226, "y":273, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastTime_03", "type":"text", "x":226, "y":294, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastCount_03", "type":"text", "x":226, "y":310, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_Name_04", "type":"text", "x":226, "y":343, "text":"", "fontname":"Lato:16", "r":0.96, "g":0.94, "b":0.88, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastTime_04", "type":"text", "x":226, "y":364, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
{ "name":"Quest_LastCount_04", "type":"text", "x":226, "y":380, "text":"", "fontname":"Lato:14", "r":0.63, "g":0.59, "b":0.51, "a":1.0, "text_horizontal_align":"left" },
) },
			),
		},
	),
}
