// Cynex: midl uretimi basliklar (dimm.h, msctf.h) derlensin diye
#pragma once
#include <objbase.h>
#define __RPC_FAR
#define __RPC_API
#define __RPC_USER
#define __RPC_STUB
#define RPC_ENTRY
#define EXTERN_C extern "C"
#define DECLSPEC_NOVTABLE
typedef void* RPC_IF_HANDLE;
typedef void* I_RPC_HANDLE;
typedef struct _RPC_MESSAGE* PRPC_MESSAGE;
interface IRpcStubBuffer;
interface IRpcChannelBuffer;
