#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "HWFaceRecSDK.h"

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
int   initialCompare();