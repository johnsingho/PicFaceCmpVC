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

//pserial: Serial 类
//data   : 接收/发送的数据指针
//len    : 该数据区的长度
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

	//收发统计
	int SendCount;
	int RecvCount;

	//windows: mHandle=CreateFile返回句柄
	//linux  : mHandle=open返回句柄
	//只是统一为 void* 而已
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

	//附加一个参数，以便 hook 时，把这个参数带出,默认为NULL
	void* mAddData;
	string GetName();
	void* Handle();
	bool Close();
	void EmptyBuffer();
	bool Open(const char* sName,int bandrate,int bitcount,int stopbit,int check);
	int  Write(unsigned char* buf, int len);
	int  Read(unsigned char* buf, int len);
	int  CheckInCount();  //检查接受缓冲区数据长度
	int  ReadTimeout(unsigned char* buf, int len, int ms);

	void SetFilter(char* f,int len);
	void FilterOn();
	void FilterOff();

	int GetReadCount() {return RecvCount;}
	int GetWriteCount() {return SendCount;}

	//使用过滤器读，写
	int ReadTimeoutFilter(unsigned char *data, int datalength,int ms);
	int WriteFilter(unsigned char *data, int datalength);
};







#endif