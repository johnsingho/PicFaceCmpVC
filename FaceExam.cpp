// FaceExam.cpp: implementation of the CFaceExam class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "camera.h"
#include "FaceExam.h"
#include "cameraDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////


CJobManager::CJobManager()
{
    
}

CJobManager::~CJobManager()
{
}

void CJobManager::doWork( void* pData)
{
    m_pCurHandler->Do(pData);
}

void CJobManager::disPatch( CHandlerBase* pHandler, void* pData )
{
    m_pCurHandler=pHandler;
    pHandler->Bind(this);
    pHandler->PreDo(pData);
}

