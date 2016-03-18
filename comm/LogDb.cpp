// LogDb.cpp: implementation of the CLogDb class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogDb.h"
#include "misc.h"


//照片日志数据库
//#define DEF_LOG_DB_FILENAME "shebao.dat"
#define MAX_LOG_DB_SIZE     (500*1024*1024)
#define DEF_LOG_DB_PWD      "We_Want^girls*"
#define DEF_LOG_DB_NAME     "TBL_LOG"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogDb::CLogDb()
 : m_db(NULL)
 , m_MaxFileSizeBytes(MAX_LOG_DB_SIZE)
{
}

CLogDb::~CLogDb()
{
    CloseDB();

}


void CLogDb::SetInitFileName( const char* pstrPathLogDB )
{
	m_strLastFileName=pstrPathLogDB;
}

//是否还能够写这个文件？
//
bool CLogDb::CanWriteFile( const char* pstrPathLogDB)
{
    __int64 nFileSize = GetFileSizeByName(pstrPathLogDB);
    return (-1==nFileSize || (nFileSize>=0 && nFileSize<m_MaxFileSizeBytes));
}

bool CLogDb::TryOpen()
{
    bool bWrite = CanWriteFile(m_strLastFileName);
    if(m_db)
    {
        if(bWrite)
        {
            return true;
        }else{
            CloseDB();
            m_strLastFileName = GenNextFileName(m_strLastFileName);
        }
    }
    else if(!bWrite) 
    {
           m_strLastFileName = GenNextFileName(m_strLastFileName);
    }
        
    return OpenDB(m_strLastFileName, DEF_LOG_DB_PWD);
}

CString CLogDb::GenNextFileName( const char* pstrPathLogDB)
{
    CString strOldPath(pstrPathLogDB);
    CString strPre;
    CString strPos;
    int iDot = strOldPath.ReverseFind('.');
    if(iDot>0)
    {
        strPre = strOldPath.Mid(0, iDot);
        int iEnd=strPre.GetLength()-1;
        for(;iEnd>=0; iEnd--)
        {
            if(!isdigit(strPre[iEnd]))
            {
                break;
            }
        }
        strPre = strPre.Mid(0,iEnd+1);
        strPos=strOldPath.Mid(iDot);
    }else if(iDot<0){
        strPre=strOldPath;
    }
    CString strNewPath;
    char szBuf[16];
    int i=0;
    while(1)
    {
        strNewPath=strPre;
        sprintf(szBuf, "%08d", i++);
        strNewPath += szBuf;
        strNewPath += strPos;
        if(CanWriteFile(strNewPath))
        {
            break; //找到了
        }
    }
    return strNewPath;
}

bool CLogDb::OpenDB( CString strFileName, CString strPass)
{
	TRACE1("CLogDb::OpenDB, opening %s\n", strFileName);
    if (sqlite3_open(strFileName, &m_db) != SQLITE_OK)
    {
        return false;
    }
    CString strPSstr;
    strPSstr.Format("PRAGMA key ='%s'", strPass);
	if(sqlite3_exec(m_db, strPSstr, NULL, NULL, NULL) != SQLITE_OK)
    {
        TRACE("CLogDb::OpenDB, 密码不正确\n");
		CloseDB();
        return false;
	}
    if(!CreateTable())
    {
        CloseDB();
        return false;
    }
    return true;
}

void CLogDb::CloseDB()
{
	if(m_db)
    {
        sqlite3_close(m_db);
        m_db=NULL;
    }
}

bool CLogDb::CreateTable()
{	
    if(ExistTable(DEF_LOG_DB_NAME))
    {
        return true;
    }
    CString strCreate;
    strCreate.Format("CREATE TABLE %s([ID] INTEGER PRIMARY KEY,[name] VARCHAR(20),[IDNo] CHAR(20),"
                     "[UpdateTime] TimeStamp NOT NULL DEFAULT(datetime('now','localtime')),[IDPhoto] BLOB, [LivePhoto] BLOB,"
                     "[RecRate] FLOAT DEFAULT(0.0))",
                     DEF_LOG_DB_NAME);
    char* perrMsg=NULL;
    int res = sqlite3_exec(m_db, strCreate, NULL, NULL, &perrMsg);
    sqlite3_free(perrMsg);
    return (SQLITE_OK == res);
}

bool CLogDb::ExistTable( const char* pstrTab )
{
    CString strSql;
    strSql.Format("SELECT COUNT(*) FROM sqlite_master where type='table' and name='%s'", pstrTab);
    char** ppazResult=NULL;
    char* perrMsg=NULL;
    int nRow=0;
    int nColumn=0;    
    int result = sqlite3_get_table( m_db, strSql, &ppazResult, &nRow, &nColumn, &perrMsg);  
    bool bOk = false;
    if( SQLITE_OK == result )
    {
        bOk = atoi(ppazResult[1])>0;
    }else{
        sqlite3_free(perrMsg);
    }
    sqlite3_free_table( ppazResult); 

    return bOk;
}

// 插入图片记录
//
bool CLogDb::InsertRec( const char* pstrName, const char* pstrID, float fCurScore, const char* pstrLastIDPicPath, const char* pstrFileName )
{
    if(!LoadBlobData(pstrLastIDPicPath, pstrFileName))
    {
        return false;
    }
	CString strIns;
    strIns.Format("INSERT INTO %s(name,IDNo,RecRate,IDPhoto,LivePhoto) VALUES('%s','%s',%g,?,?)", 
                  DEF_LOG_DB_NAME,
                  pstrName, pstrID,
                  fCurScore);

    sqlite3_stmt* pStmt = NULL;  
    int nRtn = sqlite3_prepare_v2(m_db, strIns, -1, &pStmt, NULL);  
    if(nRtn != SQLITE_OK)  
    {
        return false;
    }
    
    int iCol=1;
    nRtn = sqlite3_bind_blob(pStmt, iCol++, m_blobIDPhoto.GetData(), m_blobIDPhoto.GetDataSize(), NULL);
    if(nRtn != SQLITE_OK)  
    {
        return false;
    }
    nRtn = sqlite3_bind_blob(pStmt, iCol++, m_blobLivePhoto.GetData(), m_blobLivePhoto.GetDataSize(), NULL);
    if(nRtn != SQLITE_OK)  
    {
        return false;
    }
    nRtn = sqlite3_step(pStmt);  
    nRtn = sqlite3_finalize(pStmt);
    return (SQLITE_OK==nRtn);
}  

bool CLogDb::LoadBlobData( const char* pstrLastIDPicPath, const char* pstrFileName )
{
	CFile fileID;
    if(!fileID.Open(pstrLastIDPicPath, CFile::modeRead|CFile::shareDenyNone, NULL))
    {
        TRACE("***LoadBlobData打开身份证照片失败\n");
        return false;
    }
    CFile filePhoto;
    if(!filePhoto.Open(pstrFileName, CFile::modeRead|CFile::shareDenyNone, NULL))
    {
        TRACE("***LoadBlobData打开现场照片失败\n");
        return false;
    }

    DWORD dwBytes = fileID.GetLength();
    m_blobIDPhoto.ReSize(dwBytes);
    fileID.Read(m_blobIDPhoto.GetData(), dwBytes);

    dwBytes = filePhoto.GetLength();
    m_blobLivePhoto.ReSize(dwBytes);
    filePhoto.Read(m_blobLivePhoto.GetData(), dwBytes);
    return true;
}


