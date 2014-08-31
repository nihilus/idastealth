#pragma once

#include "IDAStealthPropertyPage.h"
#include <iostream>
#include "WTLCommon.h"
#include "resource.h"

class CIDAStealthOptionsDialog;

class CIDAStealthPageMisc : 
	public CIDAStealthPropertyPage<CIDAStealthPageMisc, IDD_DIALOG4>
{
public:

	CIDAStealthPageMisc(CIDAStealthOptionsDialog* parentDlg) :
		CIDAStealthPropertyPage<CIDAStealthPageMisc, IDD_DIALOG4>(parentDlg) {}
	~CIDAStealthPageMisc() {}

protected:

	void updateGlobalEnableState(bool /*newState*/);
	void loadPageConfig();
	bool savePageConfig();

private:

	LRESULT OnAddProfileClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelProfileClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnProfilesSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void switchProfile();
	void initComboBox();

	typedef CIDAStealthPropertyPage<CIDAStealthPageMisc, IDD_DIALOG4> BasePropertyPage;
	
	BEGIN_MSG_MAP(CIDAStealthPageMisc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_ADD_PROFILE, BN_CLICKED, OnAddProfileClick)
		COMMAND_HANDLER(IDC_DEL_PROFILE, BN_CLICKED, OnDelProfileClick)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnProfilesSelChange)
		CHAIN_MSG_MAP(CPropertyPageImpl<CIDAStealthPageMisc>)
		CHAIN_MSG_MAP(BasePropertyPage)
	END_MSG_MAP()

	CComboBox cboProfiles_;
	int rdbPatchingMethod_;
	bool passExceptions_;
	int tcpPort_;
	bool haltInSEH_;
	bool haltAfterSEH_;
	bool logSEH_;

	BEGIN_DDX_MAP(CIDAStealthPageMisc)
		DDX_CONTROL_HANDLE(IDC_PROFILES, cboProfiles_)
		DDX_RADIO(IDC_AUTO_SELECTION, rdbPatchingMethod_)
		DDX_CHECK(IDC_PASS_EXCEPTIONS, passExceptions_)
		DDX_CHECK(IDC_HALT_IN_SEH, haltInSEH_)
		DDX_CHECK(IDC_HALT_AFTER_SEH, haltAfterSEH_)
		DDX_CHECK(IDC_LOG_SEH, logSEH_)
		DDX_INT(IDC_TCP_PORT, tcpPort_)
	END_DDX_MAP()
};