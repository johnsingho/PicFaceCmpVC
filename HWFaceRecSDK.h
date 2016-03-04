#ifndef HWFACERECSDK_H
#define HWFACERECSDK_H

typedef long HWRESULT;
typedef void*  HW_HANDLE;

#define S_OK  0
#define S_FAIL 1

#define LEFT_EYE   6
#define RIGHT_EYE  14

#ifndef  __RECT
#define  __RECT
typedef struct tagARect
{
	int top;
	int bottom;
	int left;
	int right;
}Rect;
#endif

#ifndef __POS
#define __POS
typedef struct  tagPos
{
	int col;
	int row;
	int width;
	int confi;
}Pos;
#endif

#ifndef __KEYPOS
#define __KEYPOS
typedef struct tagKeyPos
{
	Pos face;
	Pos leftEye;
	Pos rightEye;
	Pos leftUpperEye;
	Pos rightUpperEye;
	Pos leftleftEye;
	Pos leftrightEye;
	Pos rightleftEye;
	Pos rightrightEye;
	Pos leftNostril;
	Pos rightNostril;
	Pos nosePoint;
	Pos leftMouth;
	Pos rightMouth;
}KeyPos;
#endif
#ifndef HW_FACE_INFO
#define HW_FACE_INFO
typedef struct tagHWFaceInfo{
    Rect  m_FaceRect;//人脸定位框
    float m_afKeyPoint[81*2];//
    KeyPos m_KeyPos;//
}HWFaceInfo;
#endif

#ifdef __cplusplus
extern "C"
{
#endif


//初始化核心, strName 输入dll所在路径，如"D:\\Prog"
//程序加载后运行一次，先于HWInitial运行
//return: S_OK, S_FAIL
HWRESULT HWInitialD( char *strName);

//释放核心
//程序退出前运行一次
//return: S_OK, S_FAIL
HWRESULT HWReleaseD( );

//初始化一个HANDLE . 多线程情况下各个线程初始化各自的Handle,
//pHandle [output]指向初始化好的Handle
//strName [input]NULL
//return: S_OK, S_FAIL
HWRESULT HWInitial( HW_HANDLE *pHandle, char *strName);

//释放Handle
//pHandle [input]指向HWInitial初始化好的Handle
HWRESULT HWRelease( HW_HANDLE *pHandle);

/**************************************************

    人脸定位
    
*************************************************/
//人脸定位
//Handle [input] HWInitial初始化好的Handle
//pImg   [input] 输入图片灰度信息，数据内容:图片从左上到右下，逐行 每行从左到右逐点排列各像素的灰度值
//nImgWidth nImgHeight [input] 图片的宽度高度
//pnMaxFace [input] 需要定位最多人脸个数 （1~10)
//          [output] *pnMaxFace 为实际定位的人脸个数
//pFaceInfo [output] 输出每个人脸定位信息。 需要外部申请*pnMaxFace个 HWFaceInfo空间。
//return: S_OK, S_FAIL
HWRESULT HWDetectFaceKeyPoints( HW_HANDLE Handle,
                            const unsigned char*pImg, 
                            int nImgWidth, int nImgHeight,
                            int* pnMaxFace, 
                            HWFaceInfo *pFaceInfo);

//设置是否大头照之类的证件照片。如果确定是证件照，则可以设置iPortrait = 1,否则设为0。
//设为1则定位较容易。
//Handle [input] HWInitial初始化好的Handle
//iPortrait [input] 1 是的。0 不确定。
//return: S_OK, S_FAIL
HWRESULT HWSetPortrait( HW_HANDLE Handle, int iPortrait);
/**************************************************

    特征和比对

/*************************************************/
//
//Handle [input] HWInitial初始化好的Handle
//piFtrSize [output] 输出特征字节个数
//return: S_OK, S_FAIL
HWRESULT HWGetFeatureSize( HW_HANDLE Handle, int *piFtrSize );


//提取特征。
//Handle [input] HWInitial初始化好的Handle
//pImg   [input] 输入图片灰度信息。数据内容:图片从左上到右下，逐行 每行从左到右逐点排列各像素的灰度值
//nImgWidth, nImgHeight[input] 图片的宽度高度
//pFaceInfo   [input] 一个人脸信息
//pOutFeature [output]输出特征串。特征串长度见HWGetFeatureSize， 需要外部申请好。
//return : S_OK. other failed
HWRESULT HWExtractFeature( HW_HANDLE Handle,
                          const unsigned char* pImg, int nImgWidth, int nImgHeight,
                          HWFaceInfo *pFaceInfo,
						              unsigned char *pOutFeature);


//用于单独比较两张图片的特征串相似性。
//Handle [input] HWInitial初始化好的Handle
//pFeaA  [input] 特征串
//pFeaB  [input] 特征串
//fScore [output] 相似性度量值，0~1.0 ，越大越相似。
//return : S_OK. other failed
HWRESULT  HWCompareFeature( HW_HANDLE Handle,
                            const unsigned char *pFeaA,
                            const unsigned char *pFeaB,
                            float *fScore);
                               

#ifdef __cplusplus
}
#endif

#endif //HWFACERECSDK_H