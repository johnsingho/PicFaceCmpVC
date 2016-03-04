// IDStatic.cpp : implementation file
//

#include "stdafx.h"
#include "IDStatic.h"
#include <mbstring.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
#define ID_PHOTO_WID 102
#define ID_PHOTO_HEI 126

#define CLR_TITLE  RGB(54,159,216)
#define CLR_INFO   RGB(0,0,0)

#define CHR_MASK   '*'

/////////////////////////////////////////////////////////////////////////////
// CIDStatic

CIDStatic::CIDStatic()
    : m_pBmpMy(NULL)
    , m_pBmpIDPhoto(NULL)
    , m_pCurBg(NULL)
    , m_pIDDecoder(NULL)
{
    m_fontText.CreateFont(14,7,0,0,FW_NORMAL,
        FALSE,FALSE,0,GB2312_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"黑体");
}

CIDStatic::~CIDStatic()
{
}


BEGIN_MESSAGE_MAP(CIDStatic, CStatic)
    //{{AFX_MSG_MAP(CIDStatic)
        ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIDStatic message handlers

void CIDStatic::SetIDDraw( const IplImage *pImage, CIDBaseTextDecoder* pidDecoder)
{
    //CSingleLock lock(&m_cs);
    DelTempBmp();
    m_pBmpMy = IplImage2CBitmap(this, pImage);
    SetIDDraw(m_pBmpMy, pidDecoder);
}

void CIDStatic::SetIDDraw( CBitmap* pbmBack, CIDBaseTextDecoder* pidDecoder)
{
    CSingleLock lock(&m_cs, TRUE);
    m_pCurBg=pbmBack;
    m_pIDDecoder = pidDecoder;
}

void CIDStatic::PostNcDestroy() 
{
	DelTempBmp();
    CStatic::PostNcDestroy();
}

void CIDStatic::DelTempBmp()
{
    if(m_pBmpMy)
    {
        m_pBmpMy->DeleteObject();
        delete m_pBmpMy;
        m_pBmpMy=NULL;
    }
    if(m_pBmpIDPhoto)
    {
        m_pBmpIDPhoto->DeleteObject();
        delete m_pBmpIDPhoto;
        m_pBmpIDPhoto=NULL;
    }
}


void CIDStatic::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    // Do not call CStatic::OnPaint() for painting messages
 
    CSingleLock lock(&m_cs, TRUE);
    DrawBg(&dc);
    DrawIDCardInfo(&dc);
}

void CIDStatic::DrawBg( CDC* pDC )
{    
    if(!m_pCurBg || !m_pCurBg->m_hObject)
    {
        return;
    }
    
    CRect rectClient;
    GetClientRect(&rectClient);
    int x=0;
    int y=0;
    int cx=rectClient.Width();
    int cy=rectClient.Height();

    pDC->FillSolidRect(&rectClient, RGB(255,255,255)); //全填白色
    pDC->SetStretchBltMode(HALFTONE); 

    COLORREF crOldBack = pDC->GetBkColor();    
    CDC dcImage;
    CDC dcTrans;    
    
    // Create two memory DCs for the image and the mask
    dcImage.CreateCompatibleDC(pDC);
    dcTrans.CreateCompatibleDC(pDC);
    
    // Select the image into the appropriate dc
    CBitmap* pOldBitmapImage = (CBitmap*)dcImage.SelectObject(m_pCurBg);
    BITMAP bm;
    m_pCurBg->GetBitmap(&bm);
    int nBmWidth=bm.bmWidth;
    int nBmHeight=bm.bmHeight;
    
    // Create the mask bitmap
    CBitmap bitmapTrans;
    bitmapTrans.CreateBitmap(cx, cy, 1, 1, NULL);

    CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);
    COLORREF crColour = RGB(255,255,255);
    dcImage.SetBkColor(crColour);
    //dcTrans.BitBlt(x, y, nBmWidth, nBmHeight, &dcImage, 0, 0, SRCCOPY);
    dcTrans.StretchBlt(x, y, cx, cy, &dcImage, 0, 0, nBmWidth, nBmHeight, SRCCOPY);

    pDC->StretchBlt(x, y, cx, cy, &dcImage, 0, 0, nBmWidth, nBmHeight, SRCINVERT);
    pDC->StretchBlt(x, y, cx, cy, &dcTrans, 0, 0, nBmWidth, nBmHeight, SRCAND);
    pDC->StretchBlt(x, y, cx, cy, &dcImage, 0, 0, nBmWidth, nBmHeight, SRCINVERT);
    
    pDC->SetBkColor(crOldBack);
}

static void IncreRectArea(CRect& rect, int nW, int nH)
{
    rect.SetRect(rect.left, rect.top, rect.right+nW, rect.bottom+nH);
}

void CIDStatic::DrawIDCardInfo( CDC* pDC)
{
    if(!m_pIDDecoder){ return;}
    CONST int nYDel = 20;

    CRect rectClient;
    GetClientRect(&rectClient);
    CRect rectTitle(rectClient.left+10, rectClient.top+10, rectClient.Width()-ID_PHOTO_WID, 40);
    CRect rectInfo(rectTitle);
    rectInfo.OffsetRect(40, 0);

    pDC->SetBkMode(TRANSPARENT);
    CFont* pOldFont = pDC->SelectObject(&m_fontText);
    CString strText;
    CString strTemp;
    CString strShow;

    strText = "姓名：";
    ClrTextOut(pDC, strText, rectTitle, nYDel, CLR_TITLE);
    strShow = MaskName(m_pIDDecoder->GetName());
    ClrTextOut(pDC, strShow, rectInfo, nYDel, CLR_INFO);

    strText = "性别：";
    ClrTextOut(pDC, strText, rectTitle, nYDel, CLR_TITLE);
    ClrTextOut(pDC, m_pIDDecoder->GetSex(), rectInfo, nYDel, CLR_INFO);

    strText = "民族：";
    ClrTextOut(pDC, strText, rectTitle, nYDel, CLR_TITLE);
    ClrTextOut(pDC, m_pIDDecoder->GetEthnic(), rectInfo, nYDel, CLR_INFO);
    
    strText = "出生：";
    ClrTextOut(pDC, strText, rectTitle, nYDel, CLR_TITLE);
    CString strBir(m_pIDDecoder->GetBirthDayYYYYMMDD());
    //strTemp.Format("%s年%d月%d日", strBir.Mid(0,4), atoi(strBir.Mid(4,2)), atoi(strBir.Mid(6,2)));
    strTemp.Format("%s年**月**日", strBir.Mid(0,4));
    ClrTextOut(pDC, strTemp, rectInfo, nYDel, CLR_INFO);

    strText = "地址：";
    int nYDelAddr = nYDel*3;
    IncreRectArea(rectTitle, 0, nYDelAddr);
    ClrTextOut(pDC, strText, rectTitle, nYDelAddr, CLR_TITLE);
    IncreRectArea(rectInfo, -40, nYDelAddr);
    strShow = MaskAddress(m_pIDDecoder->GetAddress());
    ClrTextOut(pDC, strShow, rectInfo, nYDelAddr, CLR_INFO);

    rectInfo.OffsetRect(35, 0);

    strText = "身份证号：";    
    ClrTextOut(pDC, strText, rectTitle, nYDel, CLR_TITLE);
    strShow = MaskID(m_pIDDecoder->GetID());
    ClrTextOut(pDC, strShow, rectInfo, nYDel, CLR_INFO);

    strText = "有效期限：";
    ClrTextOut(pDC, strText, rectTitle, nYDel, CLR_TITLE);
    strTemp.Format("%s - %s", m_pIDDecoder->GetExpireBeginYYYYMMDD(), m_pIDDecoder->GetExpireEndYYYYMMDD());
    ClrTextOut(pDC, strTemp, rectInfo, nYDel, CLR_INFO);
    
    pDC->SelectObject(pOldFont);
    ShowPic(pDC, rectClient.right-ID_PHOTO_WID-5, rectClient.top+10);
}

void CIDStatic::SetIDPhoto( const IplImage* pImgPic)
{
    m_pBmpIDPhoto = IplImage2CBitmap(this, pImgPic);
}

void CIDStatic::ClrTextOut( CDC* pDC, CString strText, CRect &rectText, int nYOff, COLORREF clr/*=RGB(0,0,0)*/)
{
    COLORREF clrOld = pDC->GetTextColor();
    pDC->SetTextColor(clr);
    pDC->DrawText(strText, &rectText, DT_LEFT|DT_WORDBREAK);
    pDC->SetTextColor(clrOld);
    rectText.OffsetRect(0, nYOff);
}

void CIDStatic::ShowPic( CDC* pDC, int x, int y)
{
    CDC dcImage;    
    dcImage.CreateCompatibleDC(pDC);
    dcImage.SelectObject(m_pBmpIDPhoto);
    BITMAP bm;
    m_pBmpIDPhoto->GetBitmap(&bm);
    pDC->StretchBlt(x, y, ID_PHOTO_WID, ID_PHOTO_HEI, &dcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
}

CString CIDStatic::MaskName( const char* pstrName)
{
    CString strTemp = ClearTailSpace(pstrName);
    int nHan = strTemp.GetLength()/2;
    CString strRet;
    strRet = strTemp.Mid(0,2);
    if(nHan==2)
    {
        strRet += CHR_MASK;
    }else{
        for(int i=1; i<=nHan-1; i++)
        {
            if(i==nHan-1)
            {
                strRet += strTemp.Mid(i*2,2);
                break;
            }
            strRet += CHR_MASK;
        }
    }    
    
    return strRet;
}

CString CIDStatic::MaskAddress( const char* pstrAddr)
{
    CString strAddr = ClearTailSpace(pstrAddr);
    const BYTE* pAddr = (const BYTE*)(const char*)strAddr;
    int nLen = strAddr.GetLength()+1;
    char* pstrBuf = new char[nLen];
    memset(pstrBuf, 0, nLen);
    int nWords = _mbslen(pAddr);
    int nGet = 0;
    if(nWords<10)
    {
        nGet = nWords/2;
    }else{
        nGet = nWords/3; //只显示地址前面三分之一
    }
    
    int nStart = nWords-nGet;
    _mbsncpy((BYTE*)pstrBuf, pAddr, nGet);
    int nEnd = _mbslen((const BYTE*)pstrBuf);
    char strMask[2];
    strMask[0]=CHR_MASK;
    strMask[1]=NULL;
    for(int i=0; i<nStart; i++)
    {
        _mbscat((BYTE*)pstrBuf, (const BYTE*)strMask);
    }
    CString strRet(pstrBuf);
    delete[] pstrBuf;
    return strRet;
}

CString CIDStatic::MaskID( const char* pstrID)
{
    CString strID = ClearTailSpace(pstrID);
    //strID.SetAt(3, CHR_MASK);
    const int nLen = strlen(pstrID);
    for(int i=0; i<4; i++)
    {
        strID.SetAt(i+10, CHR_MASK);
    }
    return strID;
}

CString CIDStatic::ClearTailSpace( const char* pstr)
{
    CString strTemp(pstr);
    int iEmp = strTemp.Find("  ");
    if(iEmp>0)
    {
        strTemp = strTemp.Mid(0, iEmp);
    }
    return strTemp;
}
