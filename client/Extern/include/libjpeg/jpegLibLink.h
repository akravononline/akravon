#ifndef _JPEGLIBLINK_H_
#define _JPEGLIBLINK_H_

#ifndef _RUNTIME_LIBRARY

#if defined( _UNICODE )
#define _RUNTIME_CHARSET			"W"
#else
#define _RUNTIME_CHARSET			"A"
#endif

#if defined( _DEBUG ) || defined( DEBUG )
#define _RUNTIME_DEBUG				"_d"
#else
#define _RUNTIME_DEBUG				""
#endif

#endif

#pragma comment( lib, "jpeg" _RUNTIME_DEBUG ".lib" )

#endif /* !_JPEGLIBLINK_H_ */
