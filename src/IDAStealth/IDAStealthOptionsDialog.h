#pragma once

#include "WTLCommon.h"
#include "IDAStealthPage1.h"
#include "IDAStealthPage2.h"
#include "IDAStealthDriversPage.h"
#include "IDAStealthAboutPage.h"
#include "IDAStealthPageMisc.h"

class CIDAStealthOptionsDialog : public CPropertySheetImpl<CIDAStealthOptionsDialog>
{
public:

	CIDAStealthOptionsDialog(LPCTSTR title) : CPropertySheetImpl(title)
	{
		page1_ = new CIDAStealthPage1(this);
		page2_ = new CIDAStealthPage2(this);
		page3_ = new CIDAStealthDriversPage(this);
		page4_ = new CIDAStealthPageMisc(this);
		AddPage(*page1_);
		AddPage(*page2_);
		AddPage(*page3_);
		AddPage(*page4_);
		AddPage(pageAbout_);
		m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	}

	~CIDAStealthOptionsDialog()
	{
		delete page1_;
		delete page2_;
		delete page3_;
		delete page4_;
	}

	void flushConfig()
	{
		// force each page to perform DDX and save its config
		page1_->saveConfig();
		page2_->saveConfig();
		page3_->saveConfig();
		page4_->saveConfig();
	}

	void loadNewProfile(const std::string& profileName)
	{
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.switchToProfile(profileName);
		page1_->loadConfig();
		page2_->loadConfig();
		page3_->loadConfig();
		page4_->loadConfig();
	}

	void updateGlobalEnableState(bool globalEnable)
	{
		page1_->setGlobalEnableState(globalEnable);
		page2_->setGlobalEnableState(globalEnable);
		page3_->setGlobalEnableState(globalEnable);
		page4_->setGlobalEnableState(globalEnable);
	}

private:

	BEGIN_MSG_MAP(CIDAStealthOptionsDialog)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		CHAIN_MSG_MAP(CPropertySheetImpl<CIDAStealthOptionsDialog>)
	END_MSG_MAP()

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam == TRUE) CenterWindow();
		bHandled = FALSE;
		return 0;
	}

	CIDAStealthPage1* page1_;
	CIDAStealthPage2* page2_;
	CIDAStealthDriversPage* page3_;
	CIDAStealthPageMisc* page4_;
	CIDAStealthAboutPage pageAbout_;
};