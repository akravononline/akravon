// Cynex: d3d8.h icin com makrolari - arayuzler duz c++ soyut sinif olur
#pragma once
#include <windows.h>

#ifndef interface
#define interface struct
#endif
#define DECLSPEC_UUID(x)
#define DECLSPEC_NOVTABLE
#define MIDL_INTERFACE(x) struct
#define STDMETHOD(method) virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type, method) virtual type STDMETHODCALLTYPE method
#define STDMETHODIMP HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
#define PURE = 0
#define THIS_
#define THIS void
#define DECLARE_INTERFACE(iface) interface DECLSPEC_NOVTABLE iface
#define DECLARE_INTERFACE_(iface, baseiface) interface DECLSPEC_NOVTABLE iface : public baseiface

DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

interface IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
	virtual ULONG STDMETHODCALLTYPE Release() = 0;
	virtual ~IUnknown() {}
};
typedef IUnknown* LPUNKNOWN;

interface ISequentialStream : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead) = 0;
	virtual HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten) = 0;
};
interface IStream : public ISequentialStream {};
typedef IStream* LPSTREAM;

#define COINIT_MULTITHREADED 0x0
#define COINIT_APARTMENTTHREADED 0x2
#define CLSCTX_INPROC_SERVER 0x1
#define CLSCTX_ALL 0x17
inline HRESULT CoInitialize(LPVOID) { return S_OK; }
inline HRESULT CoInitializeEx(LPVOID, DWORD) { return S_OK; }
inline void CoUninitialize() {}
inline HRESULT CoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID* ppv) { if (ppv) *ppv = NULL; return E_NOINTERFACE; }
