// LogDBExp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Sqlite/sqlite3.h"
#include <windows.h>

////////////////////////////////////////////////////

// 以下两个要与comm\LogDb.cpp的同步
#define DEF_LOG_DB_PWD      "We_Want^girls*"
#define DEF_LOG_DB_NAME     "TBL_LOG"

#define DEF_EXTRACT_OUT_DIR  "LogDB_Extract"

////////////////////////////////////////////////////

static const char* MakeModuleFileName( const char* pstrFileName)
{
    const int nBufLen = 1024;
    static char strBuf[nBufLen]={0};
    DWORD dwLen = GetModuleFileName( GetModuleHandle(NULL), strBuf, nBufLen);
    
    char* pstrSlash = strrchr(strBuf, '\\');
    if( pstrSlash)
    {
        ++pstrSlash;
        *pstrSlash=NULL;
    }
    strcat(strBuf, pstrFileName);
    return strBuf;
}


bool ExtractData( const char* pstrDbFile );
bool WriteOutFiles( const char* pstrIDNo, const char* pstrUpdateTime, int idPhotoLen, const void* pIDPhotoData, int livePhotoLen, const void* pLivePhotoData );

////////////////////////////////////////////////////////////////

void ShowUse( const char* pstrExe) 
{
	printf("使用方法：\n");
    printf("\t%s db文件\n", pstrExe);
}


// 加密样本图片导出工具
// H.Z.XIN 2016-03-07
//
int main(int argc, char* argv[])
{
	if(argc < 2)
    {
        ShowUse(argv[0]);
        return -1;
    }
    ExtractData(argv[1]);
	return 0;
}


char g_strTarDir[1024]={0};

bool ExtractImgs( sqlite3* db ) 
{
    char szBuf[256]={0};
    sprintf(szBuf, "SELECT name,IDNo,UpdateTime,IDPhoto,LivePhoto,RecRate FROM %s ORDER BY IDNo", DEF_LOG_DB_NAME);
    //sprintf(szBuf, "SELECT * FROM %s", DEF_LOG_DB_NAME);
    sqlite3_stmt *statement=NULL;
    if(sqlite3_prepare_v2(db, szBuf, -1, &statement, NULL) != SQLITE_OK)
    {
        printf("查询失败: %s\n", sqlite3_errmsg(db));
        return false;
    }

    strcpy(g_strTarDir, MakeModuleFileName(DEF_EXTRACT_OUT_DIR));
    CreateDirectory(g_strTarDir, NULL);
    while(SQLITE_ROW == sqlite3_step(statement))
    {
        const char* pstrName       = (const char*)sqlite3_column_text(statement, 0); 
        const char* pstrIDNo       = (const char*)sqlite3_column_text(statement, 1); 
        const char* pstrUpdateTime = (const char*)sqlite3_column_text(statement, 2); 
        
        int idPhotoLen = sqlite3_column_bytes(statement, 3);
        const void* pIDPhotoData = sqlite3_column_blob(statement, 3);
        int livePhotoLen = sqlite3_column_bytes(statement, 4);
        const void* pLivePhotoData = sqlite3_column_blob(statement, 4);

        //识别率
        double dblRecRate          = sqlite3_column_double(statement, 5);
        //printf("pstrUpdateTime=%s, idPhoto size=%d, livePhoto size=%d\n", 
        //        pstrUpdateTime, idPhotoLen, livePhotoLen);
        WriteOutFiles(pstrIDNo, pstrUpdateTime, idPhotoLen, pIDPhotoData, livePhotoLen, pLivePhotoData);
    }
    return true;
}

bool ExtractData( const char* pstrDbFile ) 
{
    sqlite3* db=NULL;
    bool bRet=false;
    char szBuf[300]={0};
    const char* pstrSql=NULL;
    int rc = sqlite3_open(pstrDbFile, &db);
    if( rc ){
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        goto ED_EXIT;
    }
    
    //check ps
    sprintf(szBuf,"PRAGMA key ='%s'", DEF_LOG_DB_PWD);
    if (sqlite3_exec(db, (const char*)szBuf, NULL, NULL, NULL) != SQLITE_OK){
        printf("Password not match!\n");
        goto ED_EXIT;
    };

    ExtractImgs(db);

ED_EXIT:
    sqlite3_close(db);
    return bRet;
}


bool WriteOutFiles( const char* pstrIDNo, const char* pstrUpdateTime, int idPhotoLen, const void* pIDPhotoData, int livePhotoLen, const void* pLivePhotoData )
{
    char szNameBuf[1024];
    char* pstrFileName =szNameBuf;
    char szTime[32];
    int   nYear=0;
    int nMon=0;
    int nDay=0;
    int nHour=0;
    int nMin=0;
    int nSec=0;
    //2016-03-08 11:24:11
    sscanf(pstrUpdateTime, "%d-%d-%d %d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec);
    sprintf(szTime, "%04d%02d%02d%02d%02d%02d", nYear, nMon, nDay, nHour, nMin, nSec);
    int iPos = sprintf(szNameBuf, "%s\\%s_%s", g_strTarDir, pstrIDNo, szTime);

    //write ID photo
    sprintf(szNameBuf+iPos, ".bmp");
    HANDLE hIDFile = CreateFile(pstrFileName, FILE_GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if(hIDFile==INVALID_HANDLE_VALUE)
    {
        printf("Create file error: %s\n", pstrFileName);
        return false;
    }else{
        DWORD dwWrite=0;
        WriteFile(hIDFile, pIDPhotoData, idPhotoLen, &dwWrite, NULL);
        CloseHandle(hIDFile);    
    }
    
    //write live photo
    sprintf(szNameBuf+iPos, "Live.jpg");
    HANDLE hLiveFile = CreateFile(pstrFileName, FILE_GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if(hLiveFile==INVALID_HANDLE_VALUE)
    {
        printf("Create file error: %s\n", pstrFileName);
        CloseHandle(hIDFile);
        return false;
    }else{
        DWORD dwWrite=0;
        WriteFile(hLiveFile, pLivePhotoData, livePhotoLen, &dwWrite, NULL);
        CloseHandle(hLiveFile);    
    }
    return true;
}