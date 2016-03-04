#ifndef HEAD_SW_LOCK03_HHH
#define HEAD_SW_LOCK03_HHH

#include "serial.h"

#define MAX_CMDLEN 64

//命令
#define LOCK_CMD_GATE  0x0102
#define LOCK_CMD_LED   0x0107

//命令的参数
#define CMDDATA_GATEON1		0x0003 /*右闸门，暂时没用 H.Z.XIN 2016-01-07*/
#define CMDDATA_GATEON2		0x0004 /*左闸门*/
#define CMDDATA_LED1ON		0x0003
#define CMDDATA_LED1OFF		0x0004
#define CMDDATA_LED2ON		0x0005
#define CMDDATA_LED2OFF		0x0006
#define CMDDATA_LED3ON		0x0007
#define CMDDATA_LED3OFF		0x0008
#define CMDDATA_LED4ON		0x0009
#define CMDDATA_LED4OFF		0x000a
#define CMDDATA_LED5ON		0x000b
#define CMDDATA_LED5OFF		0x000c


class SWLock03
{
private:
	Serial mSerial;
	HANDLE mLock;

	int MakeCommand(unsigned short Cmd,unsigned short CmdData);

	unsigned char mRecvbuf[MAX_CMDLEN];
	int mRecvlen;
	unsigned char mSendbuf[MAX_CMDLEN];
	int mSendlen;
	int RecvPackage(unsigned short Cmd,unsigned short CmdData);
	
public:
	SWLock03();
	~SWLock03();

	bool SerialOpen(int nPort, int nBaudRate=9600);
	void SerialClose();
	bool SerialOK();


	void Lock();
	void UnLock();

	
	int SendCommand(unsigned short Cmd,unsigned short CmdData);
    int SendCommandNoReturn(unsigned short Cmd,unsigned short CmdData);
#ifdef _DEBUG
    CFile m_fileLog;
#endif
};


// 用来操作灯板和闸门
class CGateBoardOper
{
public:
    CGateBoardOper():m_pLocker(NULL){}
    inline void Bind(SWLock03* pLock){ m_pLocker=pLock;}
    bool SwitchLight( int iLight, bool bOpen);
    bool OpenGate( int iGate);
    bool TurnoffAllLight();
private:
    SWLock03* m_pLocker;
};


#endif