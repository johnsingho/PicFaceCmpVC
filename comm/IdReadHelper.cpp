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

// nPort对应于实际的COM口， nPort=1表示COM1
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


//返回>0成功，从串口接收结果放 mRecvData
//mRecvData的前十个字节是固定的状态返回
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
	int sendlen=3;  //sendlen固定为3,cmd+para+chksum

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

	nrecv = mSerial.ReadTimeout(mRecvData,7,timeout); //数据头5字节，数据长度2字节
	if (nrecv!=7)
		return -1;

	nlen = (mRecvData[5]<<8) | mRecvData[6];
	//数据长度不可能小于3
	if (nlen<3)
	{
		//printf("Error:RecvData len %d<3\n",nlen);
		return -1;
	}
	nrecv = mSerial.ReadTimeout(mRecvData+7,nlen,2000);
	if (nrecv!=nlen)
		return -1;

	nrecv = 7+nlen;
	cks = SumCheck(mRecvData+5,nlen+2-1); //2位长度要算，但最后1位不算
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

//SAM_V状态检测，是否正常工作
int CIdReadHelper::SAM_GetState()
{
	unsigned char sw1,sw2,sw3;
	int ret;
	ret = SendCommand(0x11,0xff,sw1,sw2,sw3,1000);
	if ( (ret>0) && (sw3==0x90) )
		return 0;
	return -1;
}

//复位SAM_V 
int CIdReadHelper::SAM_Reset()
{
	unsigned char sw1,sw2,sw3;
	int ret;
	ret = SendCommand(0x10,0xff,sw1,sw2,sw3,1000);
	if ( (ret>0) && (sw3==0x90) )
		return 0;
	return ret;
}

//设置COM口波特率
//默认是115200
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

//寻找证/卡
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

//读固定信息 
int CIdReadHelper::SAM_ReadBaseMsg() 
{
	unsigned char sw1,sw2,sw3;
	int ret = SendCommand(0x30,0x01,sw1,sw2,sw3,3000);
	const int RET_HEAD=10;

	if((ret>0) && (sw3==0x90))
	{
		//所有固定文本信息,其实都是256
		pucBaseTextLen = (mRecvData[RET_HEAD]<<8) | mRecvData[RET_HEAD+1];
		memcpy(pucBaseText, mRecvData+RET_HEAD+4, pucBaseTextLen);
		
		//读出的照片数据,其实都是1024
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
		pstr="相片解码正确";
		break;
	case 0:
		pstr="调用sdtapi.dll错误";
		break;
	case -1:
		pstr="相片解码错误";
		break;
	case -2:
		pstr="wlt文件后缀错误";
		break;
	case -3:
		pstr="wlt文件打开错误";
		break;
	case -4:
		pstr="wlt文件格式错误";
		break;
	case -5:
		pstr="软件未授权";
		break;
	case -6:
		pstr="设备连接错误";
		break;
	default:
		pstr="unknown";
		break;
	}
	char szBuf[100];
	sprintf(szBuf, "***GetBmp return, %s\n", pstr);
	OutputDebugString(szBuf);
}

// 1成功
// 0或负数失败
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


// wchat_t* ----> char*再解释
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
	case 1: strcpy(m_strSex, "男"); break;
	case 2: strcpy(m_strSex, "女"); break;
	case 9:
	default: strcpy(m_strSex, "其他"); break;
	}

	//民族
	pstrData = WtoA((const wchar_t*)&ptr[0x21-1], 4);
	int nEthnic = atoi(pstrData);
	strcpy(m_strEthnic, DecodeEthnic(nEthnic));

	//出生日期
	pstrData = WtoA((const wchar_t*)&ptr[0x25-1], 16);
	strcpy(m_strBirthDay, pstrData);

	//地址
	pstrData = WtoA((const wchar_t*)&ptr[0x35-1], 70);
	//int nLen = strlen(pstrData);
	strcpy(m_strAddress, pstrData);

	//身份证号
	pstrData = WtoA((const wchar_t*)&ptr[0x7B-1], 36);
	strcpy(m_strID, pstrData);

	//签发单位
	pstrData = WtoA((const wchar_t*)&ptr[0x9F-1], 30);
	strcpy(m_strDepartment, pstrData);

	//有效期起始
	pstrData = WtoA((const wchar_t*)&ptr[0xBD-1], 16);
	strcpy(m_strExpireBegin, pstrData);
	//有效期结束
	pstrData = WtoA((const wchar_t*)&ptr[0xCD-1], 16);
	strcpy(m_strExpireEnd, pstrData);

	//追加信息
	pstrData = WtoA((const wchar_t*)&ptr[0xDD-1], 36);
	strcpy(m_strAddition, pstrData);
	return true;
}


const char* CIDBaseTextDecoder::DecodeEthnic( int nEthnic)
{
	switch(nEthnic)
	{
	case 1:  return "汉族";
	case 2:  return "蒙古族";
	case 3:  return "回族";
	case 4:  return "藏族";
	case 5:  return "维吾尔族";
	case 6:  return "苗族";
	case 7:  return "彝族";
	case 8:  return "壮族";
	case 9:  return "布依族";
	case 10: return "朝鲜族";
	case 11: return "满族";
	case 12: return "侗族";
	case 13: return "瑶族";
	case 14: return "白族";
	case 15: return "土家族";
	case 16: return "哈尼族";
	case 17: return "哈萨克族";
	case 18: return "傣族";
	case 19: return "黎族";
	case 20: return "傈僳族";
	case 21: return "佤族";
	case 22: return "畲族";
	case 23: return "高山族";
	case 24: return "拉祜族";
	case 25: return "水族";
	case 26: return "东乡族";
	case 27: return "纳西族";
	case 28: return "景颇族";
	case 29: return "柯尔克孜族";
	case 30: return "土族";
	case 31: return "达斡尔族";
	case 32: return "仫佬族";
	case 33: return "羌族";
	case 34: return "布朗族";
	case 35: return "撒拉族";
	case 36: return "毛难族";
	case 37: return "仡佬族";
	case 38: return "锡伯族";
	case 39: return "阿昌族";
	case 40: return "普米族";
	case 41: return "塔吉克族";
	case 42: return "怒族";
	case 43: return "乌孜别克族";
	case 44: return "俄罗斯族";
	case 45: return "鄂温克族";
	case 46: return "崩龙族";
	case 47: return "保安族";
	case 48: return "裕固族";
	case 49: return "京族";
	case 50: return "塔塔尔族";
	case 51: return "独龙族";
	case 52: return "鄂伦春族";
	case 53: return "赫哲族";
	case 54: return "门巴族";
	case 55: return "珞巴族";
	case 56: return "基诺族";
	case 97: return "其他";
	case 98: return "外国血统";
	default: return "其他";
	}
    return "其他";
}
