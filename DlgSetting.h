#if !defined(AFX_DLGSETTING_H__ACC9C854_29AA_4ED8_99F9_91542B2327E8__INCLUDED_)
#define AFX_DLGSETTING_H__ACC9C854_29AA_4ED8_99F9_91542B2327E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSetting.h : header file
//

#include "cameraDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgSetting dialog

class CDlgSetting : public CDialog
{
// Construction
public:
	CDlgSetting(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSetting)
	enum { IDD = IDD_DLG_SETTINGS };
	CEdit	m_edReqFaceScore;
	CEdit	m_edIDCom;
	CEdit	m_edCheckTickMax;
	CEdit	m_edCheckFaceMax;
	CEdit	m_edCamTick;
	CEdit	m_edCamFace;
	CEdit	m_edBoardCOM;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSetting)
	public:
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSetting)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    
    void ShowCfg( stConfigInfo* pCfgInfo );
    void StoreCfg();
public:
    void SetInfo( stConfigInfo* pCfgInfo ){m_pCfgInfo=pCfgInfo;}
private:
    stConfigInfo* m_pCfgInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETTING_H__ACC9C854_29AA_4ED8_99F9_91542B2327E8__INCLUDED_)
