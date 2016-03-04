#ifndef COMPARE_H_2016_03_
#define COMPARE_H_2016_03_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "HWFaceRecSDK.h"
#include "highgui.h"

//////////////////////////////////////////////////////////////////////////////////
typedef HWRESULT (*tagHWInitialD)( char *strName);
typedef HWRESULT (*tagHWReleaseD)();
typedef HWRESULT (*tagHWInitial)( HW_HANDLE *pHandle, char *strName);
typedef HWRESULT (*tagHWRelease)( HW_HANDLE *pHandle);
typedef HWRESULT (*tagHWDetectFaceKeyPoints)(HW_HANDLE Handle,
                                             const unsigned char*pImg, 
                                             int nImgWidth, int nImgHeight,
                                             int* pnMaxFace, 
                                             HWFaceInfo *pFaceInfo);
                                                         
typedef HWRESULT (*tagHWGetFeatureSize)( HW_HANDLE Handle, int *piFtrSize );
typedef HWRESULT (*tagHWExtractFeature)( HW_HANDLE Handle,
                                         const unsigned char* pImg, int nImgWidth, int nImgHeight,
                                         HWFaceInfo *pFaceInfo,
                                         unsigned char *pOutFeature);
                                         typedef HWRESULT  (*tagHWCompareFeature)( HW_HANDLE Handle,
                                         const unsigned char *pFeaA,
                                         const unsigned char *pFeaB,
                                         float *fScore);
typedef HWRESULT (*tagHWSetPortrait)( HW_HANDLE Handle, int iPortrait);
                                                            
//////////////////////////////////////////////////////////////////////////////////

#define PHOTO_WIDTH  (102*2)
#define PHOTO_HEIGHT (126*2)
#define IMAGE_WIDTH  640
#define IMAGE_HEIGHT 480

typedef struct _PIC_
{
	int width;
	int height;
	unsigned char* pixel;
}PICPIXEL;

PICPIXEL* CreatePicPixel(int nW, int nH);
void DeletePicPixel(PICPIXEL* pPicPixel);

int   InitEngine();
//float TestCompare1V1( HW_HANDLE MyHandle, PICPIXEL* pIDPic, PICPIXEL* pLivePic);
float TestCompare1V1( HW_HANDLE MyHandle, PICPIXEL* pIDPic, PICPIXEL* pLivePic, float fInitFaceCmpRate, int bPortrait=1);
//void  TestCompare1VN( HW_HANDLE MyHandle);
void  ReleaseEngine();
int   InitialCompare();


#endif // COMPARE_H_2016_03_
