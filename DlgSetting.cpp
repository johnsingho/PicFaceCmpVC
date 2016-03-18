// DlgSetting.cpp : implementation file
//

#include "stdafx.h"
#include "camera.h"
#include "DlgSetting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSetting dialog


CDlgSetting::CDlgSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetting::IDD, pParent)
    , m_pCfgInfo(NULL)
{
	//{{AFX_DATA_INIT(CDlgSetting)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetting)
	DDX_Control(pDX, IDC_EDIT_REQFACESCORE, m_edReqFaceScore);
	DDX_Control(pDX, IDC_EDIT_IDCOM, m_edIDCom);
	DDX_Control(pDX, IDC_EDIT_CHKTICKMAX, m_edCheckTickMax);
	DDX_Control(pDX, IDC_EDIT_CHKFACEMAX, m_edCheckFaceMax);
	DDX_Control(pDX, IDC_EDIT_CAMTICK, m_edCamTick);
	DDX_Control(pDX, IDC_EDIT_CAMFACE, m_edCamFace);
	DDX_Control(pDX, IDC_EDIT_BOARD, m_edBoardCOM);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSetting, CDialog)
	//{{AFX_MSG_MAP(CDlgSetting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CDlgSetting message handlers



void CDlgSetting::OnOK() 
{
	StoreCfg();
	
	CDialog::OnOK();
}

CScrollBar* CDlgSetting::GetScrollBarCtrl(int nBar) const
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::GetScrollBarCtrl(nBar);
}

BOOL CDlgSetting::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ShowCfg(m_pCfgInfo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSetting::ShowCfg( stConfigInfo* pCfgInfo )
{
    CString str;
    
    str.Format("%d", pCfgInfo->nIDReaderCOM);
	m_edIDCom.SetWindowText(str);
    str.Format("%d", pCfgInfo->nReqFaceCmpScore);
	m_edReqFaceScore.SetWindowText(str);
    str.Format("%d", pCfgInfo->nMaxRetryTicketChk);
	m_edCheckTickMax.SetWindowText(str);
    str.Format("%d", pCfgInfo->nMaxRetryFaceCmp);
	m_edCheckFaceMax.SetWindowText(str);
    str.Format("%d", pCfgInfo->nCamTicketID);
	m_edCamTick.SetWindowText(str);
    str.Format("%d", pCfgInfo->nCamFaceID);
	m_edCamFace.SetWindowText(str);
    str.Format("%d", pCfgInfo->nGateBoardCOM);
	m_edBoardCOM.SetWindowText(str);
}

void CDlgSetting::StoreCfg()
{
    CString strVal;

    m_edIDCom.GetWindowText(strVal);
    m_pCfgInfo->nIDReaderCOM=atoi(strVal);
    m_edReqFaceScore.GetWindowText(strVal);
    m_pCfgInfo->nReqFaceCmpScore=atoi(strVal);
    m_edCheckTickMax.GetWindowText(strVal);
    m_pCfgInfo->nMaxRetryTicketChk=atoi(strVal);
    m_edCheckFaceMax.GetWindowText(strVal);
    m_pCfgInfo->nMaxRetryFaceCmp=atoi(strVal);
    m_edCamTick.GetWindowText(strVal);
    m_pCfgInfo->nCamTicketID=atoi(strVal);
    m_edCamFace.GetWindowText(strVal);
    m_pCfgInfo->nCamFaceID=atoi(strVal);
    m_edBoardCOM.GetWindowText(strVal);
    m_pCfgInfo->nGateBoardCOM=atoi(strVal);

    m_pCfgInfo->bReWrite=true;
}
