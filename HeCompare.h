#if !defined(AFX_HECOMPARE_H__963880E5_193C_4B6E_964F_A1EAF2483CF2__INCLUDED_)
#define AFX_HECOMPARE_H__963880E5_193C_4B6E_964F_A1EAF2483CF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Compare.h"
#include <afxmt.h>

// 最多同时检查的人脸数
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
    
    PICPIXEL*      m_pIDPhoto; /*身份证照片数据*/
    PICPIXEL*      m_pLivePic; /*现场照片数据*/
    CvvImage       m_livePic;  /*现场照片*/
    CCriticalSection m_csLockLivePic; /*锁定现场照片*/

    HWFaceInfo     m_liveFaceInfos[MAX_DETECT_FACES];
    HWFaceInfo     m_curLiveFace;
};






#endif // !defined(AFX_HECOMPARE_H__963880E5_193C_4B6E_964F_A1EAF2483CF2__INCLUDED_)
