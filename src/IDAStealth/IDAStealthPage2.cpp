#include "IDAStealthPage2.h"

void CIDAStealthPage2::loadPageConfig()
{
	HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	blockInput_ = config.getBlockInput();
	suspThread_ = config.getSuspendThread();
	ntTerminate_ = config.getNtTerminate();
	parentProcess_ = config.getFakeParentProcess();
	hideIDAProcess_ = config.getHideIDAProcess();
	hideIDAWnd_ = config.getHideIDAWindow();
	dbgPrintExcp_ = config.getDbgPrintException();
	openProcess_ = config.getOpenProcess();
	switch_ = config.getSwitchDesktop();
	antiAttach_ = config.getKillAntiAttach();
	ntYield_ = config.getNtYieldExecution();
	outputDbgStr_ = config.getOutputDbgStr();
	ntSetInfoThread_ = config.getNtSetInfoThread();
	DoDataExchange(FALSE);
}

bool CIDAStealthPage2::savePageConfig()
{
	if (DoDataExchange(TRUE))
	{
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.setBlockInput(blockInput_);
		config.setSuspendThread(suspThread_);
		config.setNtTerminate(ntTerminate_);
		config.setFakeParentProcess(parentProcess_);
		config.setHideIDAProcess(hideIDAProcess_);
		config.setHideIDAWindow(hideIDAWnd_);
		config.setDbgPrintException(dbgPrintExcp_);
		config.setOpenProcess(openProcess_);
		config.setSwitchDesktop(switch_);
		config.setKillAntiAttach(antiAttach_);
		config.setNtYieldExecution(ntYield_);
		config.setOutputDbgStr(outputDbgStr_);
		config.setNtSetInfoThread(ntSetInfoThread_);
		return true;
	}
	else return false;
}

void CIDAStealthPage2::updateGlobalEnableState(bool newState)
{
	BOOL state = newState ? TRUE : FALSE;
	::EnableWindow(GetDlgItem(IDC_BLOCKINPUT), state);
	::EnableWindow(GetDlgItem(IDC_SUSPENDTHREAD), state);
	::EnableWindow(GetDlgItem(IDC_TERMINATE), state);
	::EnableWindow(GetDlgItem(IDC_PARENTPROCESS), state);
	::EnableWindow(GetDlgItem(IDC_HIDEIDAPROCESS), state);
	::EnableWindow(GetDlgItem(IDC_HIDEIDAWND), state);
	::EnableWindow(GetDlgItem(IDC_DBGPRNTEXCEPT), state);
	::EnableWindow(GetDlgItem(IDC_OPENPROCESS), state);
	::EnableWindow(GetDlgItem(IDC_SWITCH), state);
	::EnableWindow(GetDlgItem(IDC_ANTIATTACH), state);
	::EnableWindow(GetDlgItem(IDC_NTYIELD), state);
	::EnableWindow(GetDlgItem(IDC_OUTDBGSTR), state);
	::EnableWindow(GetDlgItem(IDC_NTSIT), state);
}