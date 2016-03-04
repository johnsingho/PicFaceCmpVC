// cameraDlg.h : header file
//

#if !defined(AFX_CAMERADLG_H__DCA5C06A_DD8F_407F_95F5_D5B3C4E76E14__INCLUDED_)
#define AFX_CAMERADLG_H__DCA5C06A_DD8F_407F_95F5_D5B3C4E76E14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

#include "MyButton.h"
#include "MyStatic.h"
#include "IDStatic.h"
#include "comm/IdReadHelper.h"
#include "FaceExam.h"
#include "camerads.h"
#include "highgui.h"
#include <cv.h>
#include <afxmt.h>

#include "comm/sw_lock03.h"
#include "comm/MindVisionCam.h"


//配置文件信息
struct stConfigInfo
{
    bool bReWrite;
    unsigned char nIDReaderCOM;     /*身份证读卡器的COM口位置*/
    //char          szZBarPath[300];  /*zbarimg.exe所在的位置*/
    int           nCamFaceID;       /*人脸摄像头ID*/
    int           nCamTicketID;
    int           nMaxRetryFaceCmp;    /*单轮的最多人脸识别次数*/
    int           nMaxRetryTicketChk;  /*单轮的最多车票检测次数*/
    int           nReqFaceCmpScore;  /*允许通过的人脸识别相似度，不应该低于60*/
    int           nGateBoardCOM;     /*灯板、闸门控制器COM口*/
    double        fInitFaceCmpRate;  /*用于2016-01-29新SDK测试的初始识别率(0~999)*/
};


// 主界面
class CCameraDlg : public CDialog
{
// Construction
public:
	CCameraDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCameraDlg)
	enum { IDD = IDD_CAMERA_DLGPIC };
	CMyStatic	m_stTimeShow;
	CMyStatic	m_stLeftInfo;
    CMyStatic	m_stLeftInfoTitle;
    CMyStatic	m_stRightInfoTitle;
	CIDStatic	m_stIDShow;
	CMyStatic	m_stIDCheck;
    CIDStatic	m_stTicketShow;
	CMyStatic	m_stTicketCheck;
	CStatic	    m_stCam;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCameraDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCameraDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnBtnTest();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    static UINT InitEnvThread(void *param);
    static UINT MainThread(void *param);
    static UINT ShowCamPicThread(void *param);
    static UINT InitSoundThread(void *param);

    bool TryInitIDCardReader();
    void UpdatePromptInfo( CString sMsg, COLORREF clrText=-1);
    // 允许主工作线程退出?
    inline bool WorkDone(){return m_workThreads.bMainCanExit;}    
    // 摄像头画面线程结束?
    inline bool UpdateCamPicDone(){return m_workThreads.bCamPicCanExit;}
    
    bool ReadIDCardInfo();
    bool DoCheckTicket(CString& strOutQrCode);
    bool DoFaceCmp(float* pfScore=NULL);
    void SwitchCamera( bool bFaceOrTicket );
    CvvImage* SaveFramePic( IplImage* pFrame);
    inline int GetMaxFaceCmpTimes()   {return m_cfgInfo.nMaxRetryFaceCmp;}
    inline int GetMaxTicketChkTimes() {return m_cfgInfo.nMaxRetryTicketChk;}
    void LetGo();
    void ResetIDCardInfo();
    void UpdateRightPicText( const char* pstrRight);
    void UpdateIconCheck( int nIcon, int nState );
    void FlashAndLight( int iLight );    
    void SwitchLight( int iLight, bool bOpen);    
    void FlashLight( int iLight, DWORD dwMs );
    void AsyncPlayVoice(const char* pstrVoice);
    void PlayVoice(const char* pstrVoice);        
    void FaceCamPlay( bool bPlay );
    bool ValidTicket( CString strQrCode );
    inline CIDBaseTextDecoder* GetIDCardInfo(){return &m_idTextDecoder;}
    void KeepCompareInfo();
private:
    void LoadConfigInfo();
    void SaveConfigInfo();
    void SetUI();
    bool InitIDCardReader( int nPort);
    void InitVars();
    bool TryInitFaceCmp();
    bool TryInitCameras();    
    void ShowIDCardInfo(CIDBaseTextDecoder* pidDecoder);
    bool WritePhotoFile( const char* pstrID, unsigned char* pbyPhoto);    
    CWnd* GetCtrlCamPic();
    void DoExit();
    void StartMainThread();
    void StartCamThread();
    void LockCam();
    void UnLockCam();
    bool TryInitGateBoard();    
    //bool SaveTicketPic(); 
    
    inline void* GetCurCamera( bool& bFaceOrTicket)
    {
        bFaceOrTicket=m_swCamerPic.bFaceOrTicket;
        return m_swCamerPic.pCurCamera;
    }
    void UpdateShowTime();
    void DetectFace( IplImage* pFrame);    
    
private:
    CBitmap m_bmpBg;
	CSize   m_bmpSize;
    CBitmap m_bmpRight;
    CBitmap m_bmpWrong;
    CBitmap m_bmpIDCard;
    CBitmap m_bmpIDBack;
    CBitmap m_bmpTicket;

    stConfigInfo  m_cfgInfo;
    CIdReadHelper m_idCardReader;
    CIDBaseTextDecoder m_idTextDecoder;

    //CCameraDS m_camFace;   //拍人摄像头，像素较高
    CMindVisionCamOper m_camFace;
    CCameraDS m_camTicket; //拍车票，像素较低

    CString   m_strLastIDPicPath; //上一次的照片全路径
    
    //用来切换摄像头显示
    struct stCameraPic
    {
        CCriticalSection csSwitch;
        //CCameraDS*       pCurCamera;
        void*            pCurCamera;
        bool             bFaceOrTicket; /*true拍人，false拍车票*/
        CvvImage         m_curFrameImg; /*当前帧图像*/
        CString          m_strLastTickFile; /*上一次的车票截图文件*/
    }m_swCamerPic;

    struct stWorkThread
    {
        CWinThread*   pMainThread;
        volatile bool bMainCanExit;
        CWinThread*   pCamPicThread;
        volatile bool bCamPicCanExit;
    }m_workThreads;

    struct stPlaySoundThread
    {
        CDialog*    pMainDlg;
        CWinThread* pSoundThread;
        CCriticalSection csSound;
        int         nState; /*>0播放，=0没动作，<0退出线程*/
        CString     sSoundFile;
    }m_playSoundThread;

    CBitmap m_bmpPerson;

    SWLock03       m_gateBoard; //灯板，闸门控制
    CGateBoardOper m_gateBoardOper;
    CFaceDetect    m_faceDetector; //使用人脸识别库
};

///////////////////////////////////////////////////

class CHandlerReadIDCard : public CHandlerBase
{
public:
    CHandlerReadIDCard(){}
    virtual void PreDo(void* pData);
    virtual void Do(void* pData);
private:    

};

class CHandlerFaceCmp : public CHandlerBase
{
public:
    CHandlerFaceCmp(){}
    virtual void PreDo(void* pData);
    virtual void Do( void* pData );    
    void WriteFaceCmpLog( void* pData, float fScore );
};

class CHandlerTicketCheck : public CHandlerBase
{
public:
    CHandlerTicketCheck(){}
    virtual void PreDo(void* pData);
    virtual void Do( void* pData );    
};

class CHandlerOpenGate : public CHandlerBase
{
public:
    CHandlerOpenGate(){}    
    virtual void Do( void* pData );    
};

// 不太重要的错误
// 处理完之后就回到刷身份证的画面
//
class CHandlerException : public CHandlerBase
{
public:
    CHandlerException(){}    
    virtual void Do( void* pData);    
};

// 重要的错误
// 不回到刷身份证画面
//
class CHandlerError : public CHandlerBase
{
public:
    CHandlerError(){}    
    virtual void Do( void* pData);    
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERADLG_H__DCA5C06A_DD8F_407F_95F5_D5B3C4E76E14__INCLUDED_)
