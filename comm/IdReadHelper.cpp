#include "StdAfx.h"
#include "IdReadHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static 
unsigned char SumCheck(unsigned char * bData, int nLen)
{
	unsigned char bRet = 0;
	for(int n = 0 ; n < nLen ; n++)
	{
		bRet =  bRet ^ bData[n]; 
	}
	return bRet;
}


static void ReadHook(void* handle,unsigned char* data,int datalen)
{
// #ifdef _DEBUG
//     char szBuf[128]={0};
// 	sprintf(szBuf, "Serial Read,handle=%d\n", handle);
//     OutputDebugString(szBuf);
// 	for(int i = 0; i < datalen; i++)
// 	{
// 		sprintf(szBuf, "%02x ", data[i]);
//         OutputDebugString(szBuf);
// 	}
// 	OutputDebugString("\n");
// #endif
}

static void WriteHook(void* handle,unsigned char* data,int datalen)
{
// #ifdef _DEBUG
//     char szBuf[128]={0};
// 	sprintf(szBuf, "Serial Write,handle=%d\n", handle);
//     OutputDebugString(szBuf);
// 	for(int i = 0; i < datalen; i++)
// 	{
// 		sprintf(szBuf, "%02x ", data[i]);
//         OutputDebugString(szBuf);
// 	}
// 	OutputDebugString("\n");
// #endif
}



/////////////////////////////////////////////////////////////////////

CIdReadHelper::CIdReadHelper(void)
{
	ResetData();
	m_hDllWltRS=NULL;
	m_pfnGetBmp=NULL;
}


CIdReadHelper::~CIdReadHelper(void)
{
	SerialClose();
	UnloadWltRs_dll();
}


void CIdReadHelper::ResetData()
{
	memset(mRecvData,0,sizeof(mRecvData));
	memset(pucIIN,0,sizeof(pucIIN));
	memset(pucSN,0,sizeof(pucSN));
	memset(pucBaseText,0,sizeof(pucBaseText));
	memset(pucPhoto,0,sizeof(pucPhoto));
	memset(pucExtra,0,sizeof(pucExtra));
    m_bLastRead=false;
	pucBaseTextLen=0;
}

// nPort��Ӧ��ʵ�ʵ�COM�ڣ� nPort=1��ʾCOM1
//
bool CIdReadHelper::SerialOpen(int nPort, int nBaudRate/*=115200*/)
{
	int mDataBit  = 8;
	int mStopBit  = Serial::STOPBIT_1;
	int mCheckBit = Serial::CHECKBIT_NULL;
	int mCtrlBit  = Serial::CTRL_NULL;
	char szPortName[8];
	
	sprintf(szPortName, "COM%d", nPort);
	mSerial.mAddData   = this;
	mSerial.cbReadHook = ReadHook;
	mSerial.cbWriteHook= WriteHook;
	return mSerial.Open(szPortName,nBaudRate,mDataBit,mStopBit,mCheckBit);
}

void CIdReadHelper::SerialClose()
{
	mSerial.Close();
}


//����>0�ɹ����Ӵ��ڽ��ս���� mRecvData
//mRecvData��ǰʮ���ֽ��ǹ̶���״̬����
//
int CIdReadHelper::SendCommand(unsigned char cmd,
								unsigned char para,
								unsigned char &sw1,
								unsigned char &sw2,
								unsigned char &sw3,
								int timeout)
{
	unsigned char cSendData[128];
	unsigned char cks;
	int nsend,nrecv,nlen;
	int sendlen=3;  //sendlen�̶�Ϊ3,cmd+para+chksum

	sw1 = 0;
	sw2 = 0;
	sw3 = 0;
	cSendData[0] = 0XAA;
	cSendData[1] = 0XAA;
	cSendData[2] = 0XAA;
	cSendData[3] = 0X96;
	cSendData[4] = 0X69;
	cSendData[5] = 0;
	cSendData[6] = sendlen;
	cSendData[7] = cmd;
	cSendData[8] = para;
	cSendData[9] = SumCheck(cSendData+5,4);

	//mSerial.EmptyBuffer();
	nsend = mSerial.Write(cSendData,10);
	if (nsend!=10)
		return -1;

	nrecv = mSerial.ReadTimeout(mRecvData,7,timeout); //����ͷ5�ֽڣ����ݳ���2�ֽ�
	if (nrecv!=7)
		return -1;

	nlen = (mRecvData[5]<<8) | mRecvData[6];
	//���ݳ��Ȳ�����С��3
	if (nlen<3)
	{
		//printf("Error:RecvData len %d<3\n",nlen);
		return -1;
	}
	nrecv = mSerial.ReadTimeout(mRecvData+7,nlen,2000);
	if (nrecv!=nlen)
		return -1;

	nrecv = 7+nlen;
	cks = SumCheck(mRecvData+5,nlen+2-1); //2λ����Ҫ�㣬�����1λ����
	if (mRecvData[nrecv-1]!=cks)
	{
		//printf("ChecksumErr,read:%02x,calc:%02x.\n",mRecvData[nrecv-1],cks);
		return -1;
	}

	sw1 = mRecvData[7];
	sw2 = mRecvData[8];
	sw3 = mRecvData[9];

	return nrecv;
}

//SAM_V״̬��⣬�Ƿ���������
int CIdReadHelper::SAM_GetState()
{
	unsigned char sw1,sw2,sw3;
	int ret;
	ret = SendCommand(0x11,0xff,sw1,sw2,sw3,1000);
	if ( (ret>0) && (sw3==0x90) )
		return 0;
	return -1;
}

//��λSAM_V 
int CIdReadHelper::SAM_Reset()
{
	unsigned char sw1,sw2,sw3;
	int ret;
	ret = SendCommand(0x10,0xff,sw1,sw2,sw3,1000);
	if ( (ret>0) && (sw3==0x90) )
		return 0;
	return ret;
}

//����COM�ڲ�����
//Ĭ����115200
int CIdReadHelper::SetBaudRate(int nBaud)
{
	unsigned char sw1,sw2,sw3;
	int ret;
	unsigned char nRate=0x00;
	switch(nBaud)
	{
	case 57600: nRate=0x01; break;
	case 38400: nRate=0x02; break;
	case 19200: nRate=0x03; break;
	case 9600:  nRate=0x04; break;
	default:    nRate=0x00; break;
	}	
	ret = SendCommand(0x60,nRate,sw1,sw2,sw3,1000);
	if((ret>0) && (sw3==0x90))
	{
		return 0;
	}
	return -1;
}

//Ѱ��֤/��
int CIdReadHelper::SAM_FindCard()	
{
	unsigned char sw1,sw2,sw3;
	int ret;
	ret = SendCommand(0x20,0x01,sw1,sw2,sw3,1000);
	if ( (ret>0) && (sw3==0x9f) )
	{
		memcpy(pucIIN, &mRecvData[10], 4);
		//printf("FindCard,sw:%d,%d,%d.\n",sw1,sw2,sw3);
		return 0;
	}
	return ret;
}

int CIdReadHelper::SAM_SelectCard()	
{
	unsigned char sw1,sw2,sw3;
	int ret;
	//aa aa aa 96 69 00 03 20 01 22 ed
	ret = SendCommand(0x20,0x02,sw1,sw2,sw3,1000);
	if( (ret>0) && (sw3==0x90) )
	{
		memcpy(pucSN,mRecvData+10,8);
		return 0;
	}
	return ret;
}

//���̶���Ϣ 
int CIdReadHelper::SAM_ReadBaseMsg() 
{
	unsigned char sw1,sw2,sw3;
	int ret = SendCommand(0x30,0x01,sw1,sw2,sw3,3000);
	const int RET_HEAD=10;

	if((ret>0) && (sw3==0x90))
	{
		//���й̶��ı���Ϣ,��ʵ����256
		pucBaseTextLen = (mRecvData[RET_HEAD]<<8) | mRecvData[RET_HEAD+1];
		memcpy(pucBaseText, mRecvData+RET_HEAD+4, pucBaseTextLen);
		
		//��������Ƭ����,��ʵ����1024
		pucPhotoLen = (mRecvData[RET_HEAD+2]<<8) | mRecvData[RET_HEAD+3];
		memcpy(pucPhoto, mRecvData+RET_HEAD+4+pucBaseTextLen, pucPhotoLen); //?

		return 0;
	}
	return -1;
}

int CIdReadHelper::ReadCard()
{
	int nRet;
	ResetData();
	mSerial.EmptyBuffer();
	//nRet = SAM_Reset();
	//if (nRet!=WFS_SUCCESS)
	//	return nRet;
	nRet = SAM_FindCard();
	if (nRet != 0)
		return nRet;
	nRet = SAM_SelectCard();
	if (nRet != 0)
	{
		//printf("SAM_SelectCard nRet=%d\n", nRet);
		return nRet;
	}
	nRet = SAM_ReadBaseMsg();
    m_bLastRead = (0==nRet);
	return nRet;
}


int CIdReadHelper::SAM_ReadExtraMsg() 
{
	unsigned char sw1,sw2,sw3;
	int ret = SendCommand(0x30,0x03,sw1,sw2,sw3,3000);
	const int RET_HEAD=10;

	if((ret>0) && (sw3==0x90))
	{
		memcpy(pucExtra, mRecvData+RET_HEAD, 70);
		return 0;
	}
	return ret;
}

// static void* SearchThread(void *pparam)
// {
// 	CIdReadHelper* pmod=(CIdReadHelper*)pparam;
// 	if (pmod)
// 		pmod->Searching();
// 	return NULL;
// }
// 
// 
// void CIdReadHelper::Searching()
// {
// 	int nRet;
// 	printf("CreateThread  IDCard Searching.\n"); 
// 
// 	while (bCanExit==false)
// 	{
// 		Lock();
// 		nRet = ReadCard();
// 		UnLock();
// 		SleepMs(100);
// 	}
// 	printf("CIdReadHelper read thread exit.\n");
// }


////////////////////////////////////////////////////////////////
void CIdReadHelper::UnloadWltRs_dll()
{
	if(m_hDllWltRS)
	{
		FreeLibrary(m_hDllWltRS);
		m_hDllWltRS=NULL;
	}
}

bool CIdReadHelper::LoadWltRs_dll()
{
	m_hDllWltRS = LoadLibrary("WltRS.dll");
	if (m_hDllWltRS==INVALID_HANDLE_VALUE)
	{
		return false;
	}

	m_pfnGetBmp = (FGetBmp)GetProcAddress(m_hDllWltRS,"GetBmp");
	if(!m_pfnGetBmp)
	{
		FreeLibrary(m_hDllWltRS);
		m_hDllWltRS=NULL;
		return false;
	}
	return true;
}

static
void PrintGetBmpError(int nRet)
{
	const char* pstr="ok";
	switch (nRet)
	{
	case 1:
		pstr="��Ƭ������ȷ";
		break;
	case 0:
		pstr="����sdtapi.dll����";
		break;
	case -1:
		pstr="��Ƭ�������";
		break;
	case -2:
		pstr="wlt�ļ���׺����";
		break;
	case -3:
		pstr="wlt�ļ��򿪴���";
		break;
	case -4:
		pstr="wlt�ļ���ʽ����";
		break;
	case -5:
		pstr="���δ��Ȩ";
		break;
	case -6:
		pstr="�豸���Ӵ���";
		break;
	default:
		pstr="unknown";
		break;
	}
	char szBuf[100];
	sprintf(szBuf, "***GetBmp return, %s\n", pstr);
	OutputDebugString(szBuf);
}

// 1�ɹ�
// 0����ʧ��
int CIdReadHelper::wlt2bmp(const char* filename)
{
	int ret=0;	
	if(!m_pfnGetBmp)
	{
		if(!LoadWltRs_dll())
		{
			return -1;
		}
	}

	ret = m_pfnGetBmp((char*)filename,2);
	PrintGetBmpError(ret);
	return ret;
}




////////////////////////////////////////////////////
char* WtoA(const wchar_t *src, int src_bytes)
{
	const int MAX_BUFLEN=512;
	static char s_Buf[MAX_BUFLEN]={0};
    memset(s_Buf, 0, MAX_BUFLEN);
	int nSrcChars = src_bytes/sizeof(wchar_t);
	int len = ::WideCharToMultiByte(CP_ACP, 0, src, nSrcChars, s_Buf, MAX_BUFLEN, 0, 0);
	s_Buf[len]=NULL;
	return	s_Buf;
}


// wchat_t* ----> char*�ٽ���
//
bool CIDBaseTextDecoder::Decode(const unsigned char* pBaseText)
{
    m_pBaseText=pBaseText;
	if(!m_pBaseText){ return false;}
	const unsigned char* ptr = m_pBaseText;
	char* pstrData=NULL;
	pstrData = WtoA((const wchar_t*)&ptr[0], 30);
	strcpy(m_strName, pstrData);

	pstrData = WtoA((const wchar_t*)&ptr[0x1F-1], 2);
	int nSex = atoi(pstrData);
	switch(nSex)
	{
	case 1: strcpy(m_strSex, "��"); break;
	case 2: strcpy(m_strSex, "Ů"); break;
	case 9:
	default: strcpy(m_strSex, "����"); break;
	}

	//����
	pstrData = WtoA((const wchar_t*)&ptr[0x21-1], 4);
	int nEthnic = atoi(pstrData);
	strcpy(m_strEthnic, DecodeEthnic(nEthnic));

	//��������
	pstrData = WtoA((const wchar_t*)&ptr[0x25-1], 16);
	strcpy(m_strBirthDay, pstrData);

	//��ַ
	pstrData = WtoA((const wchar_t*)&ptr[0x35-1], 70);
	//int nLen = strlen(pstrData);
	strcpy(m_strAddress, pstrData);

	//���֤��
	pstrData = WtoA((const wchar_t*)&ptr[0x7B-1], 36);
	strcpy(m_strID, pstrData);

	//ǩ����λ
	pstrData = WtoA((const wchar_t*)&ptr[0x9F-1], 30);
	strcpy(m_strDepartment, pstrData);

	//��Ч����ʼ
	pstrData = WtoA((const wchar_t*)&ptr[0xBD-1], 16);
	strcpy(m_strExpireBegin, pstrData);
	//��Ч�ڽ���
	pstrData = WtoA((const wchar_t*)&ptr[0xCD-1], 16);
	strcpy(m_strExpireEnd, pstrData);

	//׷����Ϣ
	pstrData = WtoA((const wchar_t*)&ptr[0xDD-1], 36);
	strcpy(m_strAddition, pstrData);
	return true;
}


const char* CIDBaseTextDecoder::DecodeEthnic( int nEthnic)
{
	switch(nEthnic)
	{
	case 1:  return "����";
	case 2:  return "�ɹ���";
	case 3:  return "����";
	case 4:  return "����";
	case 5:  return "ά�����";
	case 6:  return "����";
	case 7:  return "����";
	case 8:  return "׳��";
	case 9:  return "������";
	case 10: return "������";
	case 11: return "����";
	case 12: return "����";
	case 13: return "����";
	case 14: return "����";
	case 15: return "������";
	case 16: return "������";
	case 17: return "��������";
	case 18: return "����";
	case 19: return "����";
	case 20: return "������";
	case 21: return "����";
	case 22: return "���";
	case 23: return "��ɽ��";
	case 24: return "������";
	case 25: return "ˮ��";
	case 26: return "������";
	case 27: return "������";
	case 28: return "������";
	case 29: return "�¶�������";
	case 30: return "����";
	case 31: return "���Ӷ���";
	case 32: return "������";
	case 33: return "Ǽ��";
	case 34: return "������";
	case 35: return "������";
	case 36: return "ë����";
	case 37: return "������";
	case 38: return "������";
	case 39: return "������";
	case 40: return "������";
	case 41: return "��������";
	case 42: return "ŭ��";
	case 43: return "���α����";
	case 44: return "����˹��";
	case 45: return "���¿���";
	case 46: return "������";
	case 47: return "������";
	case 48: return "ԣ����";
	case 49: return "����";
	case 50: return "��������";
	case 51: return "������";
	case 52: return "���״���";
	case 53: return "������";
	case 54: return "�Ű���";
	case 55: return "�����";
	case 56: return "��ŵ��";
	case 97: return "����";
	case 98: return "���Ѫͳ";
	default: return "����";
	}
    return "����";
}
