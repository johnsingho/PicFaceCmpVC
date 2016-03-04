// MyStatic.cpp : implementation file
//

#include "stdafx.h"
#include "camera.h"
#include "MyStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyStatic

CMyStatic::CMyStatic()
{
    m_clrText=RGB(20, 100, 20);
    LOGFONT logFont;
    ZeroMemory(&logFont, sizeof(logFont));
    ::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT),sizeof(logFont),&logFont);    
    m_font.CreateFontIndirect(&logFont);
}

CMyStatic::~CMyStatic()
{
}


BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
	//{{AFX_MSG_MAP(CMyStatic)
    ON_MESSAGE(WM_SETTEXT,OnSetText)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyStatic message handlers

LRESULT CMyStatic::OnSetText( WPARAM,LPARAM )
{
    LRESULT Result = Default();
    CRect Rect;
    GetWindowRect(&Rect);
    GetParent()->ScreenToClient(&Rect);
    GetParent()->InvalidateRect(&Rect);
    GetParent()->UpdateWindow();
	return Result;
}

HBRUSH CMyStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
//     CFont* pFont = this->GetFont();
//     CFont* pOldFont = pDC->SelectObject(pFont);
    pDC->SelectObject(&m_font);
	pDC->SetTextColor(GetTextClr());      //设置文字的颜色,颜色在Dlg.cpp里面设置  
    pDC->SetBkMode(TRANSPARENT);          //透明      
    return (HBRUSH)::GetStockObject(NULL_BRUSH);  
	// TODO: Return a non-NULL brush if the parent's handler should not be called
//	return NULL;
}

// void CMyStatic::MySetWindowText(LPCTSTR lpszString)
// {
// 	this->ShowWindow(SW_HIDE);
// 	this->SetWindowText(lpszString);
// 	this->ShowWindow(SW_SHOW);
// }

void CMyStatic::SetTextClr(COLORREF clrText)
{
    m_clrText = clrText;
}

COLORREF CMyStatic::GetTextClr()
{
    return m_clrText;
}

void CMyStatic::MySetFont( CFont* pFont )
{
    LOGFONT logFont;
    pFont->GetLogFont(&logFont);
    m_font.DeleteObject();
    m_font.CreateFontIndirect(&logFont);
    RedrawWindow();
}
