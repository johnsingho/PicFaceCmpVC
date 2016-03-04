// HeCompare.cpp: implementation of the CHeCompare class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HeCompare.h"
#include "comm/misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////
extern tagHWInitialD             pfnHWInitialD;
extern tagHWReleaseD             pfnHWReleaseD;
extern tagHWInitial              pfnHWInitial;
extern tagHWRelease              pfnHWRelease;
extern tagHWDetectFaceKeyPoints  pfnHWDetectFaceKeyPoints;
extern tagHWGetFeatureSize       pfnHWGetFeatureSize;
extern tagHWExtractFeature       pfnHWExtractFeature;
extern tagHWCompareFeature       pfnHWCompareFeature;
extern tagHWSetPortrait          pfnHWSetPortrait;

extern HMODULE     g_hLib;
extern HW_HANDLE   g_handle;

//////////////////////////////////////////////////


CFaceDetect::CFaceDetect()
{
    m_pIDPhoto = CreatePicPixel(PHOTO_WIDTH, PHOTO_HEIGHT);
    m_pLivePic = CreatePicPixel(IMAGE_WIDTH, IMAGE_HEIGHT);
    RestCurFaceInfo();    
}

CFaceDetect::~CFaceDetect()
{
    DeletePicPixel(m_pIDPhoto);
    m_pIDPhoto=NULL;
    DeletePicPixel(m_pLivePic);
    m_pLivePic=NULL;
}

void CFaceDetect::StoreIDPhoto( IplImage* pImgPic)
{
    GetGrayPixel(pImgPic, m_pIDPhoto->width, m_pIDPhoto->height, &m_pIDPhoto->pixel[0]);
}

void CFaceDetect::StoreFacePhoto( IplImage* pFrame)
{
    m_livePic.CopyOf(pFrame);
    GetGrayPixel(m_livePic.GetImage(), m_pLivePic->width, m_pLivePic->height, &m_pLivePic->pixel[0]);
}

void CFaceDetect::DrawFaceRect( IplImage* pFrame )
{    
    if(!DetectLiveFace())
    {
        return;
    }

    float fWS = (float)pFrame->width /m_pLivePic->width;
    float fHS = (float)pFrame->height/m_pLivePic->height;

    CvScalar rectColor = cvScalar(0,0,255);

    CvPoint pt1;
    pt1.x = m_curLiveFace.m_FaceRect.left*fWS;
    pt1.y = m_curLiveFace.m_FaceRect.top*fHS;
    CvPoint pt2;
    pt2.x = m_curLiveFace.m_FaceRect.right*fWS;
    pt2.y = m_curLiveFace.m_FaceRect.bottom*fHS;
    cvRectangle(pFrame, pt1, pt2, rectColor, 2, 8, 0);
}


bool CFaceDetect::DetectLiveFace()
{
    RestCurFaceInfo();
    int iMaxFace = MAX_DETECT_FACES;

    HWRESULT iRst=S_FAIL;   
    iRst = pfnHWDetectFaceKeyPoints( g_handle, m_pLivePic->pixel, m_pLivePic->width, m_pLivePic->height, &iMaxFace, m_liveFaceInfos);
    bool bValid = S_OK==iRst;
    if(bValid)
    {
        if(iMaxFace>1)
        {
            TRACE1("***CFaceDetect::DetectLiveFace(), Faces=%d\n", iMaxFace);
        }        
        for(int i=0; i<iMaxFace; i++)    
        {
            const HWFaceInfo& face = m_liveFaceInfos[i];
            int nMaxH = m_curLiveFace.m_FaceRect.bottom - m_curLiveFace.m_FaceRect.top;       
            int nMaxW = m_curLiveFace.m_FaceRect.right - m_curLiveFace.m_FaceRect.left;
            int nCurH = face.m_FaceRect.bottom - face.m_FaceRect.top;       
            int nCurW = face.m_FaceRect.right - face.m_FaceRect.left;
            if(nCurW*nCurH > nMaxW*nMaxH)
            {
                m_curLiveFace=face;
            }
        }
    }
    return bValid;
}

void CFaceDetect::RestCurFaceInfo()
{
    memset(&m_liveFaceInfos, 0, sizeof(m_liveFaceInfos));
    memset(&m_curLiveFace, 0, sizeof(m_curLiveFace));
    //cvZero(&m_livePic);
}

// 进行一对一对比
//
float CFaceDetect::CompareAFace( double fInitFaceCmpRate, int iPorttrail )
{
    //return TestCompare1V1(g_handle, m_pIDPhoto, m_pLivePic, fInitFaceCmpRate, iPorttrail);    
    unsigned char *pbFtrID = NULL, 
    unsigned char *pbFtrLiveFace = NULL;    
    float fScore=fInitFaceCmpRate;

    int iFtrSize=0;
    pfnHWGetFeatureSize( g_handle, &iFtrSize);
    pbFtrID       = new unsigned char[ iFtrSize];
    pbFtrLiveFace = new unsigned char[ iFtrSize];

    //如果确定是证件照，可以设Portrait= 1， 否则设Portrait = 0
    pfnHWSetPortrait( g_handle, iPorttrail);

    int iMaxFace=1;
    HWFaceInfo idFaceInfo;
    //找身份证上的人脸
    int iRst = pfnHWDetectFaceKeyPoints( g_handle, m_pIDPhoto->pixel, m_pIDPhoto->width, m_pIDPhoto->height, &iMaxFace, &idFaceInfo);
    if( iRst != S_OK)
    {
        delete[] pbFtrID;
        delete[] pbFtrLiveFace;
        return 0.0;
    }
    if( S_OK != pfnHWExtractFeature( g_handle, m_pIDPhoto->pixel, m_pIDPhoto->width, m_pIDPhoto->height, &idFaceInfo, pbFtrID))
    {
        delete[] pbFtrID;
        delete[] pbFtrLiveFace;
        return 0.0;
    }

    //找出现场照片上的人脸
    if( S_OK != pfnHWExtractFeature( g_handle, m_pLivePic->pixel, m_pLivePic->width, m_pLivePic->height, &m_curLiveFace, pbFtrLiveFace))
    {
        delete[] pbFtrID;
        delete[] pbFtrLiveFace;
        return 0.0;
    }

    OutputDebugString("***HWExtractFeature all ok\n");
    pfnHWCompareFeature( g_handle, pbFtrID, pbFtrLiveFace, &fScore);

    delete[] pbFtrID;
    delete[] pbFtrLiveFace;
    return fScore;
}


