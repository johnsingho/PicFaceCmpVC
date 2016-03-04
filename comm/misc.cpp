#include "StdAfx.h"
#include "misc.h"
#include <AFXMT.H>


///////////////////////////////////////////////////////////////////


CString MakeModuleFileName( const TCHAR* pstrFileName, HMODULE hModule/*=NULL*/ )
{
    const int nBufLen = 1024;
    TCHAR strBuf[nBufLen]={0};
    DWORD dwLen = GetModuleFileName( hModule, strBuf, nBufLen);
    
    char* pstrSlash = strrchr(strBuf, '\\');
    if( pstrSlash)
    {
        ++pstrSlash;
        *pstrSlash=NULL;
    }
    CString strPath( strBuf);
    strPath += pstrFileName;
    return strPath;
}


void logfile(const char* pstrFileName, const char* data)
{
    size_t pos;
    FILE *pf;
    CTime timTemp;
    CString strTime;    
    static CCriticalSection s_logCS;

    timTemp = CTime::GetCurrentTime();
    strTime = timTemp.Format("%Y-%m-%d %H:%M:%S->");
    
    CSingleLock lock(&s_logCS, TRUE);
    pf = fopen(pstrFileName,"a+");
    if(!pf)
    {
        return;
    }
    fprintf(pf,"%s%s\n",strTime, data);
    
    //fflush(pf);
    pos = ftell(pf);
    if (pos>0)
    {
        fflush(pf);        
    }
    fclose(pf);
}


static void FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin)
{
    ASSERT( bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));
    
    BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);
    
    memset( bmih, 0, sizeof(*bmih));
    bmih->biSize = sizeof(BITMAPINFOHEADER);
    bmih->biWidth = width;
    bmih->biHeight = origin ? abs(height) : -abs(height);
    bmih->biPlanes = 1;
    bmih->biBitCount = (unsigned short)bpp;
    bmih->biCompression = BI_RGB;
    
    if( bpp == 8 )
    {
        RGBQUAD* palette = bmi->bmiColors;
        int i;
        for( i = 0; i < 256; i++ )
        {
            palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
            palette[i].rgbReserved = 0;
        }
    }
}


CBitmap* IplImage2CBitmap(CWnd* pwnd, const IplImage *pImage)
{
    if( pImage && pImage->depth == IPL_DEPTH_8U )
    {
        CDC* pDC = pwnd->GetDC();
        uchar buffer[sizeof(BITMAPINFO) + 1024];
        BITMAPINFO* bmi = (BITMAPINFO*)buffer;
        int bmp_w = pImage->width, bmp_h = pImage->height;
        FillBitmapInfo( bmi, bmp_w, bmp_h, pImage->depth*pImage->nChannels, pImage->origin);
        
        char *pBits=NULL;
        HBITMAP hBitmap=CreateDIBSection(pDC->GetSafeHdc(),bmi,DIB_RGB_COLORS,(void**)&pBits,NULL,0);
        memcpy(pBits,pImage->imageData,pImage->imageSize);
        CBitmap *pBitmap=new CBitmap;
        pBitmap->Attach(hBitmap);
        pwnd->ReleaseDC(pDC);
        return pBitmap;
    }
    else
        return NULL;
}


