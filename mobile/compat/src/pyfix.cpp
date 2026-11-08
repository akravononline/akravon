// Cynex: Py_BuildValue bicim duzeltici (Python27/Python.h sarmalayicisi kullanir)
#include <string>

namespace cynexpy
{
	enum EKind { KIND_OTHER, KIND_S32, KIND_U32, KIND_S64, KIND_U64, KIND_PTR };

	static char MapInt(char c, int kind)
	{
		switch (kind)
		{
			case KIND_PTR:
			case KIND_S64:
				return 'L';
			case KIND_U64:
				return 'K';
			case KIND_S32:
				if (c == 'l' || c == 'L' || c == 'n')
					return 'i';
				if (c == 'k' || c == 'K')
					return 'I';
				return c;
			case KIND_U32:
				if (c == 'l' || c == 'L' || c == 'n' || c == 'k' || c == 'K')
					return 'I';
				return c;
		}
		return c;
	}

	const char* FixFormat(const char* fmt, const int* kinds, int count)
	{
		static thread_local std::string s_buf;
		if (!fmt)
			return fmt;

		bool changed = false;
		int arg = 0;
		for (int i = 0; fmt[i]; ++i)
		{
			const char c = fmt[i];
			switch (c)
			{
				case '(': case ')': case '[': case ']': case '{': case '}':
				case ':': case ',': case ' ': case '\t':
					break;

				case 's': case 'z': case 'u': case 't':
					if (fmt[i + 1] == '#')
					{
						++i;
						++arg;
					}
					++arg;
					break;

				case 'O': case 'S': case 'N':
					if (fmt[i + 1] == '&')
					{
						++i;
						++arg;
					}
					++arg;
					break;

				case 'b': case 'B': case 'h': case 'H': case 'i': case 'I':
				case 'l': case 'k': case 'L': case 'K': case 'n':
				{
					const char n = MapInt(c, arg < count ? kinds[arg] : KIND_OTHER);
					if (n != c)
					{
						if (!changed)
						{
							s_buf = fmt;
							changed = true;
						}
						s_buf[i] = n;
					}
					++arg;
					break;
				}

				default:
					++arg;
					break;
			}
		}
		return changed ? s_buf.c_str() : fmt;
	}
}
