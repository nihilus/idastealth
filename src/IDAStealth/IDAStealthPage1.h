#pragma once

#include <HideDebugger/HideDebuggerConfig.h>
#include "WTLCommon.h"
#include "IDAStealthPropertyPage.h"
#include <iostream>
#include "resource.h"

class CIDAStealthOptionsDialog;

class CIDAStealthPage1 :
	public CIDAStealthPropertyPage<CIDAStealthPage1, IDD_DIALOG1>
{
public:

	enum { IDD = IDD_DIALOG1 };

	CIDAStealthPage1(CIDAStealthOptionsDialog* parentDlg) :
		CIDAStealthPropertyPage(parentDlg) {}
	~CIDAStealthPage1() {}

	void OnDataExchangeError(UINT nCtrlID, BOOL bSave);

protected:

	void updateGlobalEnableState(bool newState);
	void loadPageConfig();
	bool savePageConfig();

private:

	// we need a typedef - otherwise macro throws errors
	typedef CIDAStealthPropertyPage<CIDAStealthPage1, IDD_DIALOG1> BasePropertyPage;
	
	BEGIN_MSG_MAP(CIDAStealthPage1)
		COMMAND_HANDLER(IDC_DBGSTART, BN_CLICKED, OnDbgEnableClick)
		COMMAND_HANDLER(IDC_DBGATTACH, BN_CLICKED, OnDbgEnableClick)
		CHAIN_MSG_MAP(CPropertyPageImpl<CIDAStealthPage1>)
		CHAIN_MSG_MAP(BasePropertyPage)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CIDAStealthPage1)
		DDX_CHECK(IDC_NTQO, ntQueryObj_)
		DDX_CHECK(IDC_RTLNTGF, rtlGetFlags_)
		DDX_CHECK(IDC_NTQSI, ntQuerySysInfo_)
		DDX_CHECK(IDC_NTQIP, ntQueryInfoProc_)
		DDX_CHECK(IDC_GETTICKCOUNT, getTickCount_)
		DDX_CHECK(IDC_PROTECTDRS, protectDRs_)
		DDX_CHECK(IDC_GETVERSION, getVersion_)
		DDX_CHECK(IDC_NTCLOSE, ntClose_)
		DDX_CHECK(IDC_DBGPRESENT, dbgPresent_)
		DDX_CHECK(IDC_NTGF, ntGF_)
		DDX_CHECK(IDC_NTHF, ntHF_)
		DDX_INT(IDC_TICK_DELTA, tickDelta_)
		DDX_CHECK(IDC_DBGATTACH, dbgAttach_)
		DDX_CHECK(IDC_DBGSTART, dbgStart_)
	END_DDX_MAP()

	LRESULT OnDbgEnableClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	bool ntQueryObj_;
	bool rtlGetFlags_;
	bool ntQuerySysInfo_;
	bool ntQueryInfoProc_;
	bool getTickCount_;
	bool protectDRs_;
	bool getVersion_;
	bool ntClose_;
	bool dbgPresent_;
	bool ntGF_;
	bool ntHF_;
	int tickDelta_;
	bool dbgAttach_;
	bool dbgStart_;
	bool oldState_;
};
