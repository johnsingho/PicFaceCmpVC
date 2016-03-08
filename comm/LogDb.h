// LogDb.h: interface for the CLogDb class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGDB_H__67DCA26C_E32A_4906_A7A8_913A58E98343__INCLUDED_)
#define AFX_LOGDB_H__67DCA26C_E32A_4906_A7A8_913A58E98343__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Sqlite/sqlite3.h"

//照片日志数据库
#define DEF_LOG_DB_FILENAME "shebao.dat"


class BlobFileInfo
{
public:
    BlobFileInfo():pBuffer(NULL),nBufSize(0)
    {
    }
    ~BlobFileInfo(){ Clear();}
    bool ReSize(unsigned int nNewSize)
    {
        if(nNewSize>nBufSize){
            Clear();                
            BYTE* pBufNew = new BYTE[nNewSize];
            if(!pBufNew){ return false;}
            pBuffer = pBufNew;
            nBufSize=nNewSize;
        }
        return true;
    }
    void Clear() 
    {
        delete[] pBuffer;
        pBuffer=NULL;
        nBufSize=0;
    }
    inline BYTE* GetData(){return pBuffer;}
    inline unsigned int GetDataSize(){return nBufSize;}
private:
    BYTE* pBuffer;
    unsigned int nBufSize;
};


class CLogDb  
{
public:
	CLogDb();
	~CLogDb();

    void CloseDB();
    bool InitOpen( const char* pstrPathLogDB);
    CString GenNextFileName( const char* pstrPathLogDB );
    bool CanWriteFile( const char* pstrPathLogDB );
    bool OpenDB( CString strFileName, CString strPass);
    bool CreateTable();
    bool ExistTable( const char* pstrTab );
    bool InsertRec( const char* pstrName, const char* pstrID, const char* pstrLastIDPicPath, const char* pstrFileName );
private:    
    bool LoadBlobData( const char* pstrLastIDPicPath, const char* pstrFileName );
private:
    sqlite3* m_db;
    unsigned int m_MaxFileSizeBytes;

    BlobFileInfo m_blobIDPhoto;
    BlobFileInfo m_blobLivePhoto;
};

#endif // !defined(AFX_LOGDB_H__67DCA26C_E32A_4906_A7A8_913A58E98343__INCLUDED_)
