// FaceExam.h: interface for the CFaceExam class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FACEEXAM_H__99A5EF94_64BC_432F_A269_4AC93E027188__INCLUDED_)
#define AFX_FACEEXAM_H__99A5EF94_64BC_432F_A269_4AC93E027188__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HeCompare.h"
#include <vector>

class CJobManager;

class CHandlerBase
{
public:
    CHandlerBase():m_pMgr(NULL)
    {}
    inline void Bind(CJobManager* pMgr){m_pMgr=pMgr;}
    inline CJobManager* GetMgr(){return m_pMgr;}
    virtual ~CHandlerBase(){}
    virtual void PreDo(void* pData){}
    virtual void Do(void* pData)=0;
private:
    CJobManager* m_pMgr;
};



//¹¤Í·
class CJobManager
{
public:
    CJobManager();
    ~CJobManager();
    void doWork(void* pData);
    void disPatch(CHandlerBase* pHandler, void* pData);
private:
    CHandlerBase* m_pCurHandler;  
};




#endif // !defined(AFX_FACEEXAM_H__99A5EF94_64BC_432F_A269_4AC93E027188__INCLUDED_)
