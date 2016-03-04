#ifndef MindVisionCam_h_2015_
#define MindVisionCam_h_2015_

#include <cv.h>

#include "CameraApi.h"


class CMindVisionCamOper
{
public:
    CMindVisionCamOper();
    ~CMindVisionCamOper();

    bool Init();
    bool InitCamera();
    void UnInitCamera();

    bool Play();
    bool Pause();
    IplImage* QueryFrame(int wTimes=1000);
    
private:
    CameraHandle m_hCamera;
    IplImage* m_pFrame;
    CvSize m_curFrameSize;
};



#endif // MindVisionCam_h_2015_
