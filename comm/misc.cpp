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

BOOL DeleteDir(const TCHAR* pszFolder)
{
    TCHAR szFindFileTemplate[MAX_PATH];
    _tcscpy(szFindFileTemplate, pszFolder);
    int nLen = _tcslen(szFindFileTemplate);
    if(nLen>0)
    {
        if(szFindFileTemplate[nLen-1]!=_T('\\'))
        {
            _tcscat(szFindFileTemplate, _T("\\"));
        }
    }
    _tcscat(szFindFileTemplate, _T("*"));
    
    WIN32_FIND_DATA FindData;
    HANDLE hFindFile = FindFirstFile(szFindFileTemplate, &FindData);
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        BOOL bMore = TRUE;
        while (bMore)
        {
            TCHAR szFileName[MAX_PATH];            
            if( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                _makepath(szFileName, NULL, pszFolder, FindData.cFileName, NULL);
                DeleteFile(szFileName);
            }
            bMore = FindNextFile(hFindFile, &FindData);
        }
        FindClose(hFindFile);
    }
    return RemoveDirectory(pszFolder);
}

// 获取文件的大小
// -1 文件不存在
// -2 文件存在，但无法读取大小
// 其它，实际文件的字节数
__int64 GetFileSizeByName(const char*  fileName)
{
    HANDLE hFile = CreateFile(fileName, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile  == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    
    LARGE_INTEGER fileSize={0};    
    if( !GetFileSizeEx(hFile, &fileSize))
    {
        CloseHandle(hFile);
        return -2;
    }
    CloseHandle(hFile);
    return (__int64)fileSize.QuadPart;
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



void ResizeImg( IplImage* pImgSrc, IplImage* pImgDst)
{
    // 读取图片的宽和高;
    //     int w = pImgSrc->width;
    //     int h = pImgSrc->height;    
    // 对图片 img 进行缩放，并存入到 SrcImage 中;
    cvResize( pImgSrc, pImgDst);

    // 重置 SrcImage 的 ROI 准备读入下一幅图片;
    cvResetImageROI( pImgDst );
}

void GetGrayPixel(IplImage* pImgPic, int nWidth, int nHeight, unsigned char* pOutPixel)
{    
    IplImage* pImgOper = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 3);
    IplImage* pImgGray = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
    ResizeImg(pImgPic, pImgOper);
    cvCvtColor(pImgOper, pImgGray, CV_RGB2GRAY);
    cvReleaseImage(&pImgOper);
    
    memcpy(pOutPixel, pImgGray->imageData, pImgGray->imageSize);
    cvReleaseImage(&pImgGray);
}

BOOL Ascii2Utf8(const char* pcszAscii, char* pszUtf8, int* pnUtf8Size)  
{  
    if (!pcszAscii || !pszUtf8 || !pnUtf8Size)  
        return FALSE;  
      
    int nWCharCount = MultiByteToWideChar(CP_ACP, 0, pcszAscii, -1, NULL, 0);  
    wchar_t* pwszStr = new wchar_t[nWCharCount];  
    MultiByteToWideChar(CP_ACP, 0, pcszAscii, -1, pwszStr, nWCharCount);  
          
    int nUtf8Count = WideCharToMultiByte(CP_UTF8, 0, pwszStr, nWCharCount, NULL, 0, NULL, NULL);  
    if (nUtf8Count > *pnUtf8Size)  
    {  
        *pnUtf8Size = nUtf8Count;  
        return FALSE;  
    }  
      
    WideCharToMultiByte(CP_UTF8, 0, pwszStr, nWCharCount, pszUtf8, nUtf8Count, NULL, NULL);  
    delete[] pwszStr;  
    *pnUtf8Size = nUtf8Count;  
    return TRUE;  
}  

BOOL Utf82Ascii(const char* pcszUtf8, char* pszAscii, int* pnAsciiSize)  
{  
    if (!pcszUtf8 || !pszAscii || !pnAsciiSize)  
        return FALSE;  
      
    int nWCharCount = MultiByteToWideChar(CP_UTF8, 0, pcszUtf8, -1, NULL, 0);  
    wchar_t* pwszStr = new wchar_t[nWCharCount];  
    MultiByteToWideChar(CP_UTF8, 0, pcszUtf8, -1, pwszStr, nWCharCount);  
          
    int nAsciiCount = WideCharToMultiByte(CP_ACP, 0, pwszStr, nWCharCount, NULL, 0, NULL, NULL);  
    if (nAsciiCount > *pnAsciiSize)  
    {  
        *pnAsciiSize = nAsciiCount;  
        return FALSE;  
    }  
      
    WideCharToMultiByte(CP_ACP, 0, pwszStr, nWCharCount, pszAscii, nAsciiCount, NULL, NULL);  
    delete[] pwszStr;  
    *pnAsciiSize = nAsciiCount;  
    return TRUE;  
}


