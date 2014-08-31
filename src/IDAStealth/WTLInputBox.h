#pragma once

#include <iostream>
#include <string>
#include "WTLCommon.h"

class CWTLInputBox : 
	public CDialogImpl<CWTLInputBox>,
	public CWinDataExchange<CWTLInputBox>
{
public:

	enum { IDD = IDD_INPUTBOX };

	CWTLInputBox() {}
	~CWTLInputBox() {}

	std::string getInput() const
	{
		if (txt_.GetLength())
		{
			std::string retVal = (const char*)txt_;
			return retVal;
		}
		else return "";
	}

private:

	LRESULT OnOkClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DoDataExchange(TRUE);
		EndDialog(0);
		return 0;
	}

	LRESULT OnCancelClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		txt_ = "";
		EndDialog(0);
		return 0;
	}

	BOOL OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{		
		DoDataExchange(FALSE);
		return TRUE;
	}

	WTL::CString txt_;
	
	BEGIN_DDX_MAP(CWTLInputBox)
		DDX_TEXT(IDC_EDITBOX, txt_)
	END_DDX_MAP()

	BEGIN_MSG_MAP(CWTLInputBox)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOkClick)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancelClick)
	END_MSG_MAP()
};