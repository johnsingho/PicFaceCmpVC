// camera.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "camera.h"
#include "cameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCameraApp

BEGIN_MESSAGE_MAP(CCameraApp, CWinApp)
	//{{AFX_MSG_MAP(CCameraApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCameraApp construction

CCameraApp::CCameraApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCameraApp object

CCameraApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCameraApp initialization

BOOL CCameraApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

    // 已经启动过一个一模一样程序了
    HANDLE	hMutex = CreateMutex(NULL, true, "_FACECAMERA_SANWIK_HZX_2015_");    
    DWORD	dwLastError = GetLastError();    
    if(dwLastError == ERROR_ALREADY_EXISTS)
    {
        MessageBox(NULL, "人脸识别通关程序已经启动过，无法重复启动！", "提示", MB_OK|MB_ICONWARNING);
        return FALSE;
    }

	CCameraDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
    CloseHandle(hMutex);
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
