// cameraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "camera.h"
#include "cameraDlg.h"
#include "comm/misc.h"
#include "zbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

// 查找身份证读卡器时的最大尝试端口
#define MAX_TRY_COM         (20)
// 默认配置文件
#define DEF_CONFIG_FILE     "profile.ini"
// 默认身份证照片目录(程序目录下)
#define DEF_IDPIC_DIR       "pic"
// 工作轮询间隔(ms)
#define DEF_WAIT_POLLING    (0.2*1000)
// 人脸摄像头轮询间隔(ms)
#define DEF_WAIT_FACE       (40)

// 车票识别重试次数
#define DEF_MAX_RETRY_TICKETCHK (25)
// 人脸识别重试次数
#define DEF_MAX_RETRY_FACECMP   (10)
// 默认的人脸识别阈值
#define DEF_REQ_FACECMP_SCORE   (60)
// 默认的灯板、闸门控制器COM口
#define DEF_GATE_BOARD_COM      (7)
// 用于2016-01-29新SDK测试的初始识别率(0~999)
#define DEF_INIT_FACECMP_RATE   (322)

#define IMAGE_CHANNELS          (3)

// from Compare.cpp
//extern PICPIXEL picPixel[2];
extern HW_HANDLE MyHandle;

using namespace zbar;
#pragma comment(lib, "libzbar-0.lib")

#define DEF_SYS_NAME "人脸识别自助进站系统"

//闸机控制板
#include "comm/sw_lock03.h"

//闸机调试窗口
#include "DialogLockTest.h"
CDialogLockTest *pDlgLockTest=NULL;


#define CLR_TITLE     RGB(255, 69, 0)
#define CLR_INFO      RGB(11, 0, 0)
#define CLR_ERROR     RGB(220, 20, 60)
#define CLR_SUCCESS   RGB(0,128, 0)
#define CLR_INFO_READ RGB(20, 100, 20)

//音频文件名
#define VOICE_INIT_OK       "InitOk.wav"
#define VOICE_INIT_FAIL     "InitFail.wav"
#define VOICE_PASS          "pass.wav"
//#define VOICE_FAIL          "fail.wav"
#define VOICE_FAIL_FACECMP  "FaceCmpFail.wav"
#define VOICE_FAIL_TICKCHK  "TicketChkFail.wav"
#define VOICE_VIEW_CAM      "viewcam.wav"
#define VOICE_PLACE_TIC     "placeTicket.wav"

//日志文件名
#define SYSTEMLOG_FILENAME  "system.log"
#define FACECMPLOG_FILENAME "FaceCmp.log"

/////////////////////////////////////////////////////////////////////////////
// CCameraDlg dialog

CCameraDlg::CCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCameraDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCameraDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_cfgInfo, 0, sizeof(m_cfgInfo));
    memset(&m_workThreads, 0, sizeof(m_workThreads));

    m_playSoundThread.pMainDlg = this;
    m_playSoundThread.pSoundThread=NULL;
    m_playSoundThread.nState=0;
    //m_playSoundThread.csSound;

    m_pIDPhoto=NULL;
    m_pLivePic=NULL;    
}

void CCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCameraDlg)
	DDX_Control(pDX, IDC_ST_TIMESHOW, m_stTimeShow);
	DDX_Control(pDX, IDC_ST_LEFT, m_stLeftInfo);
	DDX_Control(pDX, IDC_ST_LEFTTITLE, m_stLeftInfoTitle);
	DDX_Control(pDX, IDC_ST_RIGHTTITLE, m_stRightInfoTitle);
	DDX_Control(pDX, IDC_ST_IDSHOW, m_stIDShow);
	DDX_Control(pDX, IDC_ST_IDCHECK, m_stIDCheck);
	DDX_Control(pDX, IDC_ST_TICKSHOW, m_stTicketShow);
	DDX_Control(pDX, IDC_ST_TICKETCHK, m_stTicketCheck);
	DDX_Control(pDX, IDC_ST_CAM, m_stCam);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCameraDlg, CDialog)
	//{{AFX_MSG_MAP(CCameraDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
    ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
	ON_WM_RBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_BTN_TEST, OnBtnTest)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCameraDlg message handlers

BOOL CCameraDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
    LoadConfigInfo();
	SetUI();
    InitVars();
    
    logfile(SYSTEMLOG_FILENAME, "system start");
    UpdatePromptInfo("系统正在初始化，请稍候...");    
    
    AfxBeginThread(InitSoundThread, &m_playSoundThread);
    // 初始化环境
    AfxBeginThread(InitEnvThread, this);
    SetTimer(1, 1000, NULL);
#ifdef _DEBUG
// 	pDlgLockTest = new CDialogLockTest();
// 	pDlgLockTest->Create(IDD_DIALOG_LOCKTEST,this);
// 	pDlgLockTest->MoveWindow(0,0,600,300,FALSE);
// 	pDlgLockTest->ShowWindow(SW_SHOW);
//  pDlgLockTest->SetLocker(&m_gateBoard);
#endif

//#ifndef _DEBUG
    SetCursorPos(1200,1200);
    ShowCursor(FALSE);
//#endif
    
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCameraDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


BOOL CCameraDlg::OnEraseBkgnd(CDC* pDC) 
{
    CDC dcMemory;
    
    dcMemory.CreateCompatibleDC(pDC);
    CBitmap* pOldBitmap = dcMemory.SelectObject(&m_bmpBg);
    CRect rcClient;
    GetClientRect(&rcClient);
    const CSize& sBitmap = m_bmpSize;
    pDC->BitBlt(0,0,sBitmap.cx,sBitmap.cy,&dcMemory,0,0,SRCCOPY);
    dcMemory.SelectObject(pOldBitmap);    
    return TRUE;
    //return CDialog::OnEraseBkgnd(pDC);
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCameraDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCameraDlg::OnDestroy() 
{
#ifdef _DEBUG
    if(pDlgLockTest)
    {
        pDlgLockTest->DestroyWindow();
        delete pDlgLockTest;
    }
#endif

    if(m_cfgInfo.bReWrite)
    {
        SaveConfigInfo();
    }

    m_idCardReader.SerialClose();
    //m_camFace.CloseCamera();
    m_camFace.UnInitCamera();
    m_camTicket.CloseCamera();
	ReleaseEngine();

    CDialog::OnDestroy();
}


void CCameraDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==1)
    {
        UpdateShowTime();
    }
	CDialog::OnTimer(nIDEvent);
}


static void DrawToWnd( CBitmap* pBmp, CWnd* pWnd)
{
    CDC* pDC = pWnd->GetDC();
    CRect rectClient;
    pWnd->GetClientRect(&rectClient);
    rectClient.NormalizeRect();

    BITMAP bm;
    ZeroMemory(&bm, sizeof(bm));
    pBmp->GetBitmap(&bm);

    CDC dcMem; 
    dcMem.CreateCompatibleDC(pDC);
    CBitmap *poldBitmap=(CBitmap*)dcMem.SelectObject(pBmp);
    pDC->StretchBlt(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(),
                    &dcMem,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
    dcMem.SelectObject(poldBitmap);
    pWnd->ReleaseDC(pDC);
}

//重置身份证信息显示
void CCameraDlg::ResetIDCardInfo()
{
    //m_stPicLeft.SetBitmap((HBITMAP)m_bmpPerson);
    //DrawToWnd(&m_bmpPerson, &m_stPicLeft);
    
    UpdateIconCheck(0, -1);
    UpdateIconCheck(1, -1);
    m_stIDShow.SetIDDraw( &m_bmpIDCard, NULL);
    m_stIDShow.RedrawWindow();
    m_stTicketShow.SetIDDraw(&m_bmpTicket, NULL);
    CString str;
    str.Format("欢迎使用\n%s", DEF_SYS_NAME);
    UpdatePromptInfo(str);
}

//////////////////////////////////////////////////////////

UINT CCameraDlg::InitSoundThread(void *param)
{
    CCameraDlg::stPlaySoundThread* pSoundData = static_cast<CCameraDlg::stPlaySoundThread*>(param);
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pSoundData->pMainDlg);
    CSingleLock locker(&pSoundData->csSound);
    const int NWAITS = 500;
    while(1)
    {        
        if(!locker.Lock(NWAITS))
        {
            Sleep(50);
            continue;
        }
        if(pSoundData->nState<0)
        { 
            locker.Unlock();
            break;
        }
        if(pSoundData->nState>0)
        {
            pDlg->PlayVoice(pSoundData->sSoundFile);
            pSoundData->nState=0;
        }
        locker.Unlock();
    }
    return 0;
}

UINT CCameraDlg::InitEnvThread(void *param)
{   
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(param);
    //初始化身份证读取模块
    bool bInitID = pDlg->TryInitIDCardReader();    

    //初始化人脸识别库
    bool bInitFace=pDlg->TryInitFaceCmp();
    //初始化两个摄像头
    bool bInitCam =pDlg->TryInitCameras();
    //初始化灯板、闸门
    bool bInitGateBoard = pDlg->TryInitGateBoard();

    if(bInitID && bInitFace && bInitCam && bInitGateBoard)
    {
        CString str;
        str.Format("欢迎使用\n%s", DEF_SYS_NAME);
        pDlg->UpdatePromptInfo(str);
        pDlg->StartMainThread();
        pDlg->StartCamThread();
        pDlg->AsyncPlayVoice(VOICE_INIT_OK);
        pDlg->FaceCamPlay(true);
    }
    else
    {
        pDlg->AsyncPlayVoice(VOICE_INIT_FAIL);
    }

    return 0;
}

void CCameraDlg::LoadConfigInfo()
{
    CString strIni = MakeModuleFilePath(DEF_CONFIG_FILE);
    CFileStatus fStatus;
    if( !CFile::GetStatus(strIni, fStatus))
    {
        m_cfgInfo.bReWrite=true;
    }
    const int BUF_LEN=300;
    char szBuf[BUF_LEN]={0};
    int nRead=GetPrivateProfileInt("FaceDetect", "IDReaderCOM", 0, strIni);
    m_cfgInfo.nIDReaderCOM = nRead;
    
//     const char* pstrDefZbarimg = "C:\\Program Files\\ZBar\\bin\\zbarimg.exe";
//     GetPrivateProfileString("FaceDetect", "ZbarimgPath", pstrDefZbarimg, szBuf, BUF_LEN, strIni);
//     strcpy(m_cfgInfo.szZBarPath, szBuf);

    nRead = GetPrivateProfileInt("FaceDetect", "CamFaceID", 0, strIni);
    m_cfgInfo.nCamFaceID = nRead;
    nRead = GetPrivateProfileInt("FaceDetect", "CamTicketID", 1, strIni);
    m_cfgInfo.nCamTicketID = nRead;
    nRead = GetPrivateProfileInt("FaceDetect", "MaxRetryFaceCmp", DEF_MAX_RETRY_FACECMP, strIni);
    m_cfgInfo.nMaxRetryFaceCmp = nRead;
    nRead = GetPrivateProfileInt("FaceDetect", "MaxRetryTicketChk", DEF_MAX_RETRY_TICKETCHK, strIni);
    m_cfgInfo.nMaxRetryTicketChk = nRead;
    nRead = GetPrivateProfileInt("FaceDetect", "ReqFaceCmpScore", DEF_REQ_FACECMP_SCORE, strIni);
    m_cfgInfo.nReqFaceCmpScore = nRead;
    nRead = GetPrivateProfileInt("FaceDetect", "GateBoardCOM", DEF_GATE_BOARD_COM, strIni);
    m_cfgInfo.nGateBoardCOM = nRead;
    nRead = GetPrivateProfileInt("FaceDetect", "InitFaceCmpRate", DEF_INIT_FACECMP_RATE, strIni);
    m_cfgInfo.fInitFaceCmpRate = (double)nRead/1000.0;
}

void CCameraDlg::SaveConfigInfo()
{
    CString strIni = MakeModuleFilePath(DEF_CONFIG_FILE);
    CString strOut;
    
    strOut.Format("%d", m_cfgInfo.nIDReaderCOM);
    WritePrivateProfileString("FaceDetect", "IDReaderCOM", strOut, strIni);
//     strOut.Format("%s", m_cfgInfo.szZBarPath);
//     WritePrivateProfileString("FaceDetect", "ZbarimgPath", strOut, strIni);
    strOut.Format("%d", m_cfgInfo.nCamFaceID);
    WritePrivateProfileString("FaceDetect", "CamFaceID", strOut, strIni);
    strOut.Format("%d", m_cfgInfo.nCamTicketID);
    WritePrivateProfileString("FaceDetect", "CamTicketID", strOut, strIni);
    strOut.Format("%d", m_cfgInfo.nMaxRetryFaceCmp);
    WritePrivateProfileString("FaceDetect", "MaxRetryFaceCmp", strOut, strIni);
    strOut.Format("%d", m_cfgInfo.nMaxRetryTicketChk);
    WritePrivateProfileString("FaceDetect", "MaxRetryTicketChk", strOut, strIni);
    strOut.Format("%d", m_cfgInfo.nReqFaceCmpScore);
    WritePrivateProfileString("FaceDetect", "ReqFaceCmpScore", strOut, strIni);
    strOut.Format("%d", m_cfgInfo.nGateBoardCOM);
    WritePrivateProfileString("FaceDetect", "GateBoardCOM", strOut, strIni);
    strOut.Format("%d", int(m_cfgInfo.fInitFaceCmpRate*1000.0));
    WritePrivateProfileString("FaceDetect", "InitFaceCmpRate", strOut, strIni);    
}

void CCameraDlg::SetUI()
{
    m_bmpBg.LoadBitmap(IDB_BMP_BG);
    BITMAP bm;
    m_bmpBg.GetBitmap(&bm);
	m_bmpSize = CSize(bm.bmWidth, bm.bmHeight);

    m_bmpPerson.LoadBitmap(IDB_BMP_PERSON);
    m_bmpRight.LoadBitmap(IDB_BMP_RIGHT);
    m_bmpWrong.LoadBitmap(IDB_BMP_WRONG);
    m_bmpIDCard.LoadBitmap(IDB_BMP_IDCARD);
    m_bmpIDBack.LoadBitmap(IDB_BMP_IDBACK);
    m_bmpTicket.LoadBitmap(IDB_BMP_TICKET);

#ifdef JOHN_DEBUG
    // 测试退出按钮
    GetDlgItem(IDC_BTN_TEST)->ShowWindow(SW_SHOW);
#endif
    
#ifdef _DEBUG
    ::SetWindowPos(AfxGetMainWnd()->m_hWnd,HWND_TOP,-1,-1,-1,-1,SWP_NOSIZE);    
#else
     //设置前端显示,左上角显示
    ::SetWindowPos(AfxGetMainWnd()->m_hWnd,HWND_TOPMOST,-1,-1,-1,-1,SWP_NOSIZE);
#endif

    m_stCam.MoveWindow(84,89,470,358,FALSE);
    m_stCam.ShowWindow(SW_HIDE);
    m_stLeftInfoTitle.MoveWindow(85,520,330,30,TRUE);
    m_stLeftInfo.MoveWindow(85,570,330,180,TRUE);
    m_stRightInfoTitle.MoveWindow(440,550,500,220,TRUE);    
    m_stRightInfoTitle.ModifyStyle(0, SS_CENTER, 0);

    m_stIDCheck.MoveWindow(568,150,81,81,TRUE);
    m_stTicketCheck.MoveWindow(568,320,81,81,TRUE);
    //16*10
    m_stIDShow.MoveWindow(650,70,330,200, TRUE);
    m_stTicketShow.MoveWindow(650,274,330,180, TRUE);
    

    m_stLeftInfo.SetTextClr(CLR_INFO);
    m_stLeftInfoTitle.SetTextClr(CLR_INFO);
    m_stRightInfoTitle.SetTextClr(CLR_INFO);

    CFont fontInfo; 
    fontInfo.CreateFont(22,11,0,0,FW_NORMAL,
        FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"黑体");
    CFont fontTitle; 
    fontTitle.CreateFont(30,15,0,0,FW_BOLD,
        FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"黑体");
    CFont fontTitleBig; 
    fontTitleBig.CreateFont(36,18,0,0,FW_BOLD,
        FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"黑体");
    CFont fontTime; 
    fontTime.CreateFont(20,10,0,0,FW_NORMAL,
        FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"黑体");

    m_stLeftInfo.MySetFont(&fontInfo);
    m_stLeftInfoTitle.MySetFont(&fontTitle);
    m_stRightInfoTitle.MySetFont(&fontTitleBig);

    // 时间显示
    m_stTimeShow.MoveWindow(740,20,250,40,TRUE);
    m_stTimeShow.SetTextClr(CLR_INFO);
    m_stTimeShow.MySetFont(&fontTime);
    
    //------------------------------------------------
    m_stLeftInfoTitle.SetWindowText("操作指引：");
    CString strInfo;
    strInfo += "1、请将身份证放在感应区上面\n";
    strInfo += "2、请正面望向摄像头\n";
    strInfo += "3、请将车票平整放在验票口\n";
    m_stLeftInfo.SetWindowText(strInfo);

    m_stIDShow.ModifyStyle(0, SS_CENTERIMAGE, 0);
    m_stTicketShow.ModifyStyle(0, SS_CENTERIMAGE, 0);
    m_stIDCheck.ModifyStyle(0, SS_CENTERIMAGE, 0);
    m_stTicketCheck.ModifyStyle(0, SS_CENTERIMAGE, 0);

    //m_stIDCheck.SetBitmap((HBITMAP)m_bmpWrong);
    //m_stTicketCheck.SetBitmap((HBITMAP)m_bmpWrong);
    ResetIDCardInfo();
}


// 初始化内部变量
void CCameraDlg::InitVars()
{
    m_pIDPhoto = CreatePicPixel(PHOTO_WIDTH, PHOTO_HEIGHT);
    m_pLivePic = CreatePicPixel(IMAGE_WIDTH, IMAGE_HEIGHT);
    
    SwitchCamera(true);
    m_gateBoardOper.Bind(&m_gateBoard);
}

// bFaceOrTicket
// true 为人脸
// false 为车票
void CCameraDlg::SwitchCamera( bool bFaceOrTicket)
{
    LockCam();
    m_swCamerPic.bFaceOrTicket=bFaceOrTicket;
    if(bFaceOrTicket)
    {
        m_swCamerPic.pCurCamera = &m_camFace;
    }else{
        m_swCamerPic.pCurCamera = &m_camTicket;
        m_camFace.Pause();
    }
    //m_swCamerPic.pCurCamera = bFaceOrTicket ? &m_camFace: &m_camTicket;
    UnLockCam();
}

void CCameraDlg::LockCam()
{
    m_swCamerPic.csSwitch.Lock();
}
void CCameraDlg::UnLockCam()
{
    m_swCamerPic.csSwitch.Unlock();
}

CvvImage* CCameraDlg::SaveFramePic( IplImage* pFrame)
{
    //CSingleLock lock(&m_swCamerPic.csSwitch);
    CvvImage* pImg = &m_swCamerPic.m_curFrameImg;
    pImg->CopyOf(pFrame);
    return pImg;
}

CWnd* CCameraDlg::GetCtrlCamPic()
{
    return &m_stCam;
}

// 更新提示信息
void CCameraDlg::UpdatePromptInfo( CString sMsg, COLORREF clrText/*=-1*/)
{
    m_stRightInfoTitle.SetTextClr((clrText!=-1)?clrText:CLR_INFO);
    m_stRightInfoTitle.SetWindowText(sMsg);
}

bool CCameraDlg::TryInitIDCardReader()
{
    int nPreIDCOM = m_cfgInfo.nIDReaderCOM;
    bool bInit=false;
    if(nPreIDCOM > 0)
    {
        bInit=InitIDCardReader(nPreIDCOM);
    }
    
    if(!bInit)
    {
        CString sMsg;
        for(int i=0;i<MAX_TRY_COM;i++)
        {   
            bInit=InitIDCardReader(i);
            if(bInit)
            {
                m_cfgInfo.nIDReaderCOM=i;
                m_cfgInfo.bReWrite=true;
                break;
            }
        }
    }
    
    if(!bInit)
    {
        const char* pstrErr = "没有找到身份证读卡器\n或端口无法打开!";
        logfile(SYSTEMLOG_FILENAME, pstrErr);
        UpdatePromptInfo(pstrErr);
    }
    return bInit;
}

bool CCameraDlg::InitIDCardReader( int nPort)
{
    bool bOPen = m_idCardReader.SerialOpen(nPort);
    if(bOPen)
    {
        if(0==m_idCardReader.SAM_GetState())
        {
            return true;
        }else{
            m_idCardReader.SerialClose();
        }
    }
    return false;
}


//初始化人脸识别库
bool CCameraDlg::TryInitFaceCmp()
{
    int nRet = initialCompare();
    if(S_OK!=nRet)
    {
        const char* pstrErr = "人脸识别模块初始化失败！";
        logfile(SYSTEMLOG_FILENAME, pstrErr);
        UpdatePromptInfo(pstrErr);
        return false;
    }
    return true;
}

//初始化两个摄像头
bool CCameraDlg::TryInitCameras()
{
    CString strInfo;
    bool bCamFace=false;
    bool bCamTicket=false;
    
    if(0==CCameraDS::CameraCount())
    {
        strInfo="没有连接任何摄像头";
        logfile(SYSTEMLOG_FILENAME, strInfo);
        UpdatePromptInfo(strInfo);
        return false;
    }

//     bCamFace=m_camFace.OpenCamera(m_cfgInfo.nCamFaceID,0,640,480);
//     if( !bCamFace)
//     {
//         strInfo.Format("初始化人脸摄像头失败(ID=%d)", m_cfgInfo.nCamFaceID);
//     }
    bCamFace = m_camFace.Init();
    if(!bCamFace)
    {
        strInfo.Format("初始化人脸摄像头SDK失败");
    }
    else
    {
        bCamFace=m_camFace.InitCamera();
        if(!bCamFace)
        {
            strInfo.Format("连接到人脸摄像头失败！");
        }
    }

    bCamTicket=m_camTicket.OpenCamera(m_cfgInfo.nCamTicketID,0,640,480);
    if( !bCamTicket)
    {
        char strBuf[128];
        sprintf(strBuf, "初始化车票摄像头失败(ID=%d)！", m_cfgInfo.nCamTicketID);
        if(strInfo.GetLength())
        {
            strInfo += "，";
        }
        strInfo += strBuf;
    }

    bool bOk = bCamFace && bCamTicket;
    if(!bOk)
    {
        logfile(SYSTEMLOG_FILENAME, strInfo);
        UpdatePromptInfo(strInfo);
    }
    return bOk;
}


/////////////////////////////////////////////////////////
static CHandlerReadIDCard   g_handlerReadIDCard;
static CHandlerFaceCmp      g_handlerFaceCmp;
static CHandlerTicketCheck  g_handlerTicketCheck;
static CHandlerOpenGate     g_handlerOpenGate;
static CHandlerException    g_handlerException;

// 主工作线程
UINT CCameraDlg::MainThread( void *param)
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(param);
    if(!pDlg){return -1;}

    CJobManager jm;
    jm.disPatch(&g_handlerReadIDCard, pDlg); //init
    while(!pDlg->WorkDone())
    {
        jm.doWork(pDlg);
        //Sleep(DEF_POLLING_INT);
    }

    return 0;
}

void CCameraDlg::StartMainThread()
{
    CWinThread* pThread = AfxBeginThread(MainThread, this);
    if(!pThread)
    {
        UpdatePromptInfo("启动主工作线程失败！");
    }
    m_workThreads.pMainThread=pThread;
}

void CCameraDlg::StartCamThread()
{
    CWinThread* pThread = AfxBeginThread(ShowCamPicThread, this);
    if(!pThread)
    {
        UpdatePromptInfo("启动摄像头线程失败！");
    }
    m_workThreads.pCamPicThread=pThread;
}


// 清退
void CCameraDlg::DoExit()
{
    m_workThreads.bMainCanExit=true;
    m_workThreads.bCamPicCanExit=true;

    m_gateBoard.SerialClose();        

    CWinThread* pThread = m_workThreads.pMainThread;
    DWORD dwState=0;
    DWORD dwWaits = DEF_WAIT_POLLING*5;

    //播放声音线程
    m_playSoundThread.csSound.Lock();
    m_playSoundThread.nState = -1;
    m_playSoundThread.csSound.Unlock();
    pThread = m_playSoundThread.pSoundThread;
    if(pThread)
    {
        HANDLE hThread = pThread->m_hThread;
        GetExitCodeThread(hThread, &dwState);
        if(STILL_ACTIVE==dwState)
        {
            WaitForSingleObject(hThread, dwWaits);
        }
    }    

    if(pThread)
    {
        HANDLE hThread = pThread->m_hThread;
        GetExitCodeThread(hThread, &dwState);
        if(STILL_ACTIVE==dwState)
        {
            WaitForSingleObject(hThread, dwWaits);
        }
    }
    pThread = m_workThreads.pCamPicThread;
    if(pThread)
    {
        HANDLE hThread = pThread->m_hThread;
        GetExitCodeThread(hThread, &dwState);
        if(STILL_ACTIVE==dwState)
        {
            WaitForSingleObject(hThread, dwWaits);
        }
    }
    
    memset(&m_workThreads, 0, sizeof(m_workThreads));
    KillTimer(1);

    DeletePicPixel(m_pIDPhoto);
    m_pIDPhoto=NULL;
    DeletePicPixel(m_pLivePic);
    m_pLivePic=NULL;
}

static void ImageThreshold(IplImage* img, CvHaarClassifierCascade* cascade, CvMemStorage* storage)
{
    static CvScalar colors[] = 
    {
        {{0,0,255}},
        {{0,128,255}},
        {{0,255,255}},
        {{0,255,0}},
        {{255,128,0}},
        {{255,255,0}},
        {{255,0,0}},
        {{255,0,255}}
    };
    
    double scale = 1.3;
    IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    IplImage* small_img = cvCreateImage( 
                            cvSize( cvRound(img->width/scale),
                                    cvRound(img->height/scale)),
                                    8, 1 );

    cvCvtColor( img, gray, CV_BGR2GRAY );      //CvtColor色彩空间转换,从彩色图像img 输出灰度图像 gray
    cvResize( gray, small_img, CV_INTER_LINEAR );      //函数 cvResize 将图像gray  改变尺寸得到与small_img  同样大小
    cvEqualizeHist( small_img, small_img );            //灰度图象直方图均衡化,该方法归一化图像亮度和增强对比度。
    cvClearMemStorage( storage );         //函数并不释放内存（仅清空内存）。
                            
    if( cascade)
    {
        //double t = (double)cvGetTickCount();            //该函数可用来测量函数/用户代码的执行时间
        CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 4, 0,
                                            cvSize(35, 35));//末参数为检测窗口的最小尺寸
        for( int i=0; i<(faces ? faces->total : 0); i++ )
        {
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i);
//             CvPoint center;
//             int radius;
//             center.x = cvRound((r->x + r->width*0.5)*scale);
//             center.y = cvRound((r->y + r->height*0.5)*scale);
//             radius = cvRound((r->width + r->height)*0.25*scale);
//             cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );

            CvPoint pt1;
            pt1.x = cvRound(r->x*scale);
            pt1.y = cvRound(r->y*scale);
            CvPoint pt2;
            pt2.x = cvRound((r->x+r->width)*scale);
            pt2.y = cvRound((r->y+r->height)*scale);
            
            cvRectangle(img, pt1, pt2, colors[i%8], 2, 8, 0);
            //为何之前需要变量scale将图像先变小再检测，是否起到向下采样的作用？
        }
    }
    cvReleaseImage(&gray);
    cvReleaseImage(&small_img);
}

// 定时刷新摄像头画面
// UINT CCameraDlg::ShowCamPicThread(void *param)
// {
//     const char* cascade_name = "haarcascade_frontalface_alt2.xml";
//     CvMemStorage* storage = NULL;
//     CvHaarClassifierCascade* cascade = NULL;
// 
//     CString strCascFile = MakeModuleFilePath(cascade_name);
//     cascade = (CvHaarClassifierCascade*)cvLoad( strCascFile, 0, 0, 0 );
//     storage = cvCreateMemStorage(0);
//     
//     CCameraDlg* pDlg = (CCameraDlg*)param;
//     CWnd* pWnd = pDlg->GetCtrlCamPic();
//     pWnd->ShowWindow(SW_SHOW);
//     CDC* pDC = pWnd->GetDC();
//     RECT rectRight;
//     pWnd->GetClientRect(&rectRight);
//     bool bFaceOrTicket=false;
//     while(!pDlg->UpdateCamPicDone())
//     {
//         pDlg->LockCam();
//         CCameraDS* pCurCam = pDlg->GetCurCamera(bFaceOrTicket);
//         //获取一帧
//         IplImage *pFrame = pCurCam->QueryFrame();
//         if(!pFrame)
//         {
//             Sleep(DEF_WAIT_FACE);
//             continue;
//         }        
// 
//         //可以添加一个函数用于图像处理，将处理后的结果显示在右边窗口
//         if(bFaceOrTicket)
//         {
//             cvFlip(pFrame, NULL, 1);
//             ImageThreshold(pFrame, cascade, storage);    //增加人脸识别图框
//         }
// 
//         CvvImage* pImg = pDlg->SaveFramePic(pFrame);
//         pDlg->UnLockCam();
//         //右边显示窗口，显示处理后的图像
//         //imgRight.CopyOf(pFrame);
//         //SetRect(pDlg->rectR, 0,0,pFrame->width,pFrame->height);
//         pImg->DrawToHDC(pDC->GetSafeHdc(), &rectRight);
//         
//         if(pDlg->UpdateCamPicDone()){break;}
//         Sleep(DEF_WAIT_FACE);
//     }	
//     
//     cvReleaseHaarClassifierCascade(&cascade);
//     cvReleaseMemStorage(&storage);
//     return 0 ;
// }

// H.Z.XIN 2016/01/13 使用MindVision3.0M工业摄像头来拍人
// 定时刷新摄像头画面
UINT CCameraDlg::ShowCamPicThread(void *param)
{
    const char* cascade_name = "haarcascade_frontalface_alt2.xml";
    CvMemStorage* storage = NULL;
    CvHaarClassifierCascade* cascade = NULL;
    
    CString strCascFile = MakeModuleFilePath(cascade_name);
    cascade = (CvHaarClassifierCascade*)cvLoad( strCascFile, 0, 0, 0 );
    storage = cvCreateMemStorage(0);
    
    CCameraDlg* pDlg = (CCameraDlg*)param;
    CWnd* pWnd = pDlg->GetCtrlCamPic();
    pWnd->ShowWindow(SW_SHOW);
    CDC* pDC = pWnd->GetDC();
    CRect rectRight;
    pWnd->GetClientRect(&rectRight);
    bool bFaceOrTicket=false;
    while(!pDlg->UpdateCamPicDone())
    {
        CvvImage* pImg = NULL;
        IplImage* pFrame = NULL;
        pDlg->LockCam();
        void* pCurCam = pDlg->GetCurCamera(bFaceOrTicket);        
        if(false==bFaceOrTicket)
        {
            //拍车票
            //获取一帧
            CCameraDS* pCamTick = (CCameraDS*)pCurCam;
            pFrame=pCamTick->QueryFrame();            
        }
        else
        {
            //拍人脸
            CMindVisionCamOper* pMinCam = (CMindVisionCamOper*)pCurCam;
            pFrame = pMinCam->QueryFrame(800);            
        }
        if(!pFrame)
        {
            pDlg->UnLockCam();
            Sleep(DEF_WAIT_FACE*2);
            continue;
        }        
        if(bFaceOrTicket)
        {
            cvFlip(pFrame, NULL, 0);
            cvFlip(pFrame, NULL, 1);
            //增加人脸识别图框            
            ImageThreshold(pFrame, cascade, storage);
        }
        pImg = pDlg->SaveFramePic(pFrame);
        pDlg->UnLockCam();
        //右边显示窗口，显示处理后的图像
        //imgRight.CopyOf(pFrame);
        //SetRect(pDlg->rectR, 0,0,pFrame->width,pFrame->height);
        pImg->DrawToHDC(pDC->GetSafeHdc(), &rectRight);
        
        if(pDlg->UpdateCamPicDone()){break;}
        Sleep(DEF_WAIT_FACE);
    }	
    
    cvReleaseHaarClassifierCascade(&cascade);
    cvReleaseMemStorage(&storage);
    return 0 ;
}


//////////////////////////////////////////////////////

void CHandlerReadIDCard::Do( void* pData)
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    if(pDlg->ReadIDCardInfo())
    {        
        GetMgr()->disPatch(&g_handlerFaceCmp, pData);
    }else{
        Sleep(DEF_WAIT_POLLING);
    }
}

void CHandlerReadIDCard::PreDo( void* pData )
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    pDlg->ResetIDCardInfo();
}

// 读取身份证信息
bool CCameraDlg::ReadIDCardInfo()
{
    char	picName[64]={0};
    CString str;
    //const char* buf=NULL;
    int nRet = m_idCardReader.ReadCard();
    //及早退出
    //if(WorkDone()){return false;}
    if(nRet!=0)
    {
        //UpdatePromptInfo("请将身份证放在感应区上面!");
        UpdateIconCheck(0, -1);
        return false;
    }
    
    unsigned char* pbyText  = m_idCardReader.GetBaseText();
    unsigned char* pbyPhoto = m_idCardReader.GetPhoto();

    if( !m_idTextDecoder.Decode(pbyText))
    {
        UpdateIconCheck(0, 0);
        return false;
    }
    
    if( !WritePhotoFile(m_idTextDecoder.GetID(), pbyPhoto))
    {
        UpdatePromptInfo("提取身份证照片失败!", CLR_ERROR);
        UpdateIconCheck(0, 0);
        return false;
    }
    ShowIDCardInfo(&m_idTextDecoder);
    //UpdateIconCheck(0, 1);
    //logfile(LOG_FILENAME, buf);
    return true;
}


// 写照片位图文件
bool CCameraDlg::WritePhotoFile( const char* pstrID, unsigned char* pbyPhoto )
{
    CString strRela;
    if(!pstrID || !pstrID[0])
    {
        strRela.Format("tempPic.wlt");
    }else{
        strRela.Format("%s.wlt", pstrID);
    }
    
    //生成身份证照片的全路径
    CString strPicDir = MakeModuleFilePath(DEF_IDPIC_DIR);
    CreateDirectory(strPicDir, NULL);
    CString strWltFileName;
    strWltFileName.Format("%s\\%s", DEF_IDPIC_DIR, strRela);
    strWltFileName = MakeModuleFilePath(strWltFileName);
    if(m_strLastIDPicPath.GetLength())
    {
        DeleteFile(m_strLastIDPicPath);
    }
    m_strLastIDPicPath=strWltFileName;
    m_strLastIDPicPath.Replace(".wlt", ".bmp");

    CFile file;
    if( !file.Open(strWltFileName, CFile::modeWrite|CFile::modeCreate))
    {
        return false;
    }
    file.Write(pbyPhoto, 1024);
    file.Close();
    bool bOk = (1==m_idCardReader.wlt2bmp(strWltFileName));
    return bOk;
}


static void ResizeImg( IplImage* pImgSrc, IplImage* pImgDst)
{
    // 读取图片的宽和高;
//     int w = pImgSrc->width;
//     int h = pImgSrc->height;    
    // 对图片 img 进行缩放，并存入到 SrcImage 中;
    cvResize( pImgSrc, pImgDst);
    
    // 重置 SrcImage 的 ROI 准备读入下一幅图片;
    cvResetImageROI( pImgDst );
}


// static void PaintImg( IplImage* img, CWnd* pWnd)
// {
//     CDC* pDC = pWnd->GetDC();        // 获得显示控件的 DC;
//     HDC hDC = pDC ->GetSafeHdc();                 // 获取 HDC(设备句柄) 来进行绘图操作;
//     
//     CRect rect;
//     pWnd->GetClientRect( &rect );
//     int rw = rect.right - rect.left;              // 求出图片控件的宽和高;
//     int rh = rect.bottom - rect.top;
//     int iw = img->width;                          // 读取图片的宽和高;
//     int ih = img->height;
//     int tx = (int)(rw - iw)/2;                    // 使图片的显示位置正好在控件的正中;
//     int ty = (int)(rh - ih)/2;
//     SetRect( rect, tx, ty, tx+iw, ty+ih );
//     
//     CvvImage cimg;
//     cimg.CopyOf( img );                            // 复制图片;
//     cimg.DrawToHDC( hDC, &rect );                  // 将图片绘制到显示控件的指定区域内;
//     
//     pWnd->ReleaseDC(pDC);    
// }
 
// //取屏幕像素转换成灰度图
// //x,y 为矩形原点坐标.Width,Height为矩形宽与高
// static void  GetPixelToGray(int x,int y,int Width,int Height, unsigned char *picPixel)   
// {
//     HDC hDC = GetDC(NULL);
//     CDC *pDC = CDC::FromHandle(hDC);//获取当前整个屏幕DC
//     CDC memDC;//内存DC
//     memDC.CreateCompatibleDC(pDC);
//     
//     CBitmap memBitmap, *oldmemBitmap;//建立和屏幕兼容的bitmap
//     memBitmap.CreateCompatibleBitmap(pDC, Width, Height);
//     oldmemBitmap = memDC.SelectObject(&memBitmap);//将memBitmap选入内存DC
//     memDC.BitBlt(0, 0, Width, Height, pDC, x, y, SRCCOPY);//复制屏幕图像到内存DC
//     //以下代码保存memDC中的位图到文件
//     BITMAP bmp;
//     memBitmap.GetBitmap(&bmp);//获得位图信息
// 
//     BITMAPINFOHEADER bih = {0};//位图信息头
//     bih.biBitCount = bmp.bmBitsPixel;//每个像素字节大小
//     bih.biCompression = BI_RGB;
//     bih.biHeight = bmp.bmHeight;//高度
//     bih.biPlanes = 1;
//     bih.biSize = sizeof(BITMAPINFOHEADER);
//     bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//图像数据大小
//     bih.biWidth = bmp.bmWidth;//宽度
//     
// //     BITMAPFILEHEADER bfh = {0};//位图文件头
// //     bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
// //     bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//文件总的大小
// //     bfh.bfType = (WORD)0x4d42;
// 
//     byte * pBytes = new byte[bmp.bmWidthBytes * bmp.bmHeight];//申请内存保存位图数据
//     //获取位图数据,注意图像数据p是从左下->右下->左上->右上
//     GetDIBits(memDC.m_hDC, (HBITMAP) memBitmap.m_hObject, 0, Height, pBytes,(LPBITMAPINFO)&bih, DIB_RGB_COLORS);
// 
//     //转换一个矩形区域的RGB图至灰度图片.存入picPixel,转换公式:Gray = R*0.299 + G*0.587 + B*0.114,直接省去小数部分
//     int xx,yy;
//     long colorIndex;
//     for(yy=0;yy<Height;yy++) 
//     {
//         for(xx=0;xx<Width;xx++)
//         {
//             colorIndex=bmp.bmWidthBytes * yy + 4* xx;     //RGB像素在(xx,yy)坐标颜色.从左下角开始为原点.下左->下右->上左->上右
//             picPixel[(Height-1-yy)*Width+xx]=(unsigned char)(0.299*pBytes[colorIndex+2]+0.587*pBytes[colorIndex+1]+0.114*pBytes[colorIndex]);
//             //picPixel[(Height-1-yy)*Width+xx]=(unsigned char)((306*pBytes[colorIndex+2]+601*pBytes[colorIndex+1]+117*pBytes[colorIndex])>>10);            
//         }
//     }
//     delete[] pBytes;
//     memDC.SelectObject(oldmemBitmap);
//     ReleaseDC(NULL,hDC);
// }


static void GetGrayPixel(IplImage* pImgPic, int nWidth, int nHeight, unsigned char* pOutPixel)
{    
    IplImage* pImgOper = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, IMAGE_CHANNELS);
    IplImage* pImgGray = cvCreateImage(cvSize(nWidth, nHeight), IPL_DEPTH_8U, 1);
    ResizeImg(pImgPic, pImgOper);
    cvCvtColor(pImgOper, pImgGray, CV_RGB2GRAY);
    cvReleaseImage(&pImgOper);

    memcpy(pOutPixel, pImgGray->imageData, pImgGray->imageSize);
    cvReleaseImage(&pImgGray);
}


// 展示身份证信息, 并且保存身份证照片数据
//
void CCameraDlg::ShowIDCardInfo( CIDBaseTextDecoder* pidDecoder)
{
    int ret=-1;    
    IplImage* pImgPic = cvCreateImage( cvSize(PHOTO_WIDTH, PHOTO_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS ); //创建图像模板;
    IplImage* ipl = cvLoadImage(m_strLastIDPicPath); // 读取图片、缓存到一个局部变量ipl中;  
    if( !ipl )                      // 判断是否成功载入图片;
    {
        TRACE("***读入身份证照片失败!\n");
        cvReleaseImage( &pImgPic);
        return;
    }
    //if( pImgPic )                       // 对上一幅显示的图片数据清零;  
    //    cvZero( pImgPic );  
    
    ResizeImg( ipl, pImgPic);           // 对读入的图片进行缩放，使其宽或高最大值者小于等于640*480，再复制到 TheImage 中;  
    //PaintImg( pImgPic, &m_stPicLeft);   // 调用显示图片函数; 
    m_stIDShow.SetIDDraw(&m_bmpIDBack, pidDecoder);
    m_stIDShow.SetIDPhoto(pImgPic);
    m_stIDShow.RedrawWindow();
    
    //picPixel[0].width=PHOTO_WIDTH;    
    //picPixel[0].height=PHOTO_HEIGHT;
    GetGrayPixel(pImgPic, m_pIDPhoto->width, m_pIDPhoto->height, &m_pIDPhoto->pixel[0]);

    /////////////////////////////////////////////////
    //转换一个矩形区域0的RGB图至灰度图片.存入grayPixel,转换公式:Gray = R*0.299 + G*0.587 + B*0.114,直接省去小数部分
//     CRect rect;
//     m_stIDShow.GetWindowRect(&rect);
//     GetPixelToGray(rect.left, rect.top, picPixel[0].width, picPixel[0].height, &picPixel[0].pixel[0]);

    cvReleaseImage( &ipl);
    cvReleaseImage( &pImgPic);
}


void CHandlerTicketCheck::Do( void* pData)
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    pDlg->UpdatePromptInfo("请将车票平放在验票口!");
    pDlg->SwitchCamera(false);
    pDlg->FaceCamPlay(false);
    pDlg->AsyncPlayVoice(VOICE_PLACE_TIC);
    pDlg->FlashAndLight(0);    

    bool bCmp = false;
    CString strQrCode;
    for(int i=0; i<pDlg->GetMaxTicketChkTimes(); i++)
    {
        bCmp = pDlg->DoCheckTicket(strQrCode);
        if(bCmp){break;}
        Sleep(DEF_WAIT_FACE*2);
    }
    pDlg->SwitchLight(0, false);

    if(bCmp && pDlg->ValidTicket(strQrCode))
    {
        pDlg->UpdateIconCheck(1, 1);
        CString strCode;
        strCode.Format("二维码内容: %s", strQrCode);
        pDlg->UpdatePromptInfo(strCode);
        Sleep(0.2*1000); //
        GetMgr()->disPatch(&g_handlerOpenGate, pData);
    }else{
        pDlg->UpdateIconCheck(1, 0);
        pDlg->UpdatePromptInfo("车票验证不通过！\n请走人工验票通道。", CLR_ERROR);
        pDlg->AsyncPlayVoice(VOICE_FAIL_TICKCHK);
        GetMgr()->disPatch(&g_handlerException, pData);
    }
}

void CHandlerTicketCheck::PreDo( void* pData )
{
    //CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
}

void CHandlerFaceCmp::PreDo( void* pData )
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    pDlg->FlashLight(1, 200);
}

void CHandlerFaceCmp::Do( void* pData)
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    bool bCmp = false;
    float fScore=0.0;
    pDlg->SwitchLight(2, true);
    pDlg->SwitchLight(3, true);    
    pDlg->AsyncPlayVoice(VOICE_VIEW_CAM);

    for(int i=0; i<pDlg->GetMaxFaceCmpTimes(); i++)
    {
        bCmp = pDlg->DoFaceCmp(&fScore);
        if(bCmp){break;}
        Sleep(DEF_WAIT_FACE);
    }

    pDlg->SwitchLight(2, false);
    pDlg->SwitchLight(3, false);
    if(bCmp)
    {
        pDlg->UpdateIconCheck(0, 1);
        pDlg->UpdatePromptInfo("人脸识别通过！");        
        Sleep(0.2*1000); //! for test
        GetMgr()->disPatch(&g_handlerTicketCheck, pData);
    }else{
        pDlg->UpdateIconCheck(0, 0);
        pDlg->UpdatePromptInfo("人脸识别不通过！", CLR_ERROR);
        pDlg->AsyncPlayVoice(VOICE_FAIL_FACECMP);
        GetMgr()->disPatch(&g_handlerException, pData);
    }

    WriteFaceCmpLog(pData, fScore);
}

void CHandlerFaceCmp::WriteFaceCmpLog( void* pData, float fScore)
{    
#ifdef JOHN_DEBUG
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    CIDBaseTextDecoder* pIDInfo = pDlg->GetIDCardInfo();
    const char* pstrName = pIDInfo->GetName();
    CString strID = pIDInfo->GetID();
    strID.SetAt(13, 'X');
    strID.SetAt(15, 'X');
    CString strLog;
    strLog.Format("%20s%24s 相似度%g%%", 
                  pstrName,
                  strID,
                  fScore
                  );
    logfile(FACECMPLOG_FILENAME, strLog);
#endif//JOHN_DEBUG
}

bool CCameraDlg::DoFaceCmp( float* pfScore/*=NULL*/)
{
    //转换一个矩形区域1的RGB图至灰度图片.存入grayPixel,
    //转换公式:Gray = R*0.299 + G*0.587 + B*0.114,直接省去小数部分    
//     picPixel[1].width=IMAGE_WIDTH;
//     picPixel[1].height=IMAGE_HEIGHT;
//     CRect rect;
//     m_stCam.GetWindowRect(&rect);
//     GetPixelToGray(rect.left,rect.top,picPixel[1].width,picPixel[1].height,&picPixel[1].pixel[0]);

    //picPixel[1].width=IMAGE_WIDTH;    
    //picPixel[1].height=IMAGE_HEIGHT;    
    LockCam();
    GetGrayPixel(m_swCamerPic.m_curFrameImg.GetImage(), m_pLivePic->width, m_pLivePic->height, &m_pLivePic->pixel[0]);
    UnLockCam();

    float fScore=0.0;
    //两个图片的比对。 并且保存特征.
    if(MyHandle!=NULL)
    {
        //! temp for test
        fScore=TestCompare1V1(MyHandle, m_pIDPhoto, m_pLivePic, m_cfgInfo.fInitFaceCmpRate, 1);
    }
    fScore=fScore*100.0;
    CString strScore;
    strScore.Format("人脸识别相似度：%.2f%%", fScore);
    UpdatePromptInfo(strScore);
    TRACE1("***人脸相似度: %s\n", strScore);
    
    //输出人脸相似度
    if(pfScore){ *pfScore=fScore;}
    return (fScore>=(float)m_cfgInfo.nReqFaceCmpScore);
}


//解码二维码
// static bool DecodeQRData( const char* input, char *output)
// {
//     int i=0;
//     char buf[64]={0};
//     while(true)
//     {
//         if(*(input+i)==':')  //解码火车票号
//         {
//             buf[0]= (*(input+i+1)&0x0f)*10+(*(input+i+2)&0x0f)+'A';
//             memcpy(&buf[1],input+i+3,6);
//             buf[7]=0;
//             strcpy(output,buf);
//             return true;
//         }
//         else
//         {
//             i++;
//         }
//     }
//     return false;
// }

// 检查车票（二维码识别）
bool CCameraDlg::DoCheckTicket( CString& strOutQrCode )
{
    /* create a reader */  
    zbar_image_scanner_t *scanner = zbar_image_scanner_create();  
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    /* obtain image data */  
    int width = 0, height = 0;  
    const void *raw = NULL;  

    width =m_swCamerPic.m_curFrameImg.Width();
    height=m_swCamerPic.m_curFrameImg.Height();
    IplImage* pImg = m_swCamerPic.m_curFrameImg.GetImage();
    if(!pImg){return false;}
    IplImage* imgGray = cvCreateImage( cvSize(pImg->width,pImg->height), 8, 1 );    
    cvCvtColor( pImg, imgGray, CV_BGR2GRAY);
    raw = imgGray->imageDataOrigin;
    ///////////////////////////////////////
 
    /* wrap image data */  
    zbar_image_t *image = zbar_image_create();  
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);  
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);  
    
    /* scan the image for barcodes */  
    int n = zbar_scan_image(scanner, image); //n == 0 is failed  
    /* extract results */  
    //WCHAR *d;  
    int nd = 0;  
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);  
    for(; symbol; symbol = zbar_symbol_next(symbol))   
    {  
        /* do something useful with results */  
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);  
        //memcpy(info.type,zbar_get_symbol_name(typ),sizeof(info.type) - 1);  
        
        const char *data = zbar_symbol_get_data(symbol);  
        unsigned int len = zbar_symbol_get_data_length(symbol);  

        TRACE1("\n***QrCode = %s\n", data);
        strOutQrCode=data;
        break;
//         nd = utf8_to_utf16((BYTE *)data,len,NULL,0); //utf-8 转成 unicode 需要多少字节  
//         d = (WCHAR *)malloc((nd * 2) + 2);  
//         utf8_to_utf16((BYTE *)data,len,d,nd); //utf-8 转成 unicode  
//         memset(info.data,0,sizeof(info.data));  
//         WideCharToMultiByte(CP_ACP,0,d,nd,(CHAR *)info.data,sizeof(info.data) - 1,NULL,NULL); //unicode 转 ascii  
//         free(d);  
    }  

    cvReleaseImage(&imgGray);
    zbar_image_scanner_destroy(scanner);
    return (n!=0);
}

// bool CCameraDlg::SaveTicketPic()
// {
//     if(m_swCamerPic.m_strLastTickFile.GetLength())
//     {
//         DeleteFile(m_swCamerPic.m_strLastTickFile);
//     }
//     CTime tmNow = CTime::GetCurrentTime();
//     CString strFileName;
//     strFileName.Format("%s\\", DEF_IDPIC_DIR);
//     strFileName += tmNow.Format("ticket%H_%M_%S.png");
//     strFileName = MakeModuleFilePath(strFileName);
// 
//     bool bRet = m_swCamerPic.m_curFrameImg.Save(strFileName);
// 
//     m_swCamerPic.m_strLastTickFile = bRet?strFileName:"";
//     return bRet;
// }

// 验证通过，允许通行
void CHandlerOpenGate::Do( void* pData )
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    pDlg->LetGo();
    pDlg->SwitchCamera(true);
    pDlg->FaceCamPlay(true);
    Sleep(0.8*1000); //todo
    GetMgr()->disPatch(&g_handlerReadIDCard, pData);
}

void CCameraDlg::LetGo()
{
    m_gateBoardOper.OpenGate(1);
    AsyncPlayVoice(VOICE_PASS);
    UpdatePromptInfo("验证成功。\n祝你旅途愉快！", CLR_SUCCESS);
}

// 让独立线程来播放声音
void CCameraDlg::AsyncPlayVoice(const char* pstrVoice)
{
    CSingleLock locker(&m_playSoundThread.csSound, TRUE);
    m_playSoundThread.nState=100;
    m_playSoundThread.sSoundFile=pstrVoice;
}

void CCameraDlg::PlayVoice(const char* pstrVoice)
{
    CString strPath;
    strPath.Format("voice\\%s", pstrVoice);
    PlaySound(strPath, NULL, SND_FILENAME|SND_SYNC);
}

void CHandlerException::Do( void* pData)
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    pDlg->SwitchCamera(true);
    pDlg->FaceCamPlay(true);    
    Sleep(1.5*1000); //todo
    GetMgr()->disPatch(&g_handlerReadIDCard, pData);
}

UINT CCameraDlg::OnNcHitTest(CPoint point) 
{		
	UINT nRet = CDialog::OnNcHitTest(point);
#ifdef _DEBUG
    short nState = GetAsyncKeyState(VK_LBUTTON);
    if((nState&0X8000) && (nRet==HTCLIENT))
    {
        return HTCAPTION;
    }
#endif
    return nRet;
}

// 右键双击使程序退出
//
void CCameraDlg::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
    DoExit();
	PostQuitMessage(0);
	CDialog::OnRButtonDblClk(nFlags, point);
}

// 更新检验结果图标
// nIcon 的取值：0身份证， 其它表示车票
// nState的取值：-1不显示，0失败，1 成功
void CCameraDlg::UpdateIconCheck( int nIcon, int nState )
{
    CStatic* pWndStatic = (0==nIcon) ? &m_stIDCheck : &m_stTicketCheck;
    if(nState<0)
    {
        pWndStatic->SetBitmap((HBITMAP)NULL);
    }
    else if(nState==0)
    {
        pWndStatic->SetBitmap((HBITMAP)m_bmpWrong);
    }
    else
    {
        pWndStatic->SetBitmap((HBITMAP)m_bmpRight);
    }
    pWndStatic->ShowWindow(nState>=0);
}

void CCameraDlg::OnBtnTest() 
{
    DoExit();
    CCameraDlg::OnCancel();	
}

bool CCameraDlg::TryInitGateBoard()
{
    bool bRet = m_gateBoard.SerialOpen(m_cfgInfo.nGateBoardCOM, 9600);
    if(bRet)
    {
        m_gateBoardOper.TurnoffAllLight();
    }
    return bRet;
}

void CCameraDlg::FlashAndLight( int iLight )
{
    FlashLight(iLight, 200);
    SwitchLight(iLight, true);
}

void CCameraDlg::FlashLight( int iLight, DWORD dwMs)
{
    const int nCnt = 2;
    for(int i=0; i<nCnt; i++)   
    {
        m_gateBoardOper.SwitchLight(iLight, true);
        Sleep(dwMs);
        m_gateBoardOper.SwitchLight(iLight, false);
    }
}

void CCameraDlg::SwitchLight( int iLight, bool bOpen)
{
    m_gateBoardOper.SwitchLight(iLight, bOpen);
}

void CCameraDlg::UpdateShowTime()
{   
    CTime tmNow = CTime::GetCurrentTime();
    CString strNowTime = tmNow.Format("%Y年%m月%d日 %H:%M:%S");    
    m_stTimeShow.SetWindowText(strNowTime);
}

void CCameraDlg::FaceCamPlay( bool bPlay)
{
    if(bPlay)
    {
        m_camFace.Play();
    }else{
        m_camFace.Pause();
    }
}

// 用来验证车票合法性
bool CCameraDlg::ValidTicket( CString strQrCode )
{
    return true;
}

void CCameraDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
    SetCursorPos(120,120);
    ShowCursor(TRUE);
	CDialog::OnRButtonDown(nFlags, point);
}
