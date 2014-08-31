#pragma once

#include <ocidl.h>
#include <olectl.h>
#include "ResourceItem.h"

#include "WTLCommon.h"

class CIDAStealthAboutPage :
	public CPropertyPageImpl<CIDAStealthAboutPage>
{
public:

	enum { IDD = IDD_ABOUT };
	
	CIDAStealthAboutPage()
	{
	}

	~CIDAStealthAboutPage()
	{
		GlobalFree(hGlobal_);
	}

private:

	BEGIN_MSG_MAP(CIDAStealthPropertyPage)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		NOTIFY_HANDLER_EX(IDC_SYSLINK, NM_CLICK, OnLinkClick)
		CHAIN_MSG_MAP(CPropertyPageImpl<CIDAStealthAboutPage>)
	END_MSG_MAP()

	// draw picture manually, because if we let windows draw it via the picture box
	// there are some nasty artifacts in the logo; no idea why
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(&ps);
		RECT r;
		GetWindowRect(&r);
		logo_.Draw(dc, ((r.right-r.left) - logo_.GetWidth()) / 2, 15);
		EndPaint(&ps);
		return 0;
	}

	BOOL OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lParam*/)
	{
		ResourceItem ri(GetModuleHandle("IDAStealth.plw"), IDR_LOGO, "JPG");
		LPVOID resData = ri.getData();
		DWORD jpegSize = ri.getDataSize();

		hGlobal_ = GlobalAlloc(GMEM_MOVEABLE, jpegSize);
		LPVOID jpegData = GlobalLock(hGlobal_);
		memcpy(jpegData, resData, jpegSize);
		GlobalUnlock(hGlobal_);

		LPSTREAM jpegStream  = NULL;
		CreateStreamOnHGlobal(hGlobal_, TRUE, &jpegStream);
		logo_.Load(jpegStream);
		jpegStream->Release();

		// initialize syslink control
		linkCtrl_ = GetDlgItem(IDC_SYSLINK);
		LITEM item;
		item.mask = LIF_ITEMINDEX | LIF_URL;
		item.iLink = 0;
		lstrcpyW(item.szUrl, L"http://newgre.net/idastealth");
		linkCtrl_.SetItem(&item);

		return FALSE;
	}

	LRESULT OnLinkClick(LPNMHDR pnmh)
	{
		PNMLINK pNMLink = (PNMLINK)pnmh;
		ShellExecuteW(m_hWnd, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
		return 0;
	}

	CLinkCtrl linkCtrl_;
	CImage logo_;
	HGLOBAL hGlobal_;
};