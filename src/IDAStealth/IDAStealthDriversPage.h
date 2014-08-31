#pragma once

#include <HideDebugger/HideDebuggerConfig.h>
#include "WTLCommon.h"
#include "IDAStealthPropertyPage.h"
#include <iostream>
#include "resource.h"

class CIDAStealthDriversPage : 
	public CIDAStealthPropertyPage<CIDAStealthDriversPage, IDD_DIALOG3>
{
public:

	CIDAStealthDriversPage(CIDAStealthOptionsDialog* parentDlg) :
		CIDAStealthPropertyPage(parentDlg) {}
	~CIDAStealthDriversPage() {}

protected:

	void updateGlobalEnableState(bool globalEnable);
	void loadPageConfig();
	bool savePageConfig();

private:

	typedef CIDAStealthPropertyPage<CIDAStealthDriversPage, IDD_DIALOG3> BasePropertyPage;

	BEGIN_MSG_MAP(CIDAStealthDriversPage)
		COMMAND_HANDLER(IDC_RDTSC, BN_CLICKED, OnRDTSCClick)
		CHAIN_MSG_MAP(CPropertyPageImpl<CIDAStealthDriversPage>)
		CHAIN_MSG_MAP(BasePropertyPage)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CIDAStealthDriversPage)
		DDX_CHECK(IDC_RDTSC, useRDTSC_)
		DDX_RADIO(IDC_RDTSC_ZERO, rdbIndex_)
		DDX_CHECK(IDC_UNLOAD_DRIVER, unloadRDTSCDrv_)
		DDX_CHECK(IDC_RAND_NAME, randomizeName_)
		DDX_INT(IDC_RDTSC_DELTA, rdtscDelta_)
		DDX_CHECK(IDC_UNLOAD_STEALTH_DRV, unloadStealthDrv_)
		DDX_CHECK(IDC_NTSIT_DRV, ntSITDrv_)
		DDX_CHECK(IDC_NTQIP_DRV, ntQIPDrv_)
	END_DDX_MAP()

	void updateRDBs(HWND hWndCheckBox);
	LRESULT OnRDTSCClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	bool useRDTSC_;
	bool unloadRDTSCDrv_;
	bool unloadStealthDrv_;
	bool ntSITDrv_;
	bool ntQIPDrv_;
	bool randomizeName_;
	int rdtscDelta_;
	int rdbIndex_;
};