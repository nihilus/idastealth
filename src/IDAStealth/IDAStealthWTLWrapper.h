// global wrapper to encapsulate all WTL boiler plate code
// and to initialize the IDAStealth GUI (singleton class)

#include "WTLCommon.h"
#include "IDAStealthChooseDialog.h"
#include <iostream>

#if defined _M_IX86
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

class IDAStealthWTLWrapper
{
public:

	static IDAStealthWTLWrapper& getInstance()
	{
		static IDAStealthWTLWrapper instance_;
		return instance_;
	}

	~IDAStealthWTLWrapper()
	{
		module_.Term();
		::CoUninitialize();
	}

	void showGUI(HWND hWndParent)
	{
		//CIDAStealthChooseDialog dlg(hWndParent);
		//dlg.DoModal(hWndParent);
		CIDAStealthOptionsDialog dlg("IDAStealth v1.3");
		dlg.DoModal(hWndParent);
	}

private:

	IDAStealthWTLWrapper()
	{
		HRESULT hRes = ::CoInitialize(NULL);
		ATLASSERT(SUCCEEDED(hRes));
		AtlInitCommonControls(ICC_BAR_CLASSES | ICC_LINK_CLASS);
		hRes = module_.Init(NULL, GetModuleHandle(NULL));
		ATLASSERT(SUCCEEDED(hRes));
	}

	IDAStealthWTLWrapper(IDAStealthWTLWrapper const&);
	IDAStealthWTLWrapper& operator=(IDAStealthWTLWrapper const&);

	CAppModule module_;
};