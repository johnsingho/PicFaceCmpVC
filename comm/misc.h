#ifndef _he_2015_misc_h__
#define _he_2015_misc_h__

#include <AFX.H>
#include <AFXWIN.H>
#include <cv.h>

CString MakeModuleFileName( const TCHAR* pstrFileName, HMODULE hModule=NULL);

inline CString MakeModuleFilePath(const char* pstrFile)
{
    return MakeModuleFileName(pstrFile, GetModuleHandle(NULL));
}


void logfile(const char* pstrFileName, const char* data);
CBitmap* IplImage2CBitmap(CWnd* pwnd, const IplImage *pImage);




#endif // _he_2015_misc_h__