#if !defined(AFX_HECOMPARE_H__963880E5_193C_4B6E_964F_A1EAF2483CF2__INCLUDED_)
#define AFX_HECOMPARE_H__963880E5_193C_4B6E_964F_A1EAF2483CF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Compare.h"
#include <afxmt.h>

// ���ͬʱ����������
#define MAX_DETECT_FACES (2)

class CFaceDetect
{
public:
    CFaceDetect();
    ~CFaceDetect();
    
    void StoreIDPhoto( IplImage* pImgPic );
    void StoreFacePhoto( IplImage* pFrame );
    void DrawFaceRect( IplImage* pFrame);        
    float CompareAFace( double fInitFaceCmpRate, int iPorttrail );
    CvvImage* GetCurLivePic(){return &m_livePic;}
    void LockLivePic();
    void UnLockLivePic();
private:    
    bool DetectLiveFace();
    void RestCurFaceInfo();
    
    PICPIXEL*      m_pIDPhoto; /*���֤��Ƭ����*/
    PICPIXEL*      m_pLivePic; /*�ֳ���Ƭ����*/
    CvvImage       m_livePic;  /*�ֳ���Ƭ*/
    CCriticalSection m_csLockLivePic; /*�����ֳ���Ƭ*/

    HWFaceInfo     m_liveFaceInfos[MAX_DETECT_FACES];
    HWFaceInfo     m_curLiveFace;
};






#endif // !defined(AFX_HECOMPARE_H__963880E5_193C_4B6E_964F_A1EAF2483CF2__INCLUDED_)
