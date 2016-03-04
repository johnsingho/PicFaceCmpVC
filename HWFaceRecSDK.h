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
    Rect  m_FaceRect;//������λ��
    float m_afKeyPoint[81*2];//
    KeyPos m_KeyPos;//
}HWFaceInfo;
#endif

#ifdef __cplusplus
extern "C"
{
#endif


//��ʼ������, strName ����dll����·������"D:\\Prog"
//������غ�����һ�Σ�����HWInitial����
//return: S_OK, S_FAIL
HWRESULT HWInitialD( char *strName);

//�ͷź���
//�����˳�ǰ����һ��
//return: S_OK, S_FAIL
HWRESULT HWReleaseD( );

//��ʼ��һ��HANDLE . ���߳�����¸����̳߳�ʼ�����Ե�Handle,
//pHandle [output]ָ���ʼ���õ�Handle
//strName [input]NULL
//return: S_OK, S_FAIL
HWRESULT HWInitial( HW_HANDLE *pHandle, char *strName);

//�ͷ�Handle
//pHandle [input]ָ��HWInitial��ʼ���õ�Handle
HWRESULT HWRelease( HW_HANDLE *pHandle);

/**************************************************

    ������λ
    
*************************************************/
//������λ
//Handle [input] HWInitial��ʼ���õ�Handle
//pImg   [input] ����ͼƬ�Ҷ���Ϣ����������:ͼƬ�����ϵ����£����� ÿ�д�����������и����صĻҶ�ֵ
//nImgWidth nImgHeight [input] ͼƬ�Ŀ�ȸ߶�
//pnMaxFace [input] ��Ҫ��λ����������� ��1~10)
//          [output] *pnMaxFace Ϊʵ�ʶ�λ����������
//pFaceInfo [output] ���ÿ��������λ��Ϣ�� ��Ҫ�ⲿ����*pnMaxFace�� HWFaceInfo�ռ䡣
//return: S_OK, S_FAIL
HWRESULT HWDetectFaceKeyPoints( HW_HANDLE Handle,
                            const unsigned char*pImg, 
                            int nImgWidth, int nImgHeight,
                            int* pnMaxFace, 
                            HWFaceInfo *pFaceInfo);

//�����Ƿ��ͷ��֮���֤����Ƭ�����ȷ����֤���գ����������iPortrait = 1,������Ϊ0��
//��Ϊ1��λ�����ס�
//Handle [input] HWInitial��ʼ���õ�Handle
//iPortrait [input] 1 �ǵġ�0 ��ȷ����
//return: S_OK, S_FAIL
HWRESULT HWSetPortrait( HW_HANDLE Handle, int iPortrait);
/**************************************************

    �����ͱȶ�

/*************************************************/
//
//Handle [input] HWInitial��ʼ���õ�Handle
//piFtrSize [output] ��������ֽڸ���
//return: S_OK, S_FAIL
HWRESULT HWGetFeatureSize( HW_HANDLE Handle, int *piFtrSize );


//��ȡ������
//Handle [input] HWInitial��ʼ���õ�Handle
//pImg   [input] ����ͼƬ�Ҷ���Ϣ����������:ͼƬ�����ϵ����£����� ÿ�д�����������и����صĻҶ�ֵ
//nImgWidth, nImgHeight[input] ͼƬ�Ŀ�ȸ߶�
//pFaceInfo   [input] һ��������Ϣ
//pOutFeature [output]��������������������ȼ�HWGetFeatureSize�� ��Ҫ�ⲿ����á�
//return : S_OK. other failed
HWRESULT HWExtractFeature( HW_HANDLE Handle,
                          const unsigned char* pImg, int nImgWidth, int nImgHeight,
                          HWFaceInfo *pFaceInfo,
						              unsigned char *pOutFeature);


//���ڵ����Ƚ�����ͼƬ�������������ԡ�
//Handle [input] HWInitial��ʼ���õ�Handle
//pFeaA  [input] ������
//pFeaB  [input] ������
//fScore [output] �����Զ���ֵ��0~1.0 ��Խ��Խ���ơ�
//return : S_OK. other failed
HWRESULT  HWCompareFeature( HW_HANDLE Handle,
                            const unsigned char *pFeaA,
                            const unsigned char *pFeaB,
                            float *fScore);
                               

#ifdef __cplusplus
}
#endif

#endif //HWFACERECSDK_H