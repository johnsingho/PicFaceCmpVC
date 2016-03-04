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


//�����ļ���Ϣ
struct stConfigInfo
{
    bool bReWrite;
    unsigned char nIDReaderCOM;     /*���֤��������COM��λ��*/
    //char          szZBarPath[300];  /*zbarimg.exe���ڵ�λ��*/
    int           nCamFaceID;       /*��������ͷID*/
    int           nCamTicketID;
    int           nMaxRetryFaceCmp;    /*���ֵ��������ʶ�����*/
    int           nMaxRetryTicketChk;  /*���ֵ���೵Ʊ������*/
    int           nReqFaceCmpScore;  /*����ͨ��������ʶ�����ƶȣ���Ӧ�õ���60*/
    int           nGateBoardCOM;     /*�ư塢բ�ſ�����COM��*/
    double        fInitFaceCmpRate;  /*����2016-01-29��SDK���Եĳ�ʼʶ����(0~999)*/
};


// ������
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
    // �����������߳��˳�?
    inline bool WorkDone(){return m_workThreads.bMainCanExit;}    
    // ����ͷ�����߳̽���?
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

    //CCameraDS m_camFace;   //��������ͷ�����ؽϸ�
    CMindVisionCamOper m_camFace;
    CCameraDS m_camTicket; //�ĳ�Ʊ�����ؽϵ�

    CString   m_strLastIDPicPath; //��һ�ε���Ƭȫ·��
    
    //�����л�����ͷ��ʾ
    struct stCameraPic
    {
        CCriticalSection csSwitch;
        //CCameraDS*       pCurCamera;
        void*            pCurCamera;
        bool             bFaceOrTicket; /*true���ˣ�false�ĳ�Ʊ*/
        CvvImage         m_curFrameImg; /*��ǰ֡ͼ��*/
        CString          m_strLastTickFile; /*��һ�εĳ�Ʊ��ͼ�ļ�*/
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
        int         nState; /*>0���ţ�=0û������<0�˳��߳�*/
        CString     sSoundFile;
    }m_playSoundThread;

    CBitmap m_bmpPerson;

    SWLock03       m_gateBoard; //�ư壬բ�ſ���
    CGateBoardOper m_gateBoardOper;
    CFaceDetect    m_faceDetector; //ʹ������ʶ���
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

// ��̫��Ҫ�Ĵ���
// ������֮��ͻص�ˢ���֤�Ļ���
//
class CHandlerException : public CHandlerBase
{
public:
    CHandlerException(){}    
    virtual void Do( void* pData);    
};

// ��Ҫ�Ĵ���
// ���ص�ˢ���֤����
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
