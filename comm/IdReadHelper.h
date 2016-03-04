#pragma once

#include "serial.h"


// for 
typedef int (WINAPI *FGetBmp)(char *wltfile,int intf);


//////////////////////////////////////////////////////////////
//
// 身份证读取辅助类
// History:
//         H.Z.XIN 2015-12-22 create
//
//////////////////////////////////////////////////////////////
class CIdReadHelper
{
public:
	CIdReadHelper(void);
	void ResetData();
	~CIdReadHelper(void);
	bool SerialOpen(int nPort, int nBaudRate=115200);
	void SerialClose();
	
	int SAM_GetState();
	int SAM_Reset();
	int SAM_FindCard();
	int SAM_SelectCard();
	int SAM_ReadBaseMsg();
	int SAM_ReadExtraMsg();
	int ReadCard();
	int SetBaudRate(int nBaud);

	//取读取到的固定信息
	unsigned char* GetBaseText(){return pucBaseText;}
	//取读取的图片数据
	unsigned char* GetPhoto(){return pucPhoto;}
    
    //转换wlt文件为bmp文件
    int wlt2bmp(const char* filename);

    inline bool IsOpened(){return NULL!=mSerial.Handle();}
    inline bool IsReadOk(){return m_bLastRead;}
private:
	int SendCommand(unsigned char cmd, unsigned char para, unsigned char &sw1, unsigned char &sw2, unsigned char &sw3, int timeout);

	bool LoadWltRs_dll();
	void UnloadWltRs_dll();	
	
private:
	unsigned char mRecvData[2048];

	unsigned char pucIIN[4]; //证/卡芯片管理号
	unsigned char pucSN[8];  //证/卡芯片序列号
	unsigned char pucBaseText[260]; //固定信息文本最多256个字节
	int pucBaseTextLen;
	unsigned char pucPhoto[1024];   //固定信息照片最多1024个字节
	int pucPhotoLen;
	unsigned char pucExtra[72];     //追加信息最多70字节

	Serial mSerial;

	HMODULE m_hDllWltRS;
	FGetBmp m_pfnGetBmp;
    bool    m_bLastRead;   //上一次读卡是否成功
};


// //! ???
// #pragma pack(1)
// typedef struct tagIDCardData{
// 	char Name[32];			//姓名       
// 	char Sex[4];			//性别
// 	char Nation[6];			//名族
// 	char Born[18];			//出生日期
// 	char Address[72];		//住址
// 	char IDCardNo[38];		//身份证号
// 	char GrantDept[32];		//发证机关
// 	char UserLifeBegin[18]; //有效开始日期
// 	char UserLifeEnd[18];	//有效截止日期
// 	char reserved[38];		//保留
// }IDCardData;
// #pragma pack()


// 解释固定信息
class CIDBaseTextDecoder
{
public:
	CIDBaseTextDecoder()
	{
        m_pBaseText=NULL;
		memset(m_strName, 0, sizeof(m_strName));
		memset(m_strSex, 0, sizeof(m_strSex));
		memset(m_strEthnic, 0, sizeof(m_strEthnic));
		memset(m_strBirthDay, 0, sizeof(m_strBirthDay));
		memset(m_strAddress, 0, sizeof(m_strAddress));
		memset(m_strID, 0, sizeof(m_strID));
		memset(m_strDepartment, 0, sizeof(m_strDepartment));
		memset(m_strExpireBegin, 0, sizeof(m_strExpireBegin));
		memset(m_strExpireEnd, 0, sizeof(m_strExpireEnd));
		memset(m_strAddition, 0, sizeof(m_strAddition));
	}

	bool Decode(const unsigned char* pBaseText);
	inline const char* GetName(){ return m_strName;}
	inline const char* GetSex(){return m_strSex;}
	inline const char* GetEthnic(){return m_strEthnic;}
	inline const char* GetBirthDayYYYYMMDD(){return m_strBirthDay;}
	inline const char* GetExpireBeginYYYYMMDD(){return m_strExpireBegin;}
	inline const char* GetExpireEndYYYYMMDD(){return m_strExpireEnd;}
	inline const char* GetID(){return m_strID;}
	inline const char* GetAddress(){return m_strAddress;}
	inline const char* GetDepartment(){return m_strDepartment;}
	inline const char* GetAddition(){return m_strAddition;}
private:
	const char* DecodeEthnic( int nEthnic);

	const unsigned char* m_pBaseText;
	char m_strName[32];	
	char m_strSex[5];
	char m_strEthnic[16];
	char m_strBirthDay[10];
	char m_strAddress[72];
	char m_strID[24];
	char m_strDepartment[32];
	char m_strExpireBegin[10];
	char m_strExpireEnd[10];
	char m_strAddition[36];
};
