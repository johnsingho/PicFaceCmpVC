/*******************************************************

  Copyright (c) 2015 Hanwang Tech, All rights reserved.

  例子程序。
  核心初始化，Handle初始化。
  人脸的定位，特征提取，特征比对。  1比1 比对， 1比N 比对。
*******************************************************/
#include "stdafx.h"
#include "Compare.h"
#include "comm/misc.h"

//例子样本
// #define EXAMPLE_FILE1 "\\Img-1-1.bin"//"1.bin";
// #define EXAMPLE_FILE2 "\\Img-1-2.bin"//"1.bin";
// #define EXAMPLE_FILE3 "\\Img-2-1.bin"//"1.bin";
// #define EXAMPLE_FILE4 "\\1-0.bin"//"1.bin";
// #define EXAMPLE_FILE5 "\\1-0-1.bin"//"1.bin";
//#define STR_DLL "D:\\Proc\\HwCompare.dll"
//#define STR_PATH "D:\\Proc"

//dll库所在位置和路径
#define STR_DLL "HWcompare.dll"
char   CurrentPath[MAX_PATH];   


typedef HWRESULT (*tagHWInitialD)( char *strName);
typedef HWRESULT (*tagHWReleaseD)();
typedef HWRESULT (*tagHWInitial)( HW_HANDLE *pHandle, char *strName);
typedef HWRESULT (*tagHWRelease)( HW_HANDLE *pHandle);
typedef HWRESULT (*tagHWDetectFaceKeyPoints)( HW_HANDLE Handle,
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


tagHWInitialD             MyInitD       = NULL;
tagHWReleaseD             MyReleaseD    = NULL;
tagHWInitial              MyInitH       = NULL;
tagHWRelease              MyReleaseH    = NULL;
tagHWDetectFaceKeyPoints  MyDetect      = NULL;
tagHWGetFeatureSize       MyGetFtrSize  = NULL;
tagHWExtractFeature       MyGetFtr      = NULL;
tagHWCompareFeature       MyCompare     = NULL;
tagHWSetPortrait          MySetPortrait = NULL;

HMODULE MyLib = NULL;
HW_HANDLE MyHandle = NULL;

//int   InitEngine();
//void  TestCompare1V1( HW_HANDLE MyHandle);
//void  TestCompare1VN( HW_HANDLE MyHandle);
//void  ReleaseEngine();

//得分的阈值，判断是否同一个人。需要根据实际情况调整。
//可设置0.6f 到0.9f 之间的数值。越小越宽松，越高越严格。
//一般身份证与实际现场人脸照片 时间跨度及光线差别较大的比对情况，用0.55~0.6的阈值。
//时间跨度不大，光线差别不大的情况需要根据实际情况把阈值提高。
const float TH_RESHOLD = 0.6f; //身份证比对阈值
                          


int    initialCompare()
{
	int ret =-1;
    //识别核心初始化
    CString strCurDir = MakeModuleFilePath("");
    strncpy(CurrentPath, strCurDir, strCurDir.GetLength()-1);
	//GetCurrentDirectory(MAX_PATH,CurrentPath); 
    if( S_OK != InitEngine())
    {
//        printf("Init Failed\n");
        return ret;
    }
    //初始化一个Handle. 不同的线程可以初始化各自的Handle使用。
    if( S_OK != MyInitH( &MyHandle, CurrentPath))
    {
//        printf("Init Handle Failed\n");
        return ret;
    }
	else 
	{
//		printf("Init Handle Ok\n");
		ret=0;
	}
	return ret;
}


//===================================================
//识别核心初始化
 int     InitEngine()
{
    MyLib  = LoadLibrary( STR_DLL);
    if( MyLib == NULL )
    {
        //printf("\nLoad Lib Failed");
        return S_FAIL;
    }
    MyInitD       = (tagHWInitialD)GetProcAddress( MyLib, "HWInitialD");
    MyReleaseD    = (tagHWReleaseD )GetProcAddress( MyLib, "HWReleaseD");
    MyInitH       = (tagHWInitial)GetProcAddress( MyLib, "HWInitial");
    MyReleaseH    = (tagHWRelease)GetProcAddress( MyLib, "HWRelease");
    MyDetect      = (tagHWDetectFaceKeyPoints)GetProcAddress( MyLib, "HWDetectFaceKeyPoints");
    MyGetFtrSize  = (tagHWGetFeatureSize)GetProcAddress( MyLib, "HWGetFeatureSize");
    MyGetFtr      = (tagHWExtractFeature)GetProcAddress( MyLib, "HWExtractFeature");
    MyCompare     = (tagHWCompareFeature)GetProcAddress( MyLib, "HWCompareFeature");
    MySetPortrait = (tagHWSetPortrait)GetProcAddress( MyLib, "HWSetPortrait");

    if( MyInitD == NULL || MyReleaseD == NULL ||
        MyInitH == NULL || MyReleaseH == NULL ||
        MyDetect == NULL || MySetPortrait == NULL ||
        MyGetFtrSize == NULL ||
        MyGetFtr == NULL || MyCompare == NULL )
    {
        //printf("\nFind Proc Address Failed");
        return S_FAIL;
    }
    if( S_OK != MyInitD(CurrentPath))
    {
        //printf("\nInit Failed");
        return S_FAIL;
    }
    return S_OK;
}

//释放识别核心。
void ReleaseEngine()
{
    if( MyReleaseD != NULL )
    {
        MyReleaseD();
    }
    if( MyLib != NULL )
    {
        FreeLibrary( MyLib);
    }
}

int GetFeature( HW_HANDLE Handle, const unsigned char *pb, 
                      int iW, int iH, 
                      unsigned char *pbFtr)
{
    HWRESULT iRst;
    HWFaceInfo MyInfo;
    int iMaxFace = 1;

    iRst = MyDetect( Handle, pb, iW, iH, &iMaxFace, &MyInfo );
    //MyDetect进行人脸定位，返回的MyInfo包含人脸定位的信息，
    //MyInfo.m_FaceRect 给出的人脸框矩形坐标，应用程序可依照这个信息在屏上画出相应小框。
    //显示人脸定位的结果。
    if( iRst == S_OK && iMaxFace > 0)
    {
        //根据定位信息求出人脸特征。
        if( S_OK == MyGetFtr( Handle, pb, iW, iH, &MyInfo, pbFtr))
        {
            OutputDebugString("***HWExtractFeature ok\n"); //! temp for debug
            return S_OK;
        }
    }
    return S_FAIL;
}


// unsigned char * ReadAFile( char *str, int *piLen)
// {
//     FILE *pF;
//     int iLen;
//     unsigned char *pb;
// 
//     pF =fopen( str, "rb");
//     if( pF == NULL )
//     {
// //        printf("Open %s Failed\n", str );
//         *piLen = 0;
//         return NULL;
//     }
//     fseek( pF, 0, SEEK_END);
//     iLen = ftell( pF );
//     fseek( pF, 0, SEEK_SET);
//     pb = new BYTE[ iLen];
//     fread( pb, 1, iLen, pF );
//     fclose( pF );
//     *piLen = iLen;
//     return pb;
// }



//此函数使用的是2016-01-28之前的汉王SDK
//比较两张图片。1 比 1 比对。
//返回相似度
//
// float  TestCompare1V1( HW_HANDLE MyHandle, PICPIXEL* pIDPic, PICPIXEL* pLivePic)
// {
//     unsigned char *pbImg;
//     int iFtrSize;
//     unsigned char *pbFtr1 = NULL, *pbFtr2 = NULL;
//     float fScore=0,fScoreMax=0;
//     const int MAX_TRY_FACES = 2;
//     int i=0;
// //     pF =fopen( "Ftr.bin","wb");
// 
//     MyGetFtrSize( MyHandle, &iFtrSize);
//     pbFtr1 = new unsigned char[ iFtrSize];
//     pbFtr2 = new unsigned char[ iFtrSize*MAX_TRY_FACES];
// 
//     pbImg = pIDPic->pixel;
//     if( S_OK != GetFeature( MyHandle, pbImg, pIDPic->width, pIDPic->height, pbFtr1, 1))
//     {
//         goto CleanUp;
//     }
//     pbImg = NULL;
// 
//     pbImg = pLivePic->pixel;
//     if( S_OK != GetFeature( MyHandle, pbImg, pLivePic->width, pLivePic->height, pbFtr2, MAX_TRY_FACES))
//     {
//         goto CleanUp;
//     }
//     pbImg = NULL;
// 
// 	for(i=0;i<MAX_TRY_FACES;i++)
// 	{
// 		MyCompare( MyHandle, pbFtr1, &pbFtr2[iFtrSize*i], &fScore);
// 		if(fScore>fScoreMax)   fScoreMax=fScore;
// 	}
// 
// CleanUp:
//     if( pbFtr1 != NULL )
//     {
//         delete []pbFtr1;
//     }
//     if( pbFtr2 != NULL )
//     {
//         delete []pbFtr2;
//     }
//     if( pbImg != NULL )
//     {
//         pbImg =NULL;
//     }
//     return fScoreMax;
// }


//使用2016-01-28的新SDK
//比较两张图片。1 比 1 比对。
//返回相似度
//
float  TestCompare1V1( HW_HANDLE MyHandle, PICPIXEL* pIDPic, PICPIXEL* pLivePic, float fInitFaceCmpRate, int bPortrait/*=1*/)
{
    int iFtrSize;    
    unsigned char *pbFtr1 = NULL, *pbFtr2 = NULL;    
    float fScore=fInitFaceCmpRate;
    
    MyGetFtrSize( MyHandle, &iFtrSize);
    pbFtr1 = new unsigned char[ iFtrSize];
    pbFtr2 = new unsigned char[ iFtrSize];

  	//如果确定是证件照，可以设Portrait= 1， 否则设Portrait = 0
  	MySetPortrait( MyHandle, bPortrait);
  	
    if( S_OK != GetFeature( MyHandle, pIDPic->pixel, pIDPic->width, pIDPic->height, pbFtr1))
    {
        //printf("\nGet Ftr From %s Failed", strPic1 );
        goto CleanUp;
    }
    
    if( S_OK != GetFeature( MyHandle, pLivePic->pixel, pLivePic->width, pLivePic->height, pbFtr2))
    {
        //printf("\nGet Ftr From %s Failed", strPic2 );
        goto CleanUp;
    }
    
    OutputDebugString("***HWExtractFeature all ok\n"); //! temp for debug
    MyCompare( MyHandle, pbFtr1, pbFtr2, &fScore);
//     printf("\nCompare Score = %f", fScore);
//     if( fScore > TH_RESHOLD )
//     {
//         printf("\nMaybe Same Person");
//     }

CleanUp:
    if( pbFtr1 != NULL )
    {
        delete []pbFtr1;
    }
    if( pbFtr2 != NULL )
    {
        delete []pbFtr2;
    }

    return fScore;
}


//读入前面两个图片特征，
//一个新图片与已有的图片特征比对。 1比N 比对。
// void    TestCompare1VN( HW_HANDLE MyHandle)
// {
//     int iLen;
//     int i;
//     int iFtrSize;
//     int iNum;
//     int iW, iH;
//     int iIdx;
//     float fMaxScore;
//     unsigned char *pbImg = NULL;
//     unsigned char *pbFtrLib = NULL, *pbFtr3 = NULL;
//     char *strPic3 = EXAMPLE_FILE2;
//     
//     pbFtrLib = ReadAFile( "Ftr.bin", &iLen);
//     MyGetFtrSize( MyHandle, &iFtrSize);
//     iNum = iLen / iFtrSize;//求出文件中保存了几个人脸的特征
// 
//     pbFtr3 = new unsigned char[ iFtrSize ];
//     pbImg = ReadAFile( strPic3, &i);
//     iW = 480;
//     iH = 640;
// 
//     if( S_OK != GetFeature( MyHandle, pbImg, iW, iH, pbFtr3,1))
//     {
// //        printf("Get Ftr From %s Failed\n", strPic3 );
//         goto CleanUp;
//     }
// 
//     //与保存的特征逐一比对，求出最好的特征
//     iIdx = -1;
//     fMaxScore = 0;
//     for( i=0; i<iNum; i++)
//     {
//         float fScore;
//         MyCompare( MyHandle, &pbFtrLib[i*iFtrSize], pbFtr3, &fScore);
//         if( fScore > fMaxScore)
//         {
//             fMaxScore = fScore;
//             iIdx = i;
//         }
//     }
// //    printf("Most Similar : %d  Score = %f\n", iIdx, fMaxScore);
//     if( fMaxScore > TH_RESHOLD )
//     {
// //        printf("Maybe Same Person\n");
//     }
// 
// CleanUp:
//     if( pbImg != NULL )
//     {
//         delete []pbImg;
//     }
//     if( pbFtr3 != NULL)
//     {
//         delete []pbFtr3;
//     }
//     if( pbFtrLib != NULL )
//     {
//         delete []pbFtrLib;
//     }
// 
// }



PICPIXEL* CreatePicPixel( int nW, int nH )
{
    PICPIXEL* pPic = new PICPIXEL;
    pPic->pixel = new unsigned char[nW*nH];
    if(!pPic->pixel){ delete pPic; return NULL;}
    pPic->width=nW;
    pPic->height=nH;
    return pPic;
}

void DeletePicPixel( PICPIXEL* pPicPixel)
{
    if(pPicPixel)
    {
        delete[] pPicPixel->pixel;
        delete pPicPixel;
    }
}

