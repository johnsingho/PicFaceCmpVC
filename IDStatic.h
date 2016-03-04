#if !defined(AFX_IDSTATIC_H__80CD3255_D827_411A_AEAE_10AE0CE68A86__INCLUDED_)
#define AFX_IDSTATIC_H__80CD3255_D827_411A_AEAE_10AE0CE68A86__INCLUDED_

#include "comm/IdReadHelper.h"
#include "comm/misc.h"
#include <AFXMT.H>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IDStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIDStatic window

// 用来画身份证
//
class CIDStatic : public CStatic
{
// Construction
public:
	CIDStatic();

// Attributes
public:

// Operations
public:
    void SetIDDraw( const IplImage *pImage, CIDBaseTextDecoder* pidCardReader);
    void SetIDDraw( CBitmap* pbmBack, CIDBaseTextDecoder* pidCardReader );    
    void SetIDPhoto( const IplImage* pImgPic);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDStatic)
	protected:
	virtual void PostNcDestroy();    
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIDStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIDStatic)
		afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    void DelTempBmp();    
    void DrawBg( CDC* pDC );
    void DrawIDCardInfo( CDC* pDC);
    
    void ClrTextOut( CDC* pDC, CString strText, CRect &rectText, int nYOff, COLORREF clr=RGB(0,0,0));
    void ShowPic( CDC* pDC, int x, int y);
    CString MaskName( const char* pstrName );
    
    CString ClearTailSpace( const char* pstrName );

    CString MaskAddress( const char* pstrAddr );
    CString MaskID( const char* pstrID );
    
    CCriticalSection m_cs;

    CFont     m_fontText;
    CBitmap*  m_pBmpMy; //自己拥有的bitmap    
    CBitmap*  m_pBmpIDPhoto;
    CBitmap* m_pCurBg;
    CIDBaseTextDecoder* m_pIDDecoder;
    
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IDSTATIC_H__80CD3255_D827_411A_AEAE_10AE0CE68A86__INCLUDED_)
