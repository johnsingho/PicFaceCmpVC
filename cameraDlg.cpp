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

// �������֤������ʱ������Զ˿�
#define MAX_TRY_COM         (20)
// Ĭ�������ļ�
#define DEF_CONFIG_FILE     "profile.ini"
// Ĭ�����֤��ƬĿ¼(����Ŀ¼��)
#define DEF_IDPIC_DIR       "pic"
// ������ѯ���(ms)
#define DEF_WAIT_POLLING    (0.2*1000)
// ��������ͷ��ѯ���(ms)
#define DEF_WAIT_FACE       (40)

// ��Ʊʶ�����Դ���
#define DEF_MAX_RETRY_TICKETCHK (25)
// ����ʶ�����Դ���
#define DEF_MAX_RETRY_FACECMP   (10)
// Ĭ�ϵ�����ʶ����ֵ
#define DEF_REQ_FACECMP_SCORE   (60)
// Ĭ�ϵĵư塢բ�ſ�����COM��
#define DEF_GATE_BOARD_COM      (7)
// ����2016-01-29��SDK���Եĳ�ʼʶ����(0~999)
#define DEF_INIT_FACECMP_RATE   (322)

#define IMAGE_CHANNELS          (3)

// from Compare.cpp
//extern PICPIXEL picPixel[2];
extern HW_HANDLE MyHandle;

using namespace zbar;
#pragma comment(lib, "libzbar-0.lib")

#define DEF_SYS_NAME "����ʶ��������վϵͳ"

//բ�����ư�
#include "comm/sw_lock03.h"

//բ�����Դ���
#include "DialogLockTest.h"
CDialogLockTest *pDlgLockTest=NULL;


#define CLR_TITLE     RGB(255, 69, 0)
#define CLR_INFO      RGB(11, 0, 0)
#define CLR_ERROR     RGB(220, 20, 60)
#define CLR_SUCCESS   RGB(0,128, 0)
#define CLR_INFO_READ RGB(20, 100, 20)

//��Ƶ�ļ���
#define VOICE_INIT_OK       "InitOk.wav"
#define VOICE_INIT_FAIL     "InitFail.wav"
#define VOICE_PASS          "pass.wav"
//#define VOICE_FAIL          "fail.wav"
#define VOICE_FAIL_FACECMP  "FaceCmpFail.wav"
#define VOICE_FAIL_TICKCHK  "TicketChkFail.wav"
#define VOICE_VIEW_CAM      "viewcam.wav"
#define VOICE_PLACE_TIC     "placeTicket.wav"

//��־�ļ���
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
    UpdatePromptInfo("ϵͳ���ڳ�ʼ�������Ժ�...");    
    
    AfxBeginThread(InitSoundThread, &m_playSoundThread);
    // ��ʼ������
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

//�������֤��Ϣ��ʾ
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
    str.Format("��ӭʹ��\n%s", DEF_SYS_NAME);
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
    //��ʼ�����֤��ȡģ��
    bool bInitID = pDlg->TryInitIDCardReader();    

    //��ʼ������ʶ���
    bool bInitFace=pDlg->TryInitFaceCmp();
    //��ʼ����������ͷ
    bool bInitCam =pDlg->TryInitCameras();
    //��ʼ���ư塢բ��
    bool bInitGateBoard = pDlg->TryInitGateBoard();

    if(bInitID && bInitFace && bInitCam && bInitGateBoard)
    {
        CString str;
        str.Format("��ӭʹ��\n%s", DEF_SYS_NAME);
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
    // �����˳���ť
    GetDlgItem(IDC_BTN_TEST)->ShowWindow(SW_SHOW);
#endif
    
#ifdef _DEBUG
    ::SetWindowPos(AfxGetMainWnd()->m_hWnd,HWND_TOP,-1,-1,-1,-1,SWP_NOSIZE);    
#else
     //����ǰ����ʾ,���Ͻ���ʾ
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
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"����");
    CFont fontTitle; 
    fontTitle.CreateFont(30,15,0,0,FW_BOLD,
        FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"����");
    CFont fontTitleBig; 
    fontTitleBig.CreateFont(36,18,0,0,FW_BOLD,
        FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"����");
    CFont fontTime; 
    fontTime.CreateFont(20,10,0,0,FW_NORMAL,
        FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,"����");

    m_stLeftInfo.MySetFont(&fontInfo);
    m_stLeftInfoTitle.MySetFont(&fontTitle);
    m_stRightInfoTitle.MySetFont(&fontTitleBig);

    // ʱ����ʾ
    m_stTimeShow.MoveWindow(740,20,250,40,TRUE);
    m_stTimeShow.SetTextClr(CLR_INFO);
    m_stTimeShow.MySetFont(&fontTime);
    
    //------------------------------------------------
    m_stLeftInfoTitle.SetWindowText("����ָ����");
    CString strInfo;
    strInfo += "1���뽫���֤���ڸ�Ӧ������\n";
    strInfo += "2����������������ͷ\n";
    strInfo += "3���뽫��Ʊƽ��������Ʊ��\n";
    m_stLeftInfo.SetWindowText(strInfo);

    m_stIDShow.ModifyStyle(0, SS_CENTERIMAGE, 0);
    m_stTicketShow.ModifyStyle(0, SS_CENTERIMAGE, 0);
    m_stIDCheck.ModifyStyle(0, SS_CENTERIMAGE, 0);
    m_stTicketCheck.ModifyStyle(0, SS_CENTERIMAGE, 0);

    //m_stIDCheck.SetBitmap((HBITMAP)m_bmpWrong);
    //m_stTicketCheck.SetBitmap((HBITMAP)m_bmpWrong);
    ResetIDCardInfo();
}


// ��ʼ���ڲ�����
void CCameraDlg::InitVars()
{
    m_pIDPhoto = CreatePicPixel(PHOTO_WIDTH, PHOTO_HEIGHT);
    m_pLivePic = CreatePicPixel(IMAGE_WIDTH, IMAGE_HEIGHT);
    
    SwitchCamera(true);
    m_gateBoardOper.Bind(&m_gateBoard);
}

// bFaceOrTicket
// true Ϊ����
// false Ϊ��Ʊ
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

// ������ʾ��Ϣ
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
        const char* pstrErr = "û���ҵ����֤������\n��˿��޷���!";
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


//��ʼ������ʶ���
bool CCameraDlg::TryInitFaceCmp()
{
    int nRet = initialCompare();
    if(S_OK!=nRet)
    {
        const char* pstrErr = "����ʶ��ģ���ʼ��ʧ�ܣ�";
        logfile(SYSTEMLOG_FILENAME, pstrErr);
        UpdatePromptInfo(pstrErr);
        return false;
    }
    return true;
}

//��ʼ����������ͷ
bool CCameraDlg::TryInitCameras()
{
    CString strInfo;
    bool bCamFace=false;
    bool bCamTicket=false;
    
    if(0==CCameraDS::CameraCount())
    {
        strInfo="û�������κ�����ͷ";
        logfile(SYSTEMLOG_FILENAME, strInfo);
        UpdatePromptInfo(strInfo);
        return false;
    }

//     bCamFace=m_camFace.OpenCamera(m_cfgInfo.nCamFaceID,0,640,480);
//     if( !bCamFace)
//     {
//         strInfo.Format("��ʼ����������ͷʧ��(ID=%d)", m_cfgInfo.nCamFaceID);
//     }
    bCamFace = m_camFace.Init();
    if(!bCamFace)
    {
        strInfo.Format("��ʼ����������ͷSDKʧ��");
    }
    else
    {
        bCamFace=m_camFace.InitCamera();
        if(!bCamFace)
        {
            strInfo.Format("���ӵ���������ͷʧ�ܣ�");
        }
    }

    bCamTicket=m_camTicket.OpenCamera(m_cfgInfo.nCamTicketID,0,640,480);
    if( !bCamTicket)
    {
        char strBuf[128];
        sprintf(strBuf, "��ʼ����Ʊ����ͷʧ��(ID=%d)��", m_cfgInfo.nCamTicketID);
        if(strInfo.GetLength())
        {
            strInfo += "��";
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

// �������߳�
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
        UpdatePromptInfo("�����������߳�ʧ�ܣ�");
    }
    m_workThreads.pMainThread=pThread;
}

void CCameraDlg::StartCamThread()
{
    CWinThread* pThread = AfxBeginThread(ShowCamPicThread, this);
    if(!pThread)
    {
        UpdatePromptInfo("��������ͷ�߳�ʧ�ܣ�");
    }
    m_workThreads.pCamPicThread=pThread;
}


// ����
void CCameraDlg::DoExit()
{
    m_workThreads.bMainCanExit=true;
    m_workThreads.bCamPicCanExit=true;

    m_gateBoard.SerialClose();        

    CWinThread* pThread = m_workThreads.pMainThread;
    DWORD dwState=0;
    DWORD dwWaits = DEF_WAIT_POLLING*5;

    //���������߳�
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

    cvCvtColor( img, gray, CV_BGR2GRAY );      //CvtColorɫ�ʿռ�ת��,�Ӳ�ɫͼ��img ����Ҷ�ͼ�� gray
    cvResize( gray, small_img, CV_INTER_LINEAR );      //���� cvResize ��ͼ��gray  �ı�ߴ�õ���small_img  ͬ����С
    cvEqualizeHist( small_img, small_img );            //�Ҷ�ͼ��ֱ��ͼ���⻯,�÷�����һ��ͼ�����Ⱥ���ǿ�Աȶȡ�
    cvClearMemStorage( storage );         //���������ͷ��ڴ棨������ڴ棩��
                            
    if( cascade)
    {
        //double t = (double)cvGetTickCount();            //�ú�����������������/�û������ִ��ʱ��
        CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 4, 0,
                                            cvSize(35, 35));//ĩ����Ϊ��ⴰ�ڵ���С�ߴ�
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
            //Ϊ��֮ǰ��Ҫ����scale��ͼ���ȱ�С�ټ�⣬�Ƿ������²��������ã�
        }
    }
    cvReleaseImage(&gray);
    cvReleaseImage(&small_img);
}

// ��ʱˢ������ͷ����
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
//         //��ȡһ֡
//         IplImage *pFrame = pCurCam->QueryFrame();
//         if(!pFrame)
//         {
//             Sleep(DEF_WAIT_FACE);
//             continue;
//         }        
// 
//         //�������һ����������ͼ�����������Ľ����ʾ���ұߴ���
//         if(bFaceOrTicket)
//         {
//             cvFlip(pFrame, NULL, 1);
//             ImageThreshold(pFrame, cascade, storage);    //��������ʶ��ͼ��
//         }
// 
//         CvvImage* pImg = pDlg->SaveFramePic(pFrame);
//         pDlg->UnLockCam();
//         //�ұ���ʾ���ڣ���ʾ������ͼ��
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

// H.Z.XIN 2016/01/13 ʹ��MindVision3.0M��ҵ����ͷ������
// ��ʱˢ������ͷ����
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
            //�ĳ�Ʊ
            //��ȡһ֡
            CCameraDS* pCamTick = (CCameraDS*)pCurCam;
            pFrame=pCamTick->QueryFrame();            
        }
        else
        {
            //������
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
            //��������ʶ��ͼ��            
            ImageThreshold(pFrame, cascade, storage);
        }
        pImg = pDlg->SaveFramePic(pFrame);
        pDlg->UnLockCam();
        //�ұ���ʾ���ڣ���ʾ������ͼ��
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

// ��ȡ���֤��Ϣ
bool CCameraDlg::ReadIDCardInfo()
{
    char	picName[64]={0};
    CString str;
    //const char* buf=NULL;
    int nRet = m_idCardReader.ReadCard();
    //�����˳�
    //if(WorkDone()){return false;}
    if(nRet!=0)
    {
        //UpdatePromptInfo("�뽫���֤���ڸ�Ӧ������!");
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
        UpdatePromptInfo("��ȡ���֤��Ƭʧ��!", CLR_ERROR);
        UpdateIconCheck(0, 0);
        return false;
    }
    ShowIDCardInfo(&m_idTextDecoder);
    //UpdateIconCheck(0, 1);
    //logfile(LOG_FILENAME, buf);
    return true;
}


// д��Ƭλͼ�ļ�
bool CCameraDlg::WritePhotoFile( const char* pstrID, unsigned char* pbyPhoto )
{
    CString strRela;
    if(!pstrID || !pstrID[0])
    {
        strRela.Format("tempPic.wlt");
    }else{
        strRela.Format("%s.wlt", pstrID);
    }
    
    //�������֤��Ƭ��ȫ·��
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
    // ��ȡͼƬ�Ŀ�͸�;
//     int w = pImgSrc->width;
//     int h = pImgSrc->height;    
    // ��ͼƬ img �������ţ������뵽 SrcImage ��;
    cvResize( pImgSrc, pImgDst);
    
    // ���� SrcImage �� ROI ׼��������һ��ͼƬ;
    cvResetImageROI( pImgDst );
}


// static void PaintImg( IplImage* img, CWnd* pWnd)
// {
//     CDC* pDC = pWnd->GetDC();        // �����ʾ�ؼ��� DC;
//     HDC hDC = pDC ->GetSafeHdc();                 // ��ȡ HDC(�豸���) �����л�ͼ����;
//     
//     CRect rect;
//     pWnd->GetClientRect( &rect );
//     int rw = rect.right - rect.left;              // ���ͼƬ�ؼ��Ŀ�͸�;
//     int rh = rect.bottom - rect.top;
//     int iw = img->width;                          // ��ȡͼƬ�Ŀ�͸�;
//     int ih = img->height;
//     int tx = (int)(rw - iw)/2;                    // ʹͼƬ����ʾλ�������ڿؼ�������;
//     int ty = (int)(rh - ih)/2;
//     SetRect( rect, tx, ty, tx+iw, ty+ih );
//     
//     CvvImage cimg;
//     cimg.CopyOf( img );                            // ����ͼƬ;
//     cimg.DrawToHDC( hDC, &rect );                  // ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������;
//     
//     pWnd->ReleaseDC(pDC);    
// }
 
// //ȡ��Ļ����ת���ɻҶ�ͼ
// //x,y Ϊ����ԭ������.Width,HeightΪ���ο����
// static void  GetPixelToGray(int x,int y,int Width,int Height, unsigned char *picPixel)   
// {
//     HDC hDC = GetDC(NULL);
//     CDC *pDC = CDC::FromHandle(hDC);//��ȡ��ǰ������ĻDC
//     CDC memDC;//�ڴ�DC
//     memDC.CreateCompatibleDC(pDC);
//     
//     CBitmap memBitmap, *oldmemBitmap;//��������Ļ���ݵ�bitmap
//     memBitmap.CreateCompatibleBitmap(pDC, Width, Height);
//     oldmemBitmap = memDC.SelectObject(&memBitmap);//��memBitmapѡ���ڴ�DC
//     memDC.BitBlt(0, 0, Width, Height, pDC, x, y, SRCCOPY);//������Ļͼ���ڴ�DC
//     //���´��뱣��memDC�е�λͼ���ļ�
//     BITMAP bmp;
//     memBitmap.GetBitmap(&bmp);//���λͼ��Ϣ
// 
//     BITMAPINFOHEADER bih = {0};//λͼ��Ϣͷ
//     bih.biBitCount = bmp.bmBitsPixel;//ÿ�������ֽڴ�С
//     bih.biCompression = BI_RGB;
//     bih.biHeight = bmp.bmHeight;//�߶�
//     bih.biPlanes = 1;
//     bih.biSize = sizeof(BITMAPINFOHEADER);
//     bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//ͼ�����ݴ�С
//     bih.biWidth = bmp.bmWidth;//���
//     
// //     BITMAPFILEHEADER bfh = {0};//λͼ�ļ�ͷ
// //     bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//��λͼ���ݵ�ƫ����
// //     bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//�ļ��ܵĴ�С
// //     bfh.bfType = (WORD)0x4d42;
// 
//     byte * pBytes = new byte[bmp.bmWidthBytes * bmp.bmHeight];//�����ڴ汣��λͼ����
//     //��ȡλͼ����,ע��ͼ������p�Ǵ�����->����->����->����
//     GetDIBits(memDC.m_hDC, (HBITMAP) memBitmap.m_hObject, 0, Height, pBytes,(LPBITMAPINFO)&bih, DIB_RGB_COLORS);
// 
//     //ת��һ�����������RGBͼ���Ҷ�ͼƬ.����picPixel,ת����ʽ:Gray = R*0.299 + G*0.587 + B*0.114,ֱ��ʡȥС������
//     int xx,yy;
//     long colorIndex;
//     for(yy=0;yy<Height;yy++) 
//     {
//         for(xx=0;xx<Width;xx++)
//         {
//             colorIndex=bmp.bmWidthBytes * yy + 4* xx;     //RGB������(xx,yy)������ɫ.�����½ǿ�ʼΪԭ��.����->����->����->����
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


// չʾ���֤��Ϣ, ���ұ������֤��Ƭ����
//
void CCameraDlg::ShowIDCardInfo( CIDBaseTextDecoder* pidDecoder)
{
    int ret=-1;    
    IplImage* pImgPic = cvCreateImage( cvSize(PHOTO_WIDTH, PHOTO_HEIGHT), IPL_DEPTH_8U, IMAGE_CHANNELS ); //����ͼ��ģ��;
    IplImage* ipl = cvLoadImage(m_strLastIDPicPath); // ��ȡͼƬ�����浽һ���ֲ�����ipl��;  
    if( !ipl )                      // �ж��Ƿ�ɹ�����ͼƬ;
    {
        TRACE("***�������֤��Ƭʧ��!\n");
        cvReleaseImage( &pImgPic);
        return;
    }
    //if( pImgPic )                       // ����һ����ʾ��ͼƬ��������;  
    //    cvZero( pImgPic );  
    
    ResizeImg( ipl, pImgPic);           // �Զ����ͼƬ�������ţ�ʹ��������ֵ��С�ڵ���640*480���ٸ��Ƶ� TheImage ��;  
    //PaintImg( pImgPic, &m_stPicLeft);   // ������ʾͼƬ����; 
    m_stIDShow.SetIDDraw(&m_bmpIDBack, pidDecoder);
    m_stIDShow.SetIDPhoto(pImgPic);
    m_stIDShow.RedrawWindow();
    
    //picPixel[0].width=PHOTO_WIDTH;    
    //picPixel[0].height=PHOTO_HEIGHT;
    GetGrayPixel(pImgPic, m_pIDPhoto->width, m_pIDPhoto->height, &m_pIDPhoto->pixel[0]);

    /////////////////////////////////////////////////
    //ת��һ����������0��RGBͼ���Ҷ�ͼƬ.����grayPixel,ת����ʽ:Gray = R*0.299 + G*0.587 + B*0.114,ֱ��ʡȥС������
//     CRect rect;
//     m_stIDShow.GetWindowRect(&rect);
//     GetPixelToGray(rect.left, rect.top, picPixel[0].width, picPixel[0].height, &picPixel[0].pixel[0]);

    cvReleaseImage( &ipl);
    cvReleaseImage( &pImgPic);
}


void CHandlerTicketCheck::Do( void* pData)
{
    CCameraDlg* pDlg = static_cast<CCameraDlg*>(pData);
    pDlg->UpdatePromptInfo("�뽫��Ʊƽ������Ʊ��!");
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
        strCode.Format("��ά������: %s", strQrCode);
        pDlg->UpdatePromptInfo(strCode);
        Sleep(0.2*1000); //
        GetMgr()->disPatch(&g_handlerOpenGate, pData);
    }else{
        pDlg->UpdateIconCheck(1, 0);
        pDlg->UpdatePromptInfo("��Ʊ��֤��ͨ����\n�����˹���Ʊͨ����", CLR_ERROR);
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
        pDlg->UpdatePromptInfo("����ʶ��ͨ����");        
        Sleep(0.2*1000); //! for test
        GetMgr()->disPatch(&g_handlerTicketCheck, pData);
    }else{
        pDlg->UpdateIconCheck(0, 0);
        pDlg->UpdatePromptInfo("����ʶ��ͨ����", CLR_ERROR);
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
    strLog.Format("%20s%24s ���ƶ�%g%%", 
                  pstrName,
                  strID,
                  fScore
                  );
    logfile(FACECMPLOG_FILENAME, strLog);
#endif//JOHN_DEBUG
}

bool CCameraDlg::DoFaceCmp( float* pfScore/*=NULL*/)
{
    //ת��һ����������1��RGBͼ���Ҷ�ͼƬ.����grayPixel,
    //ת����ʽ:Gray = R*0.299 + G*0.587 + B*0.114,ֱ��ʡȥС������    
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
    //����ͼƬ�ıȶԡ� ���ұ�������.
    if(MyHandle!=NULL)
    {
        //! temp for test
        fScore=TestCompare1V1(MyHandle, m_pIDPhoto, m_pLivePic, m_cfgInfo.fInitFaceCmpRate, 1);
    }
    fScore=fScore*100.0;
    CString strScore;
    strScore.Format("����ʶ�����ƶȣ�%.2f%%", fScore);
    UpdatePromptInfo(strScore);
    TRACE1("***�������ƶ�: %s\n", strScore);
    
    //����������ƶ�
    if(pfScore){ *pfScore=fScore;}
    return (fScore>=(float)m_cfgInfo.nReqFaceCmpScore);
}


//�����ά��
// static bool DecodeQRData( const char* input, char *output)
// {
//     int i=0;
//     char buf[64]={0};
//     while(true)
//     {
//         if(*(input+i)==':')  //�����Ʊ��
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

// ��鳵Ʊ����ά��ʶ��
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
//         nd = utf8_to_utf16((BYTE *)data,len,NULL,0); //utf-8 ת�� unicode ��Ҫ�����ֽ�  
//         d = (WCHAR *)malloc((nd * 2) + 2);  
//         utf8_to_utf16((BYTE *)data,len,d,nd); //utf-8 ת�� unicode  
//         memset(info.data,0,sizeof(info.data));  
//         WideCharToMultiByte(CP_ACP,0,d,nd,(CHAR *)info.data,sizeof(info.data) - 1,NULL,NULL); //unicode ת ascii  
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

// ��֤ͨ��������ͨ��
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
    UpdatePromptInfo("��֤�ɹ���\nף����;��죡", CLR_SUCCESS);
}

// �ö����߳�����������
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

// �Ҽ�˫��ʹ�����˳�
//
void CCameraDlg::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
    DoExit();
	PostQuitMessage(0);
	CDialog::OnRButtonDblClk(nFlags, point);
}

// ���¼�����ͼ��
// nIcon ��ȡֵ��0���֤�� ������ʾ��Ʊ
// nState��ȡֵ��-1����ʾ��0ʧ�ܣ�1 �ɹ�
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
    CString strNowTime = tmNow.Format("%Y��%m��%d�� %H:%M:%S");    
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

// ������֤��Ʊ�Ϸ���
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
