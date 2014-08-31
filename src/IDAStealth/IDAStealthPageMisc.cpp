#include <boost/foreach.hpp>
#include "IDAStealthOptionsDialog.h"
#include "IDAStealthPageMisc.h"
#include <HideDebugger/HideDebuggerConfig.h>
#include "WTLInputBox.h"

LRESULT CIDAStealthPageMisc::OnAddProfileClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWTLInputBox inputBox;
	inputBox.DoModal(parentDlg_->m_hWnd);
	std::string input = inputBox.getInput();
	if (input.length())
	{
		int item = cboProfiles_.AddString(input.c_str());
		cboProfiles_.SetCurSel(item);
		parentDlg_->flushConfig();
		
		// switch profile and save current dialogs to the new profile - leave dialog settings intact
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.switchToProfile(input);
		parentDlg_->flushConfig();
	}
	return 0;
}

LRESULT CIDAStealthPageMisc::OnDelProfileClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int itemID = cboProfiles_.GetCurSel();
	if (itemID != CB_ERR)
	{
		WTL::CString itemText;
		cboProfiles_.GetLBText(itemID, itemText);
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		if (config.delProfile((const char*)itemText))
		{
			cboProfiles_.DeleteString(itemID);
			itemID = itemID > 0 ? --itemID : 0;
			cboProfiles_.SetCurSel(itemID);
			switchProfile();
		}		
	}
	return 0;
}

LRESULT CIDAStealthPageMisc::OnProfilesSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	parentDlg_->flushConfig();
	switchProfile();
	return 0;
}

void CIDAStealthPageMisc::updateGlobalEnableState(bool newState)
{
	BOOL state = newState ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_PASS_EXCEPTIONS), state);
	::EnableWindow(GetDlgItem(IDC_AUTO_SELECTION), state);
	::EnableWindow(GetDlgItem(IDC_FORCE_ABS), state);
	::EnableWindow(GetDlgItem(IDC_TCP_PORT), state);
	::EnableWindow(GetDlgItem(IDC_HALT_IN_SEH), state);
	::EnableWindow(GetDlgItem(IDC_HALT_AFTER_SEH), state);
	::EnableWindow(GetDlgItem(IDC_LOG_SEH), state);
}

void CIDAStealthPageMisc::loadPageConfig()
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	rdbPatchingMethod_ = config.getInlinePatching();
	passExceptions_ = config.getPassExceptions();
	tcpPort_ = config.getRemoteTCPPort();
	haltInSEH_ = config.getHaltInSEH();
	haltAfterSEH_ = config.getHaltAfterSEH();
	logSEH_ = config.getLogSEH();
	DoDataExchange(FALSE);
}

bool CIDAStealthPageMisc::savePageConfig()
{
	if (DoDataExchange(TRUE))
	{
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.setPassExceptions(passExceptions_);
		config.setInlinePatching((InlinePatching)rdbPatchingMethod_);
		config.setRemoteTCPPort(tcpPort_);
		config.setHaltInSEH(haltInSEH_);
		config.setHaltAfterSEH(haltAfterSEH_);
		config.setLogSEH(logSEH_);
		return true;
	}
	else return false;
}

// needed to bind and populate CComboBox
BOOL CIDAStealthPageMisc::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	DoDataExchange(FALSE);
	initComboBox();
	bHandled = FALSE;
	return TRUE;
}

// get currently selected profile and propagate new profile
void CIDAStealthPageMisc::switchProfile()
{
	int itemID = cboProfiles_.GetCurSel();
	if (itemID != CB_ERR)
	{		
		WTL::CString itemText;
		cboProfiles_.GetLBText(itemID, itemText);
		std::string profile = (const char*)itemText;
		parentDlg_->loadNewProfile(profile);
	}
}

void CIDAStealthPageMisc::initComboBox()
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	BOOST_FOREACH(const std::string& profile, config.getProfiles())
	{
		int itemID = cboProfiles_.AddString(profile.c_str());
		if (profile == config.getCurrentProfile()) cboProfiles_.SetCurSel(itemID);
	}
}