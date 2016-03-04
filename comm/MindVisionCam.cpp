#include "StdAfx.h"
#include "MindVisionCam.h"

#include "Include/CameraApi.h"


#ifdef _WIN64
#pragma comment(lib, "comm/MVCAMSDK_X64.lib")
#else
#pragma comment(lib, "comm/MVCAMSDK.lib")
#endif

////////////////////////////////////////////////

static int gLanguage=1;


CMindVisionCamOper::CMindVisionCamOper()
    : m_hCamera(NULL)    
    , m_pFrame(NULL)
{    
}

    
CMindVisionCamOper::~CMindVisionCamOper()
{
    UnInitCamera();
}


bool CMindVisionCamOper::Init()
{
    return (CAMERA_STATUS_SUCCESS==CameraSdkInit(gLanguage));
}

void CMindVisionCamOper::UnInitCamera()
{
    if(m_hCamera)
    {
        CameraStop(m_hCamera);
        CameraUnInit(m_hCamera);
    }
    
    if(m_pFrame)
    {
        cvReleaseImage(&m_pFrame);
        m_pFrame=NULL;
    }
}


bool CMindVisionCamOper::InitCamera()
{
    const int MAX_CAMS = 2;
    tSdkCameraDevInfo sCameraList[MAX_CAMS];
    int iCameraNums=MAX_CAMS;

    CameraSdkStatus status;
    CRect rect;
    tSdkCameraCapbility sCameraInfo;
    memset(&sCameraInfo, 0, sizeof(sCameraInfo));
        
    if(CameraEnumerateDevice(sCameraList,&iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
    {
        return false;
    }
    
    //只假设连接了一个相机。
    if((status = CameraInit(&sCameraList[0],-1,-1,&m_hCamera)) != CAMERA_STATUS_SUCCESS)
    {
        CString msg;
        msg.Format("***Failed to init the camera! Error code is %d",status);
        OutputDebugString(msg);
        return false;
    }    
    
    //Get properties description for this camera.
    CameraGetCapability(m_hCamera,&sCameraInfo);    
    
   return true;
}

bool CMindVisionCamOper::Play()
{
    return CAMERA_STATUS_SUCCESS==CameraPlay(m_hCamera);
}

bool CMindVisionCamOper::Pause()
{
    return CAMERA_STATUS_SUCCESS==CameraPause(m_hCamera);
}


IplImage* CMindVisionCamOper::QueryFrame( int wTimes/*=1000*/ )
{
    int nWidth=0;
    int nHeight=0;
    BYTE* pPicBuf = CameraGetImageBufferEx(m_hCamera, &nWidth, &nHeight, wTimes);
    if(!pPicBuf){ return NULL;}
    
    if(nWidth!=m_curFrameSize.width || nHeight!=m_curFrameSize.height)
    {
        if(m_pFrame)
        {
            cvReleaseImage(&m_pFrame);
        }
        m_curFrameSize = cvSize(nWidth, nHeight);
	    m_pFrame = cvCreateImage(m_curFrameSize, IPL_DEPTH_8U, 3);
    }
    if(!m_pFrame)
    {
        return NULL;
    }
    memcpy(m_pFrame->imageData, pPicBuf, m_pFrame->imageSize);
    return m_pFrame;
}


