#include "StdAfx.h"
#include <string.h>
#include <stdio.h>

#include "sw_lock03.h"

#define SleepMs Sleep
#define MS_WAIT 2200

static unsigned char CalcCrc(unsigned char *p,int l)
{
	int i;
	unsigned char c = 0;
	for (i=0;i<l;i++)
		c ^= p[i];
	return c;
}


int SWLock03::MakeCommand(unsigned short Cmd,unsigned short CmdData)
{
	unsigned char pos=0,dlen=2+2; 

	mSendlen = 0;

	mSendbuf[pos++] = 0xAA;
	mSendbuf[pos++] = (unsigned char) (dlen>>8);
	mSendbuf[pos++] = (unsigned char) (dlen & 0xff);
	mSendbuf[pos++] = (unsigned char) (Cmd>>8);
	mSendbuf[pos++] = (unsigned char) (Cmd & 0xff);
	mSendbuf[pos++] = (unsigned char) (CmdData>>8);
	mSendbuf[pos++] = (unsigned char) (CmdData & 0xff);
	mSendbuf[pos++] = CalcCrc(mSendbuf+3,dlen);
	mSendbuf[pos++] = 0x55;
	mSendlen = pos;
	return mSendlen;
}

int SWLock03::SendCommand(unsigned short Cmd,unsigned short CmdData)
{
	int ret=0,retry=0;

	if (mSerial.Handle()==NULL)
		return -1;

	mSerial.EmptyBuffer();

    //����ط�����
    const int nMaxRetry=1;
resend:
	retry++;
	if (retry>nMaxRetry)
		return -1;
	ret = MakeCommand(Cmd,CmdData);
	if (ret<0)
		return ret;	
#ifdef _DEBUG
    CString strOut = CTime::GetCurrentTime().Format("%Y%m%d_%H:%M:%S");
    for(int i=0; i<mSendlen; i++)
    {
        char szBuf[4];
        sprintf(szBuf, " %02X", mSendbuf[i]);
        strOut += szBuf;
    }
    strOut += "\r\n";
    m_fileLog.Write(strOut, strOut.GetLength());
    m_fileLog.Flush();
#endif
    ret = mSerial.Write(mSendbuf,mSendlen);
	if (ret!=mSendlen)
		return -1;

	ret = RecvPackage(Cmd,CmdData);
#ifdef _DEBUG
    if(ret<-1)
    {
        CString strErr;
        strErr.Format("*����0x%0X, data=0x%0X����ֵ��ʱ\r\n", Cmd, CmdData);
        m_fileLog.Write(strErr, strErr.GetLength());
        m_fileLog.Flush();
    }    
#endif
	if (ret>=0)
		return ret;
	else if (ret==-1)
		return ret;
	else
    {
        SleepMs(50);
        //COM��ȡ����ֵ��ʱ
		goto resend;
    }
		
	return ret;
}


// �����������ȡ����ֵ
// 0��ʾ�Ѿ�����
int SWLock03::SendCommandNoReturn(unsigned short Cmd,unsigned short CmdData)
{
    int ret=0;    
    if (mSerial.Handle()==NULL)
        return -1;
    
    mSerial.EmptyBuffer();

    ret = MakeCommand(Cmd,CmdData);
    if (ret<0)
        return ret;
    ret = mSerial.Write(mSendbuf,mSendlen);
    if (ret!=mSendlen)
        return -1;
    
    return ret;
}

//����0�ɹ�
//-1 ʧ��
//-2 ��������
int SWLock03::RecvPackage(unsigned short Cmd,unsigned short CmdData)
{
	int ret;
	int len,packlen;
	unsigned char crc;
	unsigned int repcmd,repdat;
    
	ret = mSerial.ReadTimeout(mRecvbuf,3,MS_WAIT);
	if (ret==-1)
		return -1;
	if (ret!=3)
		return -2;

	len = (mRecvbuf[1]<<8) + mRecvbuf[2];
	if (len+4>MAX_CMDLEN)
		return -2;

	ret = mSerial.ReadTimeout(mRecvbuf+3,len+4-2,MS_WAIT);
	if (ret!=len+4-2)
		return -2;

	repcmd = (mRecvbuf[3]<<8) | mRecvbuf[4];
	repdat = (mRecvbuf[5]<<8) | mRecvbuf[6];

	if (repcmd!=Cmd)
		return -2;
	//if (repdat)


	packlen = ret+3;	//��������
	crc = CalcCrc(mRecvbuf+3,packlen-5);
	if (mRecvbuf[packlen-2]!=crc)
	{
		printf("BAD CRC......\n");
		return -2;
	}

	mRecvlen = packlen;

	return 0;
}

// nPort��Ӧ��ʵ�ʵ�COM�ڣ� nPort=1��ʾCOM1
//
bool SWLock03::SerialOpen(int nPort, int nBaudRate/*=9600*/)
{
	int mDataBit  = 8;
	int mStopBit  = Serial::STOPBIT_1;
	int mCheckBit = Serial::CHECKBIT_NULL;
	int mCtrlBit  = Serial::CTRL_NULL;
	char szPortName[8];
	
	sprintf(szPortName, "COM%d", nPort);
	mSerial.mAddData   = this;
	return mSerial.Open(szPortName,nBaudRate,mDataBit,mStopBit,mCheckBit);
}

void SWLock03::SerialClose()
{
	mSerial.Close();
}

SWLock03::SWLock03()
{
	mLock = CreateMutex(NULL,FALSE,"");
#ifdef _DEBUG
    extern CString MakeModuleFileName( const TCHAR* pstrFileName, HMODULE hModule=NULL);
    CString strFile=MakeModuleFileName("ComLog.txt");
    m_fileLog.Open(strFile, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite, NULL);
#endif    
}

SWLock03::~SWLock03()
{	
	CloseHandle(mLock);
#ifdef _DEBUG
    m_fileLog.Close();
#endif
}


void SWLock03::Lock()
{
	WaitForSingleObject(mLock,INFINITE);
}

void SWLock03::UnLock()
{
	ReleaseMutex(mLock);
}

bool SWLock03::SerialOK()
{
	return mSerial.Handle()!=NULL;
}


/////////////////////////////////////////////////
static const short g_ArrLightCmdOpen[]={
                                        CMDDATA_LED1ON, 
                                        CMDDATA_LED2ON,
                                        CMDDATA_LED3ON,
                                        CMDDATA_LED4ON,
                                        CMDDATA_LED5ON
                                       };
                                
static const short g_ArrLightCmdClose[]={
                                        CMDDATA_LED1OFF, 
                                        CMDDATA_LED2OFF,
                                        CMDDATA_LED3OFF,
                                        CMDDATA_LED4OFF,
                                        CMDDATA_LED5OFF
                                        };
                            
static const short g_ArrGeteOpen[]={
                                    CMDDATA_GATEON1,
                                    CMDDATA_GATEON2
                                    };
                            
bool CGateBoardOper::SwitchLight( int iLight, bool bOpen)
{
    bool bOk=false;
    const int nMax = sizeof(g_ArrLightCmdOpen)/sizeof(g_ArrLightCmdOpen[0]);
    if(0>iLight || iLight>nMax){ return false;}
    
    short sData = bOpen?g_ArrLightCmdOpen[iLight]:g_ArrLightCmdClose[iLight];
    bOk = m_pLocker->SendCommand(LOCK_CMD_LED, sData)==0;
    if(!bOk)
    {
        char szBuf[128];
        sprintf(szBuf,"***��ȡ�����ʧ��: cmd=0x%X, data=0x%X\n", LOCK_CMD_LED, sData);
        OutputDebugString(szBuf);
    }
    return bOk;
}

bool CGateBoardOper::TurnoffAllLight()
{
    const int nMax = sizeof(g_ArrLightCmdOpen)/sizeof(g_ArrLightCmdOpen[0]);
    for(int i=0; i<nMax; i++)
    {
        m_pLocker->SendCommandNoReturn(LOCK_CMD_LED, g_ArrLightCmdClose[i]);
    }
    return true;
}

// iGate=0��բ��
// iGate=1��բ��
bool CGateBoardOper::OpenGate( int iGate)
{
    bool bOk=false;
    const int nMax = sizeof(g_ArrGeteOpen)/sizeof(g_ArrGeteOpen[0]);
    if(0>iGate || iGate>nMax){ return false;}
    
    short sData = g_ArrGeteOpen[iGate];
    bOk = m_pLocker->SendCommand(LOCK_CMD_GATE, g_ArrGeteOpen[iGate])==0;
    if(!bOk)
    {
        char szBuf[128];
        sprintf(szBuf, "***��ȡ�����ʧ��: cmd=0x%X, data=0x%X\n", LOCK_CMD_GATE, sData);
        OutputDebugString(szBuf);
    }
    return bOk;
}


