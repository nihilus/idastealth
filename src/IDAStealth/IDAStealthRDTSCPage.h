#pragma once

#include <HideDebugger/HideDebuggerConfig.h>
#include "WTLCommon.h"
#include "IDAStealthPropertyPage.h"
#include <iostream>
#include "resource.h"

class CIDAStealthRDTSCPage : 
	public CIDAStealthPropertyPage<CIDAStealthRDTSCPage, IDD_DIALOG3>
{
public:

	CIDAStealthRDTSCPage(CIDAStealthOptionsDialog* parentDlg) :
		CIDAStealthPropertyPage(parentDlg) {}
	~CIDAStealthRDTSCPage() {}

protected:

	void updateGlobalEnableState(bool globalEnable);
	void loadPageConfig();
	bool savePageConfig();

private:

	typedef CIDAStealthPropertyPage<CIDAStealthRDTSCPage, IDD_DIALOG3> BasePropertyPage;

	BEGIN_MSG_MAP(CIDAStealthRDTSCPage)
		COMMAND_HANDLER(IDC_RDTSC, BN_CLICKED, OnRDTSCClick)
		CHAIN_MSG_MAP(CPropertyPageImpl<CIDAStealthRDTSCPage>)
		CHAIN_MSG_MAP(BasePropertyPage)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CIDAStealthRDTSCPage)
		DDX_CHECK(IDC_RDTSC, useRDTSC_)
		DDX_RADIO(IDC_RDTSC_ZERO, rdbIndex_)
		DDX_CHECK(IDC_UNLOAD_DRIVER, unloadDriver_)
		DDX_CHECK(IDC_RAND_NAME, randomizeName_)
		DDX_INT(IDC_RDTSC_DELTA, rdtscDelta_)
	END_DDX_MAP()

	void updateRDBs(HWND hWndCheckBox);
	LRESULT OnRDTSCClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	bool useRDTSC_;
	bool unloadDriver_;
	bool randomizeName_;
	int rdtscDelta_;
	int rdbIndex_;
};