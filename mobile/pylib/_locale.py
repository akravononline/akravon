# Cynex: android'de C _locale yok - setlocale('turkish') gibi cagrilar hata vermesin
CHAR_MAX = 127
LC_CTYPE = 0
LC_NUMERIC = 1
LC_TIME = 2
LC_COLLATE = 3
LC_MONETARY = 4
LC_MESSAGES = 5
LC_ALL = 6


class Error(Exception):
	pass


_current = {}


def setlocale(category, value=None):
	if value is None:
		return _current.get(category, "C")
	if category == LC_ALL:
		for c in (LC_CTYPE, LC_NUMERIC, LC_TIME, LC_COLLATE, LC_MONETARY, LC_MESSAGES, LC_ALL):
			_current[c] = value or "C"
	else:
		_current[category] = value or "C"
	return value or "C"


def localeconv():
	return {
		"grouping": [127],
		"currency_symbol": "",
		"n_sign_posn": 127,
		"p_cs_precedes": 127,
		"n_cs_precedes": 127,
		"mon_grouping": [],
		"n_sep_by_space": 127,
		"decimal_point": ".",
		"negative_sign": "",
		"positive_sign": "",
		"p_sep_by_space": 127,
		"int_curr_symbol": "",
		"p_sign_posn": 127,
		"thousands_sep": "",
		"mon_thousands_sep": "",
		"frac_digits": 127,
		"mon_decimal_point": "",
		"int_frac_digits": 127,
	}


def strcoll(a, b):
	return cmp(a, b)


def strxfrm(s):
	return s
