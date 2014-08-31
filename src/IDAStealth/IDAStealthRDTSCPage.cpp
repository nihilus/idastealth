#include "IDAStealthRDTSCPage.h"

void CIDAStealthRDTSCPage::updateGlobalEnableState(bool newState)
{
	BOOL state = newState ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_RDTSC), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_ZERO), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_INCREASING), state);
	::EnableWindow(GetDlgItem(IDC_RDTSC_DELTA), state);
	::EnableWindow(GetDlgItem(IDC_UNLOAD_DRIVER), state);
	::EnableWindow(GetDlgItem(IDC_RAND_NAME), state);
}

void CIDAStealthRDTSCPage::updateRDBs(HWND hWndCheckBox)
{
	CButton chkRDTSC = hWndCheckBox;
	CButton rdbZero = GetDlgItem(IDC_RDTSC_ZERO);
	CButton rdbIncr = GetDlgItem(IDC_RDTSC_INCREASING);
	BOOL checked = (chkRDTSC.GetCheck()) ? TRUE : FALSE;
	rdbZero.EnableWindow(checked);
	rdbIncr.EnableWindow(checked);
}

LRESULT CIDAStealthRDTSCPage::OnRDTSCClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	updateRDBs(hWndCtl);
	return 0;
}

void CIDAStealthRDTSCPage::loadPageConfig()
{
	HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	useRDTSC_ = config.getUseRDTSC();
	rdbIndex_ = (config.getRDTSCMode() == constant) ? 0 : 1;
	unloadDriver_ = config.getUnloadRDTSC();
	randomizeName_ = config.getRandRDTSCName();
	rdtscDelta_ = config.getRDTSCIncrDelta();
	DoDataExchange(FALSE);
	updateRDBs(GetDlgItem(IDC_RDTSC));
}

bool CIDAStealthRDTSCPage::savePageConfig()
{
	if (DoDataExchange(TRUE))
	{
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.setUseRDTSC(useRDTSC_);
		if (rdbIndex_) config.setRDTSCMode(increasing);
		else config.setRDTSCMode(constant);
		config.setUnloadRDTSC(unloadDriver_);
		config.setRandRDTSCName(randomizeName_);
		config.setRDTSCIncrDelta(rdtscDelta_);
		return true;
	}
	else return false;
}