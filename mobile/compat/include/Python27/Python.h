// Cynex
#include <Python.h>
#ifdef BYTE
#undef BYTE	// Cynex: python bitset.h BYTE = char yapiyor, windows BYTE (unsigned char) ezilmesin
#endif

#ifdef __cplusplus
// Cynex: Py_BuildValue bicimi gercek arguman tipine gore duzeltilir
//  - isaretci "i" ile gidiyorsa 64 bit "L" (python tutamaclari)
//  - LONG/DWORD (32 bit) "l"/"k" ile gidiyorsa "i"/"I" (android'de long 64 bit)
#include <type_traits>
#include <stdint.h>

namespace cynexpy
{
	enum EKind { KIND_OTHER, KIND_S32, KIND_U32, KIND_S64, KIND_U64, KIND_PTR };

	template <class T> constexpr int KindOf()
	{
		typedef typename std::decay<T>::type U;
		if (std::is_pointer<U>::value)
		{
			typedef typename std::remove_cv<typename std::remove_pointer<U>::type>::type P;
			if (std::is_same<P, char>::value || std::is_same<P, unsigned char>::value || std::is_same<P, wchar_t>::value
				|| std::is_same<P, PyObject>::value || std::is_same<P, Py_UNICODE>::value || std::is_same<P, Py_complex>::value
				|| std::is_function<P>::value)
				return KIND_OTHER;
			return KIND_PTR;
		}
		if (std::is_integral<U>::value || std::is_enum<U>::value)
		{
			const bool isSigned = std::is_enum<U>::value ? true : std::is_signed<U>::value;
			if (sizeof(U) > 4)
				return isSigned ? KIND_S64 : KIND_U64;
			return (sizeof(U) == 4 && !isSigned) ? KIND_U32 : KIND_S32;
		}
		return KIND_OTHER;
	}

	template <class T> inline typename std::enable_if<std::is_pointer<typename std::decay<T>::type>::value && KindOf<T>() == KIND_PTR, long long>::type
	Conv(T v) { return (long long)(intptr_t)v; }

	template <class T> inline typename std::enable_if<!(std::is_pointer<typename std::decay<T>::type>::value && KindOf<T>() == KIND_PTR), T>::type
	Conv(T v) { return v; }

	const char* FixFormat(const char* fmt, const int* kinds, int count);
}

template <class... A> inline PyObject* CynexPy_BuildValue(const char* fmt, A... args)
{
	static const int s_kinds[] = { cynexpy::KindOf<A>()..., 0 };
	return (Py_BuildValue)(cynexpy::FixFormat(fmt, s_kinds, (int)sizeof...(A)), cynexpy::Conv<A>(args)...);
}

#define Py_BuildValue(...) CynexPy_BuildValue(__VA_ARGS__)
#endif
