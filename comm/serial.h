#ifndef _XYSERIAL_
#define _XYSERIAL_

#include <cstdio>
#include <fcntl.h>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <stdarg.h>
#include <iostream>
#include <fstream>

#ifdef _WIN32
	#include <stdlib.h>
	#include <windows.h>
#else
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/ioctl.h>
	#include <linux/serial.h>
	#include <asm/ioctls.h>
	#include <termios.h>	
	#include <unistd.h>	
#endif

using namespace std;

#define MAXSERIALFILTER 16

//pserial: Serial ��
//data   : ����/���͵�����ָ��
//len    : ���������ĳ���
typedef void (*SERIALHOOK)(void* pserial,unsigned char* data,int datalen);

class Serial
{
private:
	string mFileName;
	int mBandRate;
	int mBitCount;
	int mStopBit;
	int mCheckBit;
	
	unsigned char  mFilter[MAXSERIALFILTER];
	int   mFilterLen;
	bool  mFilterSw;

	//�շ�ͳ��
	int SendCount;
	int RecvCount;

	//windows: mHandle=CreateFile���ؾ��
	//linux  : mHandle=open���ؾ��
	//ֻ��ͳһΪ void* ����
	void* mHandle;

protected:
	void OpenCom();
	void CloseCom();
	void SetupCom();

public:

	enum 
	{
		STOPBIT_1,
		STOPBIT_1_5,
		STOPBIT_2
	};

	enum 
	{
		CHECKBIT_NULL,
		CHECKBIT_ODD,
		CHECKBIT_EVEN,
	};

	enum
	{
		CTRL_NULL,
		CTRL_SOFTWARE,
		CTRL_HARDWARE,
	};

	Serial();
	~Serial();

	SERIALHOOK cbReadHook;
	SERIALHOOK cbWriteHook;

	//����һ���������Ա� hook ʱ���������������,Ĭ��ΪNULL
	void* mAddData;
	string GetName();
	void* Handle();
	bool Close();
	void EmptyBuffer();
	bool Open(const char* sName,int bandrate,int bitcount,int stopbit,int check);
	int  Write(unsigned char* buf, int len);
	int  Read(unsigned char* buf, int len);
	int  CheckInCount();  //�����ܻ��������ݳ���
	int  ReadTimeout(unsigned char* buf, int len, int ms);

	void SetFilter(char* f,int len);
	void FilterOn();
	void FilterOff();

	int GetReadCount() {return RecvCount;}
	int GetWriteCount() {return SendCount;}

	//ʹ�ù���������д
	int ReadTimeoutFilter(unsigned char *data, int datalength,int ms);
	int WriteFilter(unsigned char *data, int datalength);
};







#endif