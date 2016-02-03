// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#if _MSC_VER >= 1400
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0501	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#if _MSC_VER < 1300
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500	// Uncomment this if using VC6 without platform SDK
#endif


#ifndef UNICODE
#define CGRIDLISTCTRLEX_GROUPMODE
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning( disable:4514 )	// warning C4514: unreferenced inline function has been remove
#pragma warning( disable:4710 )	// warning C4710: function not inlined
#pragma warning( disable:4711 )	// warning C4711: function selected for automatic inline expansion
#pragma warning( disable:4820 )	// warning C4820: X bytes padding added after data member

#pragma warning( push )
#pragma warning( disable:4548 )	// warning C4548: expression before comma has no effect; expected expression with side-effect
#pragma warning( disable:4812 )	// warning C4812: obsolete declaration style: please use '_CIP<_Interface,_IID>::_CIP' instead
#pragma warning( disable:4996 )	// warning C4996: This function or variable may be unsafe.
#pragma warning( disable:4005 )	// warning C4005: '_WIN32_WINDOWS' : macro redefinition
#pragma warning( disable:4668 )	// warning C4668: '__midl' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning( disable:4820 )	// warning C4820: X bytes padding added after data member
#pragma warning( disable:4619 )	// warning C4619: #pragma warning : there is no warning number
#pragma warning( disable:4625 )	// warning C4625: copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning( disable:4626 )	// warning C4626: assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning( disable:4365 )	// warning C4365: '=' : conversion from 'int' to 'size_t', signed/unsigned mismatch
#pragma warning( disable:4244 )	// warning C4244: 'return' : conversion from 'const time_t' to 'LONG_PTR', possible loss of data
#pragma warning( disable:4263 )	// warning C4263: member function does not override any base class virtual member function
#pragma warning( disable:4264 )	// warning C4264: no override available for virtual member function from base; function is hidden
#pragma warning( disable:4917 )	// warning C4917: a GUID can only be associated with a class, interface or namespace
#pragma warning( disable:4555 )	// warning C4555: expression has no effect; expected expression with side-effect
#pragma warning( disable:4640 )	// warning C4640: construction of local static object is not thread-safe
#pragma warning( disable:4571 )	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>

#pragma warning( pop )

#include <string>
#include <vector>
#include <algorithm>

using namespace std;