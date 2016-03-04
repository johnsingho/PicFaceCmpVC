#if !defined(AFX_MYSTATIC_H__77C2579E_F27B_4378_BA1A_74836898052A__INCLUDED_)
#define AFX_MYSTATIC_H__77C2579E_F27B_4378_BA1A_74836898052A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyStatic window

// 用来显示带颜色文本
// 可以设置字体
//
class CMyStatic : public CStatic
{
// Construction
public:
	CMyStatic();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyStatic();
	//void MySetWindowText(LPCTSTR lpszString);
    COLORREF GetTextClr();
    void SetTextClr(COLORREF clrText);
    void MySetFont( CFont* pFont);
	// Generated message map functions
protected:
	//{{AFX_MSG(CMyStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg LRESULT OnSetText(WPARAM,LPARAM);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()        
    
    COLORREF m_clrText;
    CFont    m_font;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSTATIC_H__77C2579E_F27B_4378_BA1A_74836898052A__INCLUDED_)
