#include "StdAfx.h"
#include "serial.h"
//#include "../os/os.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN(a,b) (a)>(b)?(b):(a)
#define CHECK_HANDLE do { if (mHandle==NULL) \
					{printf("Serial Not Open.\n"); return -1;}} \
					while (0);

// H.Z.XIN 2015/12/23 
// temp for debug
#ifndef err_print
#define err_print printf
#endif


static int GetBaudrate(int speed)
{
	int i;
	struct bandmap
	{
		int band;
		int flag;
	};
#ifdef _WIN32
	struct bandmap baudtable[] =
	{
		{ 1200, CBR_1200 },
		{ 2400, CBR_2400 },
		{ 4800, CBR_4800 },
		{ 9600, CBR_9600 },
		{ 19200, CBR_19200 },
		{ 38400, CBR_38400 },
		{ 57600, CBR_57600 },
		{ 115200, CBR_115200 },
		{ 128000, CBR_128000 },
		{ 256000, CBR_256000 },
	};	
#else
	struct bandmap baudtable[] =
	{
		{ 1200, B1200 },
		{ 2400, B2400 },
		{ 4800, B4800 },
		{ 9600, B9600 },
		{ 19200, B19200 },
		{ 38400, B38400 },
		{ 57600, B57600 },
		{ 115200, B115200 },
		{ 230400, B230400 },
		{ 460800, B460800 },
		{ 921600, B921600 }
	};
#endif
	
	for (i=0; i<(int)sizeof(baudtable)/(int)sizeof(baudtable[0]);i++)
	{
		if (speed==baudtable[i].band)
		{
			return baudtable[i].flag;
		}
	}
	
	return baudtable[3].flag; //default = 9600
}

#define UPPERCASE(s) std::transform(s.begin(),s.end(),s.begin(),::toupper)
#define LOWERCASE(s) std::transform(s.begin(),s.end(),s.begin(),::tolower)

Serial::Serial()
{
	cbReadHook = NULL;
	cbWriteHook = NULL;
	mHandle = NULL;
	mAddData = NULL;
	memset(mFilter,0,sizeof(mFilter));
	mFilterLen = 0;
	mFilterSw = false;
}

Serial::~Serial()
{
	if (mHandle)
		CloseCom();
}

void Serial::SetupCom()
{
#ifdef _WIN32
    COMMTIMEOUTS ctimeout;
    DCB cDCB;

	//use default buffer size
	//SetupComm(mHandle,1024,1024);
	
    if(!GetCommTimeouts(mHandle, &ctimeout)) goto err;
	//这个值好像没影响：如果获取为0代表没用到，否则代表2个byte间隔，单位ms
    ctimeout.ReadIntervalTimeout = 1;  
	//下面2个参数一起决定了一次Read调用的超时返回时间
	//这是一个乘数
    ctimeout.ReadTotalTimeoutMultiplier = 3; //影响
	//这是一个基数
    ctimeout.ReadTotalTimeoutConstant = 100; //影响
    ctimeout.WriteTotalTimeoutMultiplier = 5;
    ctimeout.WriteTotalTimeoutConstant = 100;
    if(!SetCommTimeouts(mHandle, &ctimeout))
		goto err;
	
    PurgeComm(mHandle, PURGE_TXABORT | PURGE_RXABORT 
		| PURGE_TXCLEAR | PURGE_RXCLEAR );
	
    GetCommState(mHandle, &cDCB);
    cDCB.BaudRate = GetBaudrate(mBandRate);
    cDCB.ByteSize = mBitCount;
	switch (mCheckBit)
	{
	case CHECKBIT_ODD: 
		cDCB.Parity = ODDPARITY;  break;
	case CHECKBIT_EVEN: 
		cDCB.Parity = EVENPARITY; break;
	default:
		cDCB.Parity = NOPARITY;   break;
	}
	switch (mStopBit)
	{
	case STOPBIT_1_5: 
		cDCB.StopBits = ONE5STOPBITS; break;
	case STOPBIT_2: 
		cDCB.StopBits = TWOSTOPBITS; break;
	default: 
		cDCB.StopBits = ONESTOPBIT; break;
	}
    SetCommState(mHandle,&cDCB);
#else
	struct termios newterm;
	int speed;
	int control=0;

	bzero(&newterm,sizeof(newterm));
	newterm.c_cflag |= (CLOCAL | CREAD);
	newterm.c_cflag &= ~CSIZE;
	newterm.c_cc[VTIME] = 0;
	newterm.c_cc[VMIN] = 0;  //0为系统默认1为至少收到1字符就上报
	newterm.c_lflag &= ~(ICANON | ECHOE | ISIG | ECHO);
	newterm.c_oflag &= ~OPOST;
	
	speed = GetBaudrate(mBandRate);
	cfsetispeed(&newterm, speed);
	cfsetospeed(&newterm, speed);
	
	switch(control)
	{
	case 1:{//hardware control
		newterm.c_cflag |= CRTSCTS;
		   }break;
	case 2:{//software control
		newterm.c_cflag |= (IXON | IXOFF |IXANY);
		   }break;
	default:{//no flow control	
		newterm.c_cflag &= ~CRTSCTS;
			}break;
	}
	
	newterm.c_cflag &= ~CSIZE;
	switch(mBitCount)
	{
	case 5:	newterm.c_cflag |= CS5; break;
	case 6: newterm.c_cflag |= CS6; break;
	case 7:	newterm.c_cflag |= CS7; break;
	default:newterm.c_cflag |= CS8; break;
	}
	
	switch(mCheckBit)
	{
	case CHECKBIT_ODD://odd parity
		newterm.c_cflag |= PARENB;	
		newterm.c_cflag |= PARODD;
		break;
	case CHECKBIT_EVEN://even parity
		newterm.c_cflag |= PARENB;
		newterm.c_cflag &= ~PARODD;
		break;
	default://no parity
		newterm.c_cflag &= ~PARENB; 
		break; 	
	}
	
	switch(mStopBit)
	{
	case STOPBIT_2://2 stop bits
		newterm.c_cflag |= CSTOPB;	
		break;
	default:// 1 stop bit
		newterm.c_cflag &= ~CSTOPB;
		break;
	}
	
    tcflush((int)mHandle,TCIFLUSH);/*handle unrecevie char*/
	tcsetattr((int)mHandle,TCSANOW, &newterm);

#endif
	return;
err:
	CloseCom();
}

bool Serial::Open(const char* sName,int bandrate,int bitcount,int stopbit,int check)
{
	mFileName = sName;
	mBandRate = bandrate;
	mBitCount = bitcount;
	mStopBit  = stopbit;
	mCheckBit = check;

	CloseCom();
	OpenCom();
	if ( (mHandle==NULL) || (mHandle < (void*)0) )
	{
		err_print("Open Serial Error: %s\n", sName);
		return false;
	}

	SetupCom();

	return mHandle!=NULL;
}

int  Serial::Write(unsigned char* buf, int len)
{
#ifdef _WIN32
	DWORD writelen=0;
	CHECK_HANDLE;
	WriteFile((HANDLE)mHandle, buf, len,&writelen,NULL);
	if (cbWriteHook && writelen)
	{
		SendCount += writelen;
		cbWriteHook(this, buf, writelen);
	}
	return writelen;
#else
	int  retval, writelen=0, total_len=0;
	fd_set fs_write;
	struct timeval tv;
	
	//return write(fd, data, datalength);
	CHECK_HANDLE;
	while (total_len < len)
	{
		FD_ZERO (&fs_write);
		FD_SET ( (int)mHandle, &fs_write);
		tv.tv_sec = 0;
		tv.tv_usec = 100*1000;
		
		retval = select ((int)mHandle + 1, NULL, &fs_write, NULL, &tv);
		if (retval)
		{
			writelen = write ((int)mHandle, &buf[total_len], len - total_len);
			if (cbWriteHook && writelen)
			{
				SendCount += writelen;
				cbWriteHook(this, &buf[total_len], writelen);
			}
			if (writelen > 0)
			{
				total_len += writelen;
			}
			else if(writelen == -1)
				return -1;
		}
		else
		{
			printf("can't write data to port\n");
			//tcflush (fd, TCOFLUSH);     /* flush all output data */
			break;
		}
	}
	
	return total_len;
#endif
}

int  Serial::Read(unsigned char* buf, int len)
{
	unsigned long readlen=0;
#ifdef _WIN32
	CHECK_HANDLE;
	ReadFile((HANDLE)mHandle,buf,len,&readlen,NULL);
#else
	CHECK_HANDLE;
	readlen = read ((int)mHandle, buf, len);
#endif
	
	if (cbReadHook && readlen)
	{
		RecvCount+=readlen;
		cbReadHook(this, buf, readlen);
	}

	return readlen;
}

void* Serial::Handle()
{
	return mHandle;
}

int  Serial::ReadTimeout(unsigned char* buf,int len,int ms)
{
#ifdef _WIN32
	
	unsigned long readlen=0,readed=0,timeout=0;
	unsigned long t = GetTickCount();
	int buflen;
	CHECK_HANDLE;
	while ( (readed<len) && (timeout<ms) )
	{
		buflen = CheckInCount();
		if (buflen>0)
		{
			ReadFile((HANDLE)mHandle,buf+readed,len-readed,&readlen,NULL);
			if (readlen==-1)
				return -1;
		}
		timeout = GetTickCount()-t;	//这个数值由 COMMTIMEOUTS 决定的。
		readed+=readlen;
		if (readed<len)
			Sleep(50);
	}
	
	if (cbReadHook && readed)
	{
		RecvCount+=readed;
		cbReadHook(this, buf, readed);
	}
	
	return readed;
	
#else
	
	fd_set fs_read;
	struct timeval tv;
	int retval = 0, timeout=0;
	int readed=0;
	CHECK_HANDLE;
	while ( (timeout<ms) && (readed<len) )
	{
		FD_ZERO (&fs_read);
		FD_SET ((int)mHandle, &fs_read);
		tv.tv_sec = 0;
		tv.tv_usec = 50*1000;
		
		retval = select ((int)mHandle + 1, &fs_read, NULL, NULL, &tv);
		if (retval>0)
		{
			retval = read((int)mHandle, buf+readed, len-readed);
			if (retval>0)
			{
				readed += retval;
			}
			else if (retval<0)
				return -1;
		}
		else if (retval==-1)
		{
			return -1;
		}
		else
		{
			//return 0; //don't return
		}
		timeout+=50;
	}
	
	if (cbReadHook && readed)
	{
		RecvCount+=readed;
		cbReadHook(this, buf, readed);
	}

	return readed;
	
#endif
}

void Serial::OpenCom()
{
#ifdef _WIN32
	string realcom;
	int comid,pos;
	char buf[64];
	if (mFileName.length()<4)
		return;
	UPPERCASE(mFileName);
	pos = mFileName.find("COM");
	if (pos==string::npos)
		return;
	realcom = mFileName.substr(pos);
	sscanf(realcom.c_str(),"COM%d",&comid);
	if (comid<8)
	{
		sprintf(buf,"COM%d",comid);
		realcom = buf;
	}
	else
	{
		sprintf(buf,"\\\\.\\COM%d",comid);
		realcom = buf;
	}
	mHandle = CreateFile(realcom.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if ( (mHandle==(void*)0) || (mHandle==(void*)-1) )
		mHandle = NULL;
#else
	mHandle = (void*)open(mFileName.c_str(),O_RDWR);
#endif

	SendCount = 0;
	RecvCount = 0;

	return;
}

void Serial::CloseCom()
{
#ifdef _WIN32
	if (mHandle)
		CloseHandle(mHandle);
#else
	if (mHandle)
		close((int)mHandle);
#endif
	mHandle = NULL;
}

bool Serial::Close()
{
	CloseCom();
	return true;
}

void Serial::EmptyBuffer()
{
	if (mHandle)
	{
#ifdef _WIN32
		PurgeComm((HANDLE)mHandle, PURGE_TXABORT | PURGE_RXABORT 
			| PURGE_TXCLEAR | PURGE_RXCLEAR );
#else
		tcflush ((int)mHandle,TCIOFLUSH);
#endif
	}
}

void Serial::SetFilter(char* f,int len)
{
	int l;
	if ((len<1) || (!f))
		return;
	l = MIN(len,MAXSERIALFILTER);
	memset(mFilter,0,MAXSERIALFILTER);
	memcpy(mFilter,f,l);
	mFilterLen = l;
}

void Serial::FilterOff()
{
	mFilterSw = false;
}

void Serial::FilterOn()
{
	mFilterSw = true;
}

//使用时注意：ReadTimeoutFilter只是1个字节一个字节的读
//            当写log时，注意log文件大小
int Serial::ReadTimeoutFilter(unsigned char *data, int datalength,int ms)
{
	int readed=0,i,del;
	unsigned char c;
	int t=0,r;
	unsigned long intime;

	if (datalength==0)
		return 0;
	
	intime = GetTickCount();
	while ( (readed < datalength) && (t<ms) )
	{
		r = ReadTimeout(&c,1,500);
		t = GetTickCount() - intime;
		//printf("t--->%d, ms:%d\n",t,ms);
		if ( r == -1)
		{
			break;
		}
		else if (r==0)
		{
			continue;
		}
		
		if (mFilterSw)
		{
			del = 0;
			for (i=0;i<mFilterLen;i++)
			{
				if (c==mFilter[i])
				{
					del = 1;
					break;
				}
			}
			if (del)
			{
				//读1字节丢掉
				data[readed++] = c;
				if ( 1 != ReadTimeout(&c,1,t) )
				{
					break;
				}
				continue;
			}
		}
		
		data[readed++] = c;
	}
	return readed;
}

//使用时注意：WriteFilte只是1个字节一个字节的读
//            当写log时，注意log文件大小
int Serial::WriteFilter(unsigned char *data, int datalength)
{
	int writed=0,i,rep;
	unsigned char c;
	
	while (writed < datalength)
	{
		c = data[ writed ];
		if ( 1 != Write(&c,1) )
			break;
		
		if (mFilterSw)
		{
			rep = 0;
			for (i=0;i<mFilterLen;i++)
			{
				if (c==mFilter[i])
				{
					rep = 1;
					break;
				}
			}
			if (rep)
			{
				if ( 1 != Write(&c,1) )
					break;
			}
		}
		
		writed++;
	}
	return writed;
}

int Serial::CheckInCount()
{
#ifdef _WIN32
	DWORD ec;
	COMSTAT st;
	ClearCommError(mHandle,&ec,&st);
	return st.cbInQue;
#else
	int nread;
	ioctl((int)mHandle,FIONREAD,&nread);
	return nread;
#endif
}

std::string Serial::GetName()
{
	return mFileName;
}