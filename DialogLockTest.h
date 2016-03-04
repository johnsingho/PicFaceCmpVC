#if !defined(AFX_DIALOGLOCKTEST_H__76249E1F_144A_48A3_83F1_FF1C716D9CB4__INCLUDED_)
#define AFX_DIALOGLOCKTEST_H__76249E1F_144A_48A3_83F1_FF1C716D9CB4__INCLUDED_

#include "comm/sw_lock03.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogLockTest.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogLockTest dialog

class CDialogLockTest : public CDialog
{
// Construction
public:
	CDialogLockTest(CWnd* pParent = NULL);   // standard constructor
    void SetLocker(SWLock03* pLocker){m_pLocker=pLocker;}
// Dialog Data
	//{{AFX_DATA(CDialogLockTest)
	enum { IDD = IDD_DIALOG_LOCKTEST };
	int		m_port;
    CEdit   m_editInfo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogLockTest)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogLockTest)
	afx_msg void OnButtonGateon();
	afx_msg void OnButtonGateoff();
	afx_msg void OnButtonLed1on();
	afx_msg void OnButtonLed1off();
	afx_msg void OnButtonLed2on();
	afx_msg void OnButtonLed2off();
	afx_msg void OnButtonLed3on();
	afx_msg void OnButtonLed3off();
	afx_msg void OnButtonLed4on();
	afx_msg void OnButtonLed4off();
	afx_msg void OnButtonLed5on();
	afx_msg void OnButtonLed5off();
	afx_msg void OnButton2();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    void PromptInfo(const char* pstr);

    SWLock03* m_pLocker;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGLOCKTEST_H__76249E1F_144A_48A3_83F1_FF1C716D9CB4__INCLUDED_)
