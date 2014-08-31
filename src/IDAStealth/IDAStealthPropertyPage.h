#pragma once

// common base class for all property pages

#include <iostream>
#include "WTLCommon.h"

class CIDAStealthOptionsDialog;

template <class baseT, int idd>
class CIDAStealthPropertyPage : 
	public CPropertyPageImpl<baseT>,
	public CWinDataExchange<baseT>
{
public:

	enum { IDD = idd };

	CIDAStealthPropertyPage(CIDAStealthOptionsDialog* parentDlg) :
		parentDlg_(parentDlg),
		globalEnable_(false),
		isInitialized_(false) {}
	virtual ~CIDAStealthPropertyPage() {}

	// only update this page if its currently active - otherwise controls might not have been created yet
	void setGlobalEnableState(bool globalEnable)
	{
		globalEnable_ = globalEnable;
		if (isInitialized_)
		{
			updateGlobalEnableState(globalEnable);
		}
	}

	// shield deriving class from errors if page has not yet been created
	// if page hasn't been activated, default values are loaded anyway,
	// so no need to save them explicitly
	void saveConfig()
	{
		if (isInitialized_) savePageConfig();
	}

	void loadConfig()
	{
		if (isInitialized_) loadPageConfig();
	}

	int OnSetActive()
	{
		updateGlobalEnableState(globalEnable_);
		return 0;
	}

	int OnApply()
	{
		if (savePageConfig()) return PSNRET_NOERROR;
		else return PSNRET_INVALID;
	}

protected:

	BOOL OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lParam*/)
	{
		isInitialized_ = true;
		loadPageConfig();
		return TRUE;
	}

	BEGIN_MSG_MAP(CIDAStealthPropertyPage)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CPropertyPageImpl<baseT>)
	END_MSG_MAP()

	// methods derived classes need to implement
	virtual void updateGlobalEnableState(bool newState) =0;
	virtual void loadPageConfig() =0;
	virtual bool savePageConfig() =0;

	CIDAStealthOptionsDialog* parentDlg_;

private:

	bool globalEnable_;
	bool isInitialized_;
};