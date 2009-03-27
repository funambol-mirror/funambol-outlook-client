

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Tue Nov 28 15:41:03 2006
 */
/* Compiler settings for .\FunambolAddin.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data
    VC __declspec() decoration level:
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __FunambolAddin_h__
#define __FunambolAddin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */

#ifndef __Iaddin_FWD_DEFINED__
#define __Iaddin_FWD_DEFINED__
typedef interface Iaddin Iaddin;
#endif 	/* __Iaddin_FWD_DEFINED__ */


#ifndef __addin_FWD_DEFINED__
#define __addin_FWD_DEFINED__

#ifdef __cplusplus
typedef class addin addin;
#else
typedef struct addin addin;
#endif /* __cplusplus */

#endif 	/* __addin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * );

#ifndef __Iaddin_INTERFACE_DEFINED__
#define __Iaddin_INTERFACE_DEFINED__

/* interface Iaddin */
/* [unique][helpstring][dual][uuid][object] */


EXTERN_C const IID IID_Iaddin;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("BC3DA6AD-08ED-42FB-A51C-F15805359EF3")
    Iaddin : public IDispatch
    {
    public:
    };

#else 	/* C style interface */

    typedef struct IaddinVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
            Iaddin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);

        ULONG ( STDMETHODCALLTYPE *AddRef )(
            Iaddin * This);

        ULONG ( STDMETHODCALLTYPE *Release )(
            Iaddin * This);

        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )(
            Iaddin * This,
            /* [out] */ UINT *pctinfo);

        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )(
            Iaddin * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);

        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )(
            Iaddin * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);

        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )(
            Iaddin * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);

        END_INTERFACE
    } IaddinVtbl;

    interface Iaddin
    {
        CONST_VTBL struct IaddinVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define Iaddin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Iaddin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Iaddin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Iaddin_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Iaddin_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Iaddin_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Iaddin_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __Iaddin_INTERFACE_DEFINED__ */



#ifndef __FUNAMBOLADDINLib_LIBRARY_DEFINED__
#define __FUNAMBOLADDINLib_LIBRARY_DEFINED__

/* library FUNAMBOLADDINLib */
/* [helpstring][version][uuid] */


EXTERN_C const IID LIBID_FUNAMBOLADDINLib;

EXTERN_C const CLSID CLSID_addin;

#ifdef __cplusplus

class DECLSPEC_UUID("2F84C560-A346-4E08-99DA-E37ECB529FB9")
addin;
#endif
#endif /* __FUNAMBOLADDINLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


