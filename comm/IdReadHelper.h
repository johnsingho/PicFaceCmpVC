#pragma once

#include "serial.h"


// for 
typedef int (WINAPI *FGetBmp)(char *wltfile,int intf);


//////////////////////////////////////////////////////////////
//
// ���֤��ȡ������
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

	//ȡ��ȡ���Ĺ̶���Ϣ
	unsigned char* GetBaseText(){return pucBaseText;}
	//ȡ��ȡ��ͼƬ����
	unsigned char* GetPhoto(){return pucPhoto;}
    
    //ת��wlt�ļ�Ϊbmp�ļ�
    int wlt2bmp(const char* filename);

    inline bool IsOpened(){return NULL!=mSerial.Handle();}
    inline bool IsReadOk(){return m_bLastRead;}
private:
	int SendCommand(unsigned char cmd, unsigned char para, unsigned char &sw1, unsigned char &sw2, unsigned char &sw3, int timeout);

	bool LoadWltRs_dll();
	void UnloadWltRs_dll();	
	
private:
	unsigned char mRecvData[2048];

	unsigned char pucIIN[4]; //֤/��оƬ�����
	unsigned char pucSN[8];  //֤/��оƬ���к�
	unsigned char pucBaseText[260]; //�̶���Ϣ�ı����256���ֽ�
	int pucBaseTextLen;
	unsigned char pucPhoto[1024];   //�̶���Ϣ��Ƭ���1024���ֽ�
	int pucPhotoLen;
	unsigned char pucExtra[72];     //׷����Ϣ���70�ֽ�

	Serial mSerial;

	HMODULE m_hDllWltRS;
	FGetBmp m_pfnGetBmp;
    bool    m_bLastRead;   //��һ�ζ����Ƿ�ɹ�
};


// //! ???
// #pragma pack(1)
// typedef struct tagIDCardData{
// 	char Name[32];			//����       
// 	char Sex[4];			//�Ա�
// 	char Nation[6];			//����
// 	char Born[18];			//��������
// 	char Address[72];		//סַ
// 	char IDCardNo[38];		//���֤��
// 	char GrantDept[32];		//��֤����
// 	char UserLifeBegin[18]; //��Ч��ʼ����
// 	char UserLifeEnd[18];	//��Ч��ֹ����
// 	char reserved[38];		//����
// }IDCardData;
// #pragma pack()


// ���͹̶���Ϣ
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
