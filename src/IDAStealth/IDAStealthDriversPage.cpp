#include "IDAStealthDriversPage.h"
#include "IDACommon.h"

void CIDAStealthDriversPage::updateGlobalEnableState(bool newState)
{
	BOOL state = newState ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_RDTSC), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_ZERO), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_INCREASING), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_DELTA), state);
	::EnableWindow(GetDlgItem(IDC_UNLOAD_DRIVER), state);
	::EnableWindow(GetDlgItem(IDC_RAND_NAME), state);
	::EnableWindow(GetDlgItem(IDC_NTSIT_DRV), state);
	::EnableWindow(GetDlgItem(IDC_UNLOAD_STEALTH_DRV), state);
	::EnableWindow(GetDlgItem(IDC_NTQIP_DRV), state);

	// handle radio buttons for RDTSC driver here - EnableWindow has no effect
	// if invoked from WM_INITDIALOG (as is the case in loadPageConfig())
	updateRDBs(GetDlgItem(IDC_RDTSC));
}

void CIDAStealthDriversPage::updateRDBs(HWND hWndCheckBox)
{
	CButton chkRDTSC = hWndCheckBox;
	CButton rdbZero = GetDlgItem(IDC_RDTSC_ZERO);
	CButton rdbIncr = GetDlgItem(IDC_RDTSC_INCREASING);
	BOOL checked = (chkRDTSC.GetCheck()) ? TRUE : FALSE;
	rdbZero.EnableWindow(checked);
	rdbIncr.EnableWindow(checked);
}

LRESULT CIDAStealthDriversPage::OnRDTSCClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	updateRDBs(hWndCtl);
	return 0;
}

void CIDAStealthDriversPage::loadPageConfig()
{
	HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	useRDTSC_ = config.getUseRDTSC();
	rdbIndex_ = (config.getRDTSCMode() == constant) ? 0 : 1;
	unloadRDTSCDrv_ = config.getUnloadRDTSC();
	randomizeName_ = config.getRandRDTSCName();
	rdtscDelta_ = config.getRDTSCIncrDelta();
	ntSITDrv_ = config.getStealthNtSetInfoThread();
	unloadStealthDrv_ = config.getUnloadStealth();
	ntQIPDrv_ = config.getStealthNtQueryInfoProcess();
	DoDataExchange(FALSE);
}

bool CIDAStealthDriversPage::savePageConfig()
{
	if (DoDataExchange(TRUE))
	{
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.setUseRDTSC(useRDTSC_);
		if (rdbIndex_) config.setRDTSCMode(increasing);
		else config.setRDTSCMode(constant);
		config.setUnloadRDTSC(unloadRDTSCDrv_);
		config.setRandRDTSCName(randomizeName_);
		config.setRDTSCIncrDelta(rdtscDelta_);
		config.setStealthNtSetInfoThread(ntSITDrv_);
		config.setUnloadStealth(unloadStealthDrv_);
		config.setStealthNtQueryInfoProcess(ntQIPDrv_);
		bool useStealthDriver = ntSITDrv_ || ntQIPDrv_;
		config.setUseStealthDriver(useStealthDriver);
		return true;
	}
	else return false;
}