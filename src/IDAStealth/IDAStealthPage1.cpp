#include "IDAStealthPage1.h"
#include "IDAStealthOptionsDialog.h"

LRESULT CIDAStealthPage1::OnDbgEnableClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool newState = IsDlgButtonChecked(IDC_DBGSTART) || IsDlgButtonChecked(IDC_DBGATTACH);
	if (newState != oldState_)
	{
		parentDlg_->updateGlobalEnableState(newState);
	}
	oldState_ = newState;
	return 0;
}

void CIDAStealthPage1::OnDataExchangeError(UINT nCtrlID, BOOL /*bSave*/)
{
	if (nCtrlID == IDC_TICK_DELTA)
	{
		MessageBox("Please enter a valid tick count delta!", "IDAStealth", MB_ICONWARNING);
		::SetFocus(GetDlgItem(IDC_TICK_DELTA));
	}
}

void CIDAStealthPage1::updateGlobalEnableState(bool newState)
{
	BOOL state = newState ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_NTQO), state);
	::EnableWindow(GetDlgItem(IDC_RTLNTGF), state);
	::EnableWindow(GetDlgItem(IDC_NTQSI), state);
	::EnableWindow(GetDlgItem(IDC_NTQIP), state);
	::EnableWindow(GetDlgItem(IDC_GETTICKCOUNT), state);
	::EnableWindow(GetDlgItem(IDC_PROTECTDRS), state);
	::EnableWindow(GetDlgItem(IDC_GETVERSION), state);
	::EnableWindow(GetDlgItem(IDC_NTCLOSE), state);
	::EnableWindow(GetDlgItem(IDC_DBGPRESENT), state);
	::EnableWindow(GetDlgItem(IDC_NTGF), state);
	::EnableWindow(GetDlgItem(IDC_NTHF), state);
	::EnableWindow(GetDlgItem(IDC_TICK_DELTA), state);
}

void CIDAStealthPage1::loadPageConfig()
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	ntQueryObj_ = config.getNtQueryObject();
	rtlGetFlags_ = config.getRtlGetNtGlobalFlags();
	ntQuerySysInfo_ = config.getNtQuerySystemInfo();
	ntQueryInfoProc_ = config.getNtQueryInfoProcess();
	getTickCount_ = config.getGetTickCount();
	protectDRs_ = config.getProtectDRs();
	getVersion_ = config.getGetVersion();
	ntClose_ = config.getNtClose();
	dbgPresent_ = config.getPEBIsDebugged();
	ntGF_ = config.getNtGlobalFlag();
	ntHF_ = config.getHeapFlags();
	tickDelta_ = config.getGetTickCountDelta();
	dbgAttach_ = config.getEnableDbgAttach();
	dbgStart_ = config.getEnableDbgStart();
	oldState_ = dbgStart_ || dbgAttach_;
	DoDataExchange(FALSE);
	parentDlg_->updateGlobalEnableState(dbgStart_ || dbgAttach_);
}

bool CIDAStealthPage1::savePageConfig()
{
	if (DoDataExchange(TRUE))
	{
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.setNtQueryObject(ntQueryObj_);
		config.setRtlGetNtGlobalFlags(rtlGetFlags_);
		config.setNtQuerySystemInfo(ntQuerySysInfo_);
		config.setNtQueryInfoProcess(ntQueryInfoProc_);
		config.setGetTickCount(getTickCount_);
		config.setProtectDRs(protectDRs_);
		config.setGetVersion(getVersion_);
		config.setNtClose(ntClose_);
		config.setPEBIsDebugged(dbgPresent_);
		config.setNtGlobalFlag(ntGF_);
		config.setHeapFlags(ntHF_);
		config.setGetTickCountDelta(tickDelta_);
		config.setEnableDbgAttach(dbgAttach_);
		config.setEnableDbgStart(dbgStart_);
		return true;
	}
	else return false;
}