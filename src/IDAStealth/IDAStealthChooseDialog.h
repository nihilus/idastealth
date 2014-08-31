#pragma once

#include <iostream>
#include "WTLCommon.h"
#include "IDAStealthOptionsDialog.h"
#include "IDAStealthLogWindow.h"

class CIDAStealthChooseDialog : public CDialogImpl<CIDAStealthChooseDialog>
{
public:

	enum { IDD = IDD_CHOOSE };

	CIDAStealthChooseDialog(HWND hWndParent) : hWndParent_(hWndParent) {}
	~CIDAStealthChooseDialog() {}

private:

	LRESULT OnClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnOptions(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		CIDAStealthOptionsDialog dlg("IDAStealth v1.3");
		dlg.DoModal(hWndParent_);
		return 0;
	}

	LRESULT OnLog(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		IDAStealthLogWindow& logWnd = IDAStealthLogWindow::getInstance();
		logWnd.show();
		EndDialog(wID);
		return 0;
	}

	BEGIN_MSG_MAP(CIDAStealthChooseDialog)
		COMMAND_ID_HANDLER(IDC_OPTIONS, OnOptions)
		COMMAND_ID_HANDLER(IDC_LOG, OnLog)
		COMMAND_ID_HANDLER(IDC_CLOSE, OnClose)
	END_MSG_MAP()

	HWND hWndParent_;
};