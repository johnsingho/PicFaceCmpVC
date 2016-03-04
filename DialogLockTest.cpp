// DialogLockTest.cpp : implementation file
//

#include "stdafx.h"
#include "camera.h"
#include "DialogLockTest.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogLockTest dialog


CDialogLockTest::CDialogLockTest(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogLockTest::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogLockTest)
	m_port = 0;    
	//}}AFX_DATA_INIT
    m_pLocker=NULL;
}


void CDialogLockTest::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogLockTest)
	DDX_Text(pDX, IDC_EDIT1, m_port);
	DDV_MinMaxInt(pDX, m_port, 1, 100);
    DDX_Control(pDX, IDC_EDIT_INFO, m_editInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogLockTest, CDialog)
	//{{AFX_MSG_MAP(CDialogLockTest)
	ON_BN_CLICKED(IDC_BUTTON_GATEON, OnButtonGateon)
	ON_BN_CLICKED(IDC_BUTTON_GATEOFF, OnButtonGateoff)
	ON_BN_CLICKED(IDC_BUTTON_LED1ON, OnButtonLed1on)
	ON_BN_CLICKED(IDC_BUTTON_LED1OFF, OnButtonLed1off)
	ON_BN_CLICKED(IDC_BUTTON_LED2ON, OnButtonLed2on)
	ON_BN_CLICKED(IDC_BUTTON_LED2OFF, OnButtonLed2off)
	ON_BN_CLICKED(IDC_BUTTON_LED3ON, OnButtonLed3on)
	ON_BN_CLICKED(IDC_BUTTON_LED3OFF, OnButtonLed3off)
	ON_BN_CLICKED(IDC_BUTTON_LED4ON, OnButtonLed4on)
	ON_BN_CLICKED(IDC_BUTTON_LED4OFF, OnButtonLed4off)
	ON_BN_CLICKED(IDC_BUTTON_LED5ON, OnButtonLed5on)
	ON_BN_CLICKED(IDC_BUTTON_LED5OFF, OnButtonLed5off)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogLockTest message handlers


void CDialogLockTest::OnButtonGateon() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_GATE,CMDDATA_GATEON2)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_GATE, CMDDATA_GATEON2);
        PromptInfo(str);
    }
}

void CDialogLockTest::OnButtonGateoff() 
{
    PromptInfo("");
    if(m_pLocker->SendCommand(LOCK_CMD_GATE,CMDDATA_GATEON1)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_GATE, CMDDATA_GATEON1);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed1on() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED1ON)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED1ON);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed1off() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED1OFF)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED1OFF);
        PromptInfo(str);
    }
}

void CDialogLockTest::OnButtonLed2on() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED2ON)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED2ON);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed2off() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED2OFF)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED2OFF);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed3on() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED3ON)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED3ON);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed3off() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED3OFF)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED3OFF);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed4on() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED4ON)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED4ON);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed4off() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED4OFF)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED4OFF);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed5on() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED5ON)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED5ON);
        PromptInfo(str);
    }
}


void CDialogLockTest::OnButtonLed5off() 
{
    PromptInfo("");
	if(m_pLocker->SendCommand(LOCK_CMD_LED,CMDDATA_LED5OFF)<0){
        CString str;
        str.Format("***cmd error: cmd=%X, data=%X\n", LOCK_CMD_LED, CMDDATA_LED5OFF);
        PromptInfo(str);
    }
}

void CDialogLockTest::PromptInfo(const char* pstr)
{
    m_editInfo.SetWindowText(pstr);
}

void CDialogLockTest::OnButton2() 
{
	UpdateData(TRUE);
	if (m_pLocker->SerialOK())
		return;
	if (!m_pLocker->SerialOpen(m_port))
		AfxMessageBox("´ò¿ª´®¿ÚÊ§°Ü£¡");
}

BOOL CDialogLockTest::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_port = 1;
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
