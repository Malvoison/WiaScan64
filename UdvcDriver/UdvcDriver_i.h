

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0595 */
/* at Thu Jan 10 16:34:13 2013
 */
/* Compiler settings for UdvcDriver.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0595 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

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

#ifndef __UdvcDriver_i_h__
#define __UdvcDriver_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IUVCPlugin_FWD_DEFINED__
#define __IUVCPlugin_FWD_DEFINED__
typedef interface IUVCPlugin IUVCPlugin;

#endif 	/* __IUVCPlugin_FWD_DEFINED__ */


#ifndef __UVCPlugin_FWD_DEFINED__
#define __UVCPlugin_FWD_DEFINED__

#ifdef __cplusplus
typedef class UVCPlugin UVCPlugin;
#else
typedef struct UVCPlugin UVCPlugin;
#endif /* __cplusplus */

#endif 	/* __UVCPlugin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IUVCPlugin_INTERFACE_DEFINED__
#define __IUVCPlugin_INTERFACE_DEFINED__

/* interface IUVCPlugin */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IUVCPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("94F61208-82DB-475E-85A6-24A67DF0D32F")
    IUVCPlugin : public IUnknown
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct IUVCPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUVCPlugin * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUVCPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUVCPlugin * This);
        
        END_INTERFACE
    } IUVCPluginVtbl;

    interface IUVCPlugin
    {
        CONST_VTBL struct IUVCPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUVCPlugin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUVCPlugin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUVCPlugin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUVCPlugin_INTERFACE_DEFINED__ */



#ifndef __UdvcDriverLib_LIBRARY_DEFINED__
#define __UdvcDriverLib_LIBRARY_DEFINED__

/* library UdvcDriverLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_UdvcDriverLib;

EXTERN_C const CLSID CLSID_UVCPlugin;

#ifdef __cplusplus

class DECLSPEC_UUID("2AF1A79C-40F5-4ED9-BC4F-19D13A0BEF62")
UVCPlugin;
#endif
#endif /* __UdvcDriverLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


