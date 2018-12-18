/******************************************************************************
* Copyright (c) 2013, NULL, All rights reserved.
*
* File Name  : main.c
* Description: 
* Author     : jiangfeng.zhang(retries@126.com)
* Date       : 2018/12/18
* Version    : 1.0
*
* Change Logs:
*   2018/12/18 jiangfeng.zhang : Create file.
******************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/time.h>
#include <netdb.h>
#include <net/if.h>

#include "DriverComm.h"

/******************************************************************************
* Macros.
******************************************************************************/
#define AEC_DRIVER_ERROR_PRINTF(fmt, args...) \
do \
{ \
	printf("[%s][%s][%s][%d]:","ERROR",__FILE__, __FUNCTION__, __LINE__); \
	printf(fmt,##args); \
}while(0); 
	

/******************************************************************************
* Variables.
******************************************************************************/
typedef struct
{
	unsigned char ucEnableSend:1;		//permit to send data
	unsigned char ucEnableRecv:1;		//permit to receive data
	unsigned char ucMode:1;           //select 485 or 232
	unsigned char ucReserved:1;
	unsigned char ucRate:4;		       //setting baud rate
}ST_UART_BRAD_RATE;

typedef struct
{
	unsigned char ucDir:2;
	unsigned char ucStop:2;
	unsigned char ucCheck:2;
	unsigned char ucWid:2;
}ST_UART_FLAG;

typedef struct
{
    ST_UART_FLAG   stUartFlag;
	ST_UART_BRAD_RATE  stBradRate;
}ST_UART_PARA;

/******************************************************************************
* functions.
******************************************************************************/
/******************************************************************************
* Function   :  UartParaSet
* Description:  设置指定串口参数
* Parameter  :  iDevFd：指定串口的文件描述符
                其他：串口参数
*
* Return     :
*
*
* Explain    :  此函数用于设置串口参数：波特率、校验位、停止位、传输方向等参数
******************************************************************************/
int DrivCommUartParaSet(int iDevFd,
				      unsigned char ucUartMode,
				      unsigned char ucBoadRate,
				      unsigned char ucCharWid,
				      unsigned char ucCheckBit,
				      unsigned char ucStopBit)
{
	ST_UART_PARA stUartPara;

	memset(&stUartPara, 0, sizeof(ST_UART_PARA));

	stUartPara.stBradRate.ucEnableSend = 1;
	stUartPara.stBradRate.ucEnableRecv = 1;
	stUartPara.stBradRate.ucMode = ucUartMode;
	stUartPara.stBradRate.ucRate = ucBoadRate;

	stUartPara.stUartFlag.ucCheck = ucCheckBit;
	stUartPara.stUartFlag.ucDir   = UART_DIR_SEND_RECV;
	stUartPara.stUartFlag.ucStop  = ucStopBit;
	stUartPara.stUartFlag.ucWid   = ucCharWid;

	if (0 != ioctl(iDevFd, 0x00, &stUartPara))
	{
		AEC_DRIVER_ERROR_PRINTF ("ioctl set uart para failed.\n");
		return DRIVER_ERROR_SYSTEM;
	}
	
	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  从参数集中获取Uart地址码
* Description:
* Parameter  :

*
* Return     :
*
*
* Explain    :
******************************************************************************/
int DrivCommGetUartAddrCode(int iArgc, char **ppcArgv, unsigned char *pucAddrCode)
{
	int iRet;
	char *pcDevAddr;
	unsigned char ucDevAddr;

	iRet = DrivCommGetArgv(iArgc, ppcArgv, PROTOCOL_ADDR_PARA_NAME, &pcDevAddr);
	if (DRIVER_SUCCESS != iRet)
	{
		AEC_DRIVER_ERROR_PRINTF ("can not find address code from driver argv.\n");
		return iRet;
	}

	ucDevAddr = (unsigned char) strtol(pcDevAddr, NULL, 10);
	*pucAddrCode = ucDevAddr;

	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  从参数集中获取设备文件名称
* Description:
* Parameter  :

*
* Return     :  若成功，返回设备文件名称，否则，返回NULL
*
*
* Explain    :
******************************************************************************/
char *DrivCommGetDevFileName(int iArgc, char **ppcArgv)
{
	int iRet;
	char *pcDevFile = NULL;

	iRet = DrivCommGetArgv(iArgc, ppcArgv, DEVICE_FILE_PARA_NAME, &pcDevFile);
	if (DRIVER_SUCCESS != iRet)
	{
		AEC_DRIVER_ERROR_PRINTF ("can not find device file from driver argv.\n");
		return NULL;
	}

	return pcDevFile;
}

/******************************************************************************
* Function   :  GetArgv
* Description:  此函数用于从传至驱动包的参数中提取指定参数
* Parameter  :  iArgc:参数个数（输入参数）
                ppcArgv:参数数组（输入参数）
				pcArgName:指定参数名称（输入参数）
				ppcArgValue:指定参数名称对应的参数值（输出参数）
*
* Return     :
*
*
* Explain    :
******************************************************************************/
int DrivCommGetArgv(int iArgc, char **ppcArgv, const char *pcArgName, char **ppcArgValue)
{
	int i;
	unsigned int uiLen = 0;
	int iIndex = -1;
	char *pcValue;

	if (iArgc < 1)
	{
		AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
		return DRIVER_ERROR_PARAMETER;
	}

	if (NULL == ppcArgv || NULL == pcArgName || NULL == ppcArgValue)
	{
		AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
		return DRIVER_ERROR_PARAMETER;
	}

	uiLen = strlen(pcArgName);
	if (0 == uiLen)
	{
		AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
		return DRIVER_ERROR_PARAMETER;
	}

	for (i=0; i<iArgc; i++)
	{
		if(0 == strncmp(ppcArgv[i], pcArgName, uiLen))
		{
			if (' ' == (ppcArgv[i][uiLen]))
			{
				iIndex = i;
				break;
			}
		}
	}

	if (-1 == iIndex)
	{
		AEC_DRIVER_ERROR_PRINTF ("can not find %s from driver argv.\n", pcArgName);
		return DRIVER_ERROR_DATA_NOT_FIND;
	}

	pcValue = &(ppcArgv[i][uiLen]);

	/* remove the blank from the string */

	while(' ' == *pcValue && *pcValue != '\0')
	{
		pcValue++;
	}

	*ppcArgValue = pcValue;

	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  DeviceDataWrite
* Description:  向iDevFd设备中写入uiDataLen个字节的起始位置为pucBuf的数据，
* Parameter  :  iDevFd:设备文件描述符
                pucBuf:待写入的数据
				uiDataLen:待写入的数据长度

*
* Return     :  当数据全部写入时，返回SUCCESS 0
*
*
* Explain    :  此函数会将uiDataLen长度的数据写入到iDevFd设备中，直到数据全部写入或者出错
******************************************************************************/
int DrivCommDeviceDataWrite(const int iDevFd,
				          const unsigned char *pucBuf,
				          unsigned int uiDataLen)
{
	int iRet = 0;
	unsigned int  uiLeft;
	unsigned char *pucSend;

	if(NULL == pucBuf)
	{
		AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
		return DRIVER_ERROR_PARAMETER;
	}

	DRIVER_PRINT("Enter DrivCommDeviceDataWrite\n");

	pucSend = (unsigned char*)pucBuf;
	uiLeft  = uiDataLen;

#ifdef DRIVER_DEBUG
	int iTemp;
    DRIVER_PRINT ("[%s][%s][%d] Write(%d):\n", __FILE__, __FUNCTION__, __LINE__, uiLeft);


    for (iTemp=0; iTemp<uiLeft; iTemp++)
    {
        //鎵撳嵃鍑鸿鍐欏叆鐨勬暟鎹?
        DRIVER_PRINT ("0x%02X ", pucSend[iTemp]);
    }

    DRIVER_PRINT ("\n[%s][%s][%d] Finish.\n", __FILE__, __FUNCTION__, __LINE__);
#endif // DRIVER_DEBUG

	while(uiLeft > 0)
	{
		iRet = write(iDevFd, pucSend, uiLeft);

      //  DRIVER_PRINT ("retry@ write data number to fd is[%d]\n",iRet);
        
		if (iRet <= 0)
		{
			if (errno == EINTR)
			{
				AEC_DRIVER_ERROR_PRINTF("Occur EINTR!\n");
				continue;
			}

			AEC_DRIVER_ERROR_PRINTF ("[%s][%s][%d] uart write failed,errno is %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
			return DRIVER_ERROR_WRITE;
		}

		pucSend += iRet; //???
		uiLeft  -= iRet;
	}
	
	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  DeviceDataRead
* Description:
* Parameter  :  iDevFd:设备描述符
                uiDataLen:希望读取的数据长度
				pucBuf:存放数据的Buf
				uiTimeOutMs:读超时时间，以ms为单位
				uiIntervalTimeMs:间隔时间，以ms为单位，当超出此时间认为本次接收数据完毕

*
* Return     :  读取到的字节数
*
*
* Explain    :  本函数用于从iDevFd所指定的设备中读取数据，在以下3中情况下此函数返回：
                1. 从设备中读取的字节数量达到uiDataLen个字节
				2. 在uiTimeOutMs毫秒时间内未读取到任何数据
				3. 从设备中读取到少于uiDataLen个字节的数据后，超过uiIntervalTimeMs毫秒未读取到新的数据
******************************************************************************/
int DrivCommDeviceDataRead(int iDevFd,
				          unsigned char *pucBuf,
				          unsigned int uiDataLen,
				          unsigned int uiTimeOutMs,
				          unsigned int uiIntervalTimeMs)
{
	struct timeval  stTimeOut;
	fd_set stFds;
	int    uiReadLen = 0;
	char   cEndFlag  = 0;
	int    iRet;

	if (iDevFd < 0 || NULL == pucBuf)
	{
		AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
		return DRIVER_ERROR_PARAMETER;
	}
	
	memset(&stTimeOut, 0, sizeof(stTimeOut));
	stTimeOut.tv_sec  = uiTimeOutMs/1000;
	stTimeOut.tv_usec = (uiTimeOutMs%1000)*1000;

	while(1)
	{
		FD_ZERO(&stFds);
		FD_SET(iDevFd, &stFds);

		iRet = select ((iDevFd+1), &stFds, (fd_set *)NULL, (fd_set *)NULL, &stTimeOut);
		if (0 == iRet)
		{
			DRIVER_PRINT ("[%s][%s][%d] select time out(%d).\n", __FILE__, __FUNCTION__, __LINE__, cEndFlag);
			if (1 == cEndFlag)
			{
				DRIVER_PRINT ("[%s][%s][%d] end,really time out.\n", __FILE__, __FUNCTION__, __LINE__);
				return ((int)uiReadLen);
			}
			else
			{
				DRIVER_PRINT ("[%s][%s][%d] not end,really time out.\n", __FILE__, __FUNCTION__, __LINE__);
				return DRIVER_ERROR_READ_TIMEOUT;
			}
		}
		else if (-1 == iRet)
		{
			if (errno == EINTR)
			{
				DRIVER_PRINT("select return -1 and errno == EINTR\n");
				stTimeOut.tv_sec  = uiTimeOutMs/1000;
				stTimeOut.tv_usec = (uiTimeOutMs%1000)*1000;
			    continue;
			}
			else
			{
				DRIVER_PRINT("select return -1 and errno is %d\n", errno);
		        return DRIVER_ERROR_READ;
			}
		}

		if (FD_ISSET(iDevFd, &stFds))
		{
			iRet = read(iDevFd, pucBuf+uiReadLen , uiDataLen-uiReadLen);
			if (iRet <= 0)
			{
			    DRIVER_PRINT("select read = %d\n", iRet);
				return DRIVER_ERROR_READ;
			}

			uiReadLen += iRet;

			if (uiReadLen == uiDataLen)
			{
				DRIVER_PRINT("Really Read Data:[%d]Equal to Need Read Len.\n", uiReadLen);
				return uiReadLen;
			}
			else if(uiReadLen > uiDataLen)
			{
				DRIVER_PRINT("Really Read Data:[%d]Bigger Than Need Read Len!\n", uiReadLen);
				return uiReadLen;
			}
			else
			{
				DRIVER_PRINT("Really Read Data:[%d]Smaller Than Need Read Len!\n", uiReadLen);
				stTimeOut.tv_sec  = uiIntervalTimeMs/1000;
				stTimeOut.tv_usec = (uiIntervalTimeMs%1000)*1000;
				cEndFlag = 1;
				continue;
			}
		}
	}

	DRIVER_PRINT("Exit DrivCommDeviceDataRead\n");

	return (int)uiReadLen;
}
/******************************************************************************
* Function   :  DrivCommCleanLinkCache
* Description:  此函数用于清空底层的读Buffer，确保链路干净
* Parameter  :  int iDevFd:描述符
                int iTimeSecond:时间值，单位秒，表示从当前到未来iTimeSecond秒内的数据被清空
				                最大为60，最小为1;
*
* Return     :
*
*
* Explain    :
******************************************************************************/
int DrivCommCleanLinkCache(int iDevFd, int iTimeSecond)
{
	char  acBuf[4096];
	int  iRet;
	int i;

	if (iDevFd < 0)
	{
		DRIVER_PRINT("ERROR:-----------DrivCommCleanLinkCache invalid input.\n");
		return DRIVER_ERROR_PARAMETER;
	}

	if (iTimeSecond <= 0)
	{
		iTimeSecond = 1;
	}
	else if (iTimeSecond >= 60)
	{
		iTimeSecond = 60;
	}

	for (i=0; i<iTimeSecond; i++)
	{
		sleep(1);

		while(1)
		{
		    iRet = read(iDevFd, acBuf, sizeof(acBuf));
		    if (iRet > 0)
		    {
			    DRIVER_PRINT("ERROR:-----------Driver clean Cache %d bytes.\n", iRet);
			    continue;
		    }
		    else
		    {
			    DRIVER_PRINT("ERROR:-------------Driver clean Cache return is %d.\n", iRet);
		    }

		    break;
		}
	}

	return  DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  LinkDisconnectDataConstruct
* Description:  此函数用于当从设备读取失败时构造采集点1的DO断开状态数据
* Parameter  :
*
* Return     :
*
*
* Explain    :
******************************************************************************/
int DrivCommLinkDisconnectDataConstruct(unsigned char *pucOutData, unsigned int *puiOutDataLen)
{
	unsigned int uiDataLen = 0;

	if (NULL == pucOutData || NULL == puiOutDataLen)
	{
		return DRIVER_ERROR_PARAMETER;
	}

	/* point id */

	*((unsigned short *)pucOutData) = 1;
	uiDataLen  += sizeof(unsigned short);
	pucOutData += sizeof(unsigned short);

	/* Value Type */

	*pucOutData = DATA_UCHAR;
	uiDataLen  += sizeof(unsigned char);
	pucOutData += sizeof(unsigned char);

	/* Value */

	*pucOutData = 0;
	uiDataLen  += sizeof(unsigned char);
	pucOutData += sizeof(unsigned char);

	*puiOutDataLen = uiDataLen;

	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  DrivCommLinkConnectDataConstruct
* Description:  此函数用于当从设备读取成功时构造采集点1的DO状态数据
* Parameter  :
*
* Return     :
*
*
* Explain    :
******************************************************************************/
int DrivCommLinkConnectDataConstruct(unsigned char *pucOutData, unsigned int *puiOutDataLen)
{
	unsigned char *pucTemp = NULL;

	if (NULL == pucOutData || NULL == puiOutDataLen)
	{
		return DRIVER_ERROR_PARAMETER;
	}

	pucTemp = pucOutData + (*puiOutDataLen);

	/* point id */

	*((unsigned short *)pucTemp) = 1;
	*puiOutDataLen += sizeof(unsigned short);
	pucTemp        += sizeof(unsigned short);

	/* Value Type */

	*pucTemp        = DATA_UCHAR;
	*puiOutDataLen += sizeof(unsigned char);
	pucTemp        += sizeof(unsigned char);

	/* Value */

	*pucTemp        = 1;
	*puiOutDataLen += sizeof(unsigned char);
	pucTemp        += sizeof(unsigned char);

	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  DrivCommTcpConnect
* Description:
* Parameter  :
*
* Return     :
*
*
* Explain    :
******************************************************************************/

int DrivCommTcpConnect(char *pcIpAddr, unsigned short  usPort)
{
  int iSocket;
  struct sockaddr_in s_add;
  int iFlags;
  int iRet;
  struct  timeval stTimeVal;
  fd_set hReadSet, hWriteSet;
  int iLen;
  int iError;

  if(NULL == pcIpAddr)
  {
  	DRIVER_PRINT("DrivCommTcpConnect input IP Addr is wrong!\n");
  	return DRIVER_ERROR_PARAMETER;
  }

  iSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(-1 == iSocket)
  {
    DRIVER_PRINT("socket fail ! \r\n");
    return -1;
  }

  bzero(&s_add,sizeof(struct sockaddr_in));
  s_add.sin_family=AF_INET;
  s_add.sin_addr.s_addr= inet_addr(pcIpAddr);
  s_add.sin_port=htons(usPort);

  iFlags = fcntl(iSocket, F_GETFL, 0);
  fcntl(iSocket, F_SETFL, iFlags | O_NONBLOCK);
  iError = 0;

  if((iRet = connect(iSocket, (struct sockaddr *)&s_add, sizeof(struct sockaddr))) < 0)
  {
    if(errno != EINPROGRESS)
    {
      DRIVER_PRINT("connect failed.\n");
      close(iSocket);
      return -1;
    }
    else
    {
      DRIVER_PRINT("connect send.\n");
    }
  }
  else
  {
	  DRIVER_PRINT("connect success---------------.\n");
  }

  if(iRet != 0)
  {
    FD_ZERO(&hReadSet);
    FD_SET(iSocket, &hReadSet);
    hWriteSet = hReadSet;
    stTimeVal.tv_sec = 3;
    stTimeVal.tv_usec = 0;

    iRet = select(iSocket+1, &hReadSet, &hWriteSet, NULL, &stTimeVal);
    if (0 == iRet)
	  {
		  DRIVER_PRINT ("select time out.\n");
		  close(iSocket);
		  return -1;
	  }

	  else if (-1 == iRet)
	  {
		  DRIVER_PRINT("select failed2.\n");
		  close(iSocket);
		  return -1;
	  }

    if(FD_ISSET(iSocket, &hWriteSet))
    {
      iLen = sizeof(iError);
      if(getsockopt (iSocket, SOL_SOCKET, SO_ERROR, &iError, (socklen_t *)&iLen) < 0)
      {
		close(iSocket);
		return -1;
      }
      else if(0 != iError)
      {
        DRIVER_PRINT("socket error.\n");
        close(iSocket);
		return -1;
      }
      else
      {
        DRIVER_PRINT("select connect success.\n");
      }
    }
  }

  fcntl(iSocket, F_SETFL, iFlags);

  return iSocket;
}

/******************************************************************************
* Function    : DrivCommTcpConnectTimeOut
* Description :
*               server: receive data from all SU
* Parameter   :
*               NO
* Return      : NO
* Explain     : Leander.zhang add this function in 20151023
*******************************************************************************/

int DrivCommTcpConnectTimeOut(const char *pcIP, unsigned short usPort, unsigned int uiTimeOutSecond)
{
	///////////////////////////////////////////////////////////////////////////
	// define temp parameter
	int			iReturn			= 0 ;			// func-call return value
	int			iSocket			= -1;			// function will return it
	struct		sockaddr_in		stAddr;
	struct		timeval			stTimeOut = {uiTimeOutSecond, 0};
	socklen_t	hStructTimeLen	= sizeof(stTimeOut);
    struct		hostent			*stHp;			// server info.
	int			iFlags;

	///////////////////////////////////////////////////////////////////////////
	// check parameters
	if (NULL == pcIP || usPort == 0)
	{
		return DRIVER_ERROR_PARAMETER;
	}

	DRIVER_PRINT("[%s][%d] prepare to connect (%s:%d)\n", __FUNCTION__, __LINE__, pcIP, usPort);
	///////////////////////////////////////////////////////////////////////////
	// check the ip-addr
	stHp = gethostbyname(pcIP);
	if (NULL == stHp)
	{
		res_init();
		DRIVER_PRINT("[%s][%d] Unknown server:%s.\n", __FUNCTION__, __LINE__, pcIP);
		return DRIVER_ERROR_PARAMETER;
	}
	memset((char *)&stAddr, 0, sizeof(stAddr));
	memcpy((char *)&stAddr.sin_addr, stHp->h_addr, stHp->h_length);
	stAddr.sin_family = AF_INET;
	stAddr.sin_port   = htons(usPort);

	DRIVER_PRINT("[%s][%d] create socket and connect.\n", __FUNCTION__, __LINE__);
	///////////////////////////////////////////////////////////////////////////
	// create socket file
	iSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (iSocket <= 0)
	{
		DRIVER_PRINT("[%s][%d] create socket failed.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_SOCK_CREATE;
	}
	// set socket timeout
	setsockopt(iSocket, SOL_SOCKET, SO_SNDTIMEO, &stTimeOut, hStructTimeLen);
    // connect 
	iReturn = connect(iSocket, (struct sockaddr *)&stAddr, sizeof(stAddr));
	if (iReturn != 0)
	{
		DRIVER_PRINT("errorno is %d\n", errno);
		DRIVER_PRINT("[%s][%d] socket connect failed.\n", __FUNCTION__, __LINE__);		
		close(iSocket);
		iSocket = -1;
		return DRIVER_ERROR_SOCK_CONNECT;
	}

	// set sock nonblock
	iFlags = fcntl(iSocket, F_GETFL, 0);
	fcntl(iSocket, F_SETFL, iFlags | O_NONBLOCK);

	DRIVER_PRINT("[%s][%d] sock create and connect success: %d.\n", __FUNCTION__, __LINE__, iSocket);
	
	return iSocket;
}

/******************************************************************************
* Function    : DrivCommTcpAcceptTimeout
* Description :
*               server: receive data from all SU
* Parameter   :
*               NO
* Return      : NO
* Explain     : Leander.zhang add this function in 20151024
*******************************************************************************/
int DrivCommTcpAcceptTimeout(int iServerSock, unsigned int uiSeconds)
{
	///////////////////////////////////////////////////////////////////////////
	// define temp parameter
	int			i;
	int			iReturn;
	struct		timeval			stRecvTimeout = {uiSeconds, 0};
	fd_set		stAcceptFds;
	int			iClientSock		= -1;				// this func will return it
	struct		sockaddr_in		stClientAddr;
	int			iClientLen;
	int			iFlags;	

	///////////////////////////////////////////////////////////////////////////
	// check parameters
	if (iServerSock <= 0)
	{
		DRIVER_PRINT("[%s][%d] wrong parameters.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_PARAMETER;
	}
	
	DRIVER_PRINT("[%s][%d] prepare to accept.\n",__FUNCTION__, __LINE__);
	///////////////////////////////////////////////////////////////////////////
	// select and accept
	FD_ZERO(&stAcceptFds);
	FD_SET(iServerSock, &stAcceptFds);
	// select
	iReturn = select(FD_SETSIZE, &stAcceptFds, (fd_set*)0, (fd_set*)0, (struct timeval*)&stRecvTimeout);
	if (iReturn < 1) // == 0 or == -1
	{
		FD_ZERO(&stAcceptFds);
		perror("select:");
		DRIVER_PRINT("[%s][%d]: select timeout and return %d.\n", __FUNCTION__, __LINE__, iReturn);
		return DRIVER_ERROR_SOCK_SELECT_TIME_OUT;
	}

	DRIVER_PRINT("[%s][%d] there are client connecting...\n", __FUNCTION__, __LINE__);
	///////////////////////////////////////////////////////////////////////////
	// accept
	for (i=0; i<FD_SETSIZE; i++)
	{
		if (FD_ISSET(i, &stAcceptFds))
		{
			if (i == iServerSock)
			{
				iClientLen = sizeof(stClientAddr);
				iClientSock = accept(iServerSock, (struct sockaddr*)&stClientAddr, (unsigned *)&iClientLen);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// return
	if (iClientSock > 0)
	{
		// set clientsock nonblock
		iFlags = fcntl(iClientSock, F_GETFL, 0);
		fcntl(iClientSock, F_SETFL, iFlags | O_NONBLOCK);
	}

	DRIVER_PRINT("[%s][%d] accept return %d.\n", __FUNCTION__, __LINE__, iClientSock);	
	
	return iClientSock;
}

/******************************************************************************
* Function    : DrivCommTcpServerCreate
* Description :
*               create tcp server-socket base on Port
* Parameter   :
*               NO
* Return      : NO
* Explain     : Leander.zhang add this function in 20151024
*******************************************************************************/
static void GetLocalIp(unsigned long   *pOutIpAddress)  
{  
    int   fd;    
    struct   ifreq   ifr;     
    struct   sockaddr_in*   sin;      
    fd   =   socket(PF_INET,   SOCK_DGRAM,   0);     
    memset(&ifr,   0x00,   sizeof(ifr));     
    strcpy(ifr.ifr_name,   "eth0");     
    ioctl(fd,   SIOCGIFADDR,   &ifr);     
    close(fd);     
    sin   =   (struct   sockaddr_in*   )&ifr.ifr_addr;    
    *pOutIpAddress = sin->sin_addr.s_addr;  
} 

int  DrivCommTcpServerCreate(unsigned short usPort, unsigned short usListenNum)
{
	///////////////////////////////////////////////////////////////////////////
	// define temp parameter
	int         iRes;                           // somefunctions' return-vlaue
	int         iOptValue = 1;
	int         iServerSockfd;                  // socket ID
	int         iServerLen;                     // these sockaddr_in length
	struct      sockaddr_in stServerAddress;    // define server socke-address
	unsigned long ulLocalIpAddr = 0;

	///////////////////////////////////////////////////////////////////////////
	// check parameters
	if ( (usPort == 0) || (usListenNum == 0) )
	{
		return DRIVER_ERROR_PARAMETER;
	}

	GetLocalIp(&ulLocalIpAddr);
	DRIVER_PRINT("get local ip is %ld.\n", ulLocalIpAddr);
	///////////////////////////////////////////////////////////////////////////
	// create socket
	iServerSockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (iServerSockfd <= 0)
	{
		perror("built socket failed!\n");
		DRIVER_PRINT("[%s][%d] socket create failed.\n",__FUNCTION__, __LINE__);
		return DRIVER_ERROR_SOCK_CREATE;
	}
	setsockopt(iServerSockfd, SOL_SOCKET, SO_REUSEADDR, &iOptValue, sizeof(iOptValue));

	//initalize struct: server_address;
	stServerAddress.sin_family       = AF_INET;
	stServerAddress.sin_addr.s_addr  = htonl(INADDR_ANY); // IP
	//stServerAddress.sin_addr.s_addr  = htonl(ulLocalIpAddr); // IP
	stServerAddress.sin_port         = htons(usPort);      // port
	memset(stServerAddress.sin_zero, 0, sizeof(stServerAddress.sin_zero));	
	// set sock nonblock
	/*
	iFlags = fcntl(iServerSockfd, F_GETFL, 0);
	fcntl(iServerSockfd, F_SETFL, iFlags | O_NONBLOCK);*/

	// bind
	iServerLen = sizeof(stServerAddress);	
	iRes = bind(iServerSockfd, (struct sockaddr *)&stServerAddress, iServerLen);
	if (0 != iRes)
	{
		DRIVER_PRINT("[%s][%d] socket bind failed. sock:%d, port:%d, err: %d.\n", __FUNCTION__, __LINE__, iServerSockfd, usPort, errno);
		perror("bind failed leander");	
		close (iServerSockfd);              // close socket
		return DRIVER_ERROR_SOCK_BIND;
	}

	// set the max length of connection queue = 5
	iRes = listen(iServerSockfd, usListenNum);
	if (0 != iRes)
	{
		perror("listen failed!\n");
		DRIVER_PRINT("[%s][%d] socket listen failed. sock:%d, listen:%d.\n", __FUNCTION__, __LINE__, iServerSockfd, usListenNum);
		close (iServerSockfd);
		return DRIVER_ERROR_SOCK_LISTEN;//leander.zhang 20151020
	}

	DRIVER_PRINT("[%s][%d] create server socket success SOCK:%d.\n", __FUNCTION__, __LINE__, iServerSockfd);
	
	return iServerSockfd;
}

/******************************************************************************
* Function    : DrivCommTcpSockRecvBufClean
* Description :
*               empty the recv buffer
* Parameter   :
*               NO
* Return      : NO
* Explain     : Leander.zhang add this function in 20151024
*******************************************************************************/
int  DrivCommTcpSockRecvBufClean (int iSockFd)
{
	///////////////////////////////////////////////////////////////////////////
	// define temp parameter
	int		iReturn;
	char	acRecvBuf[1024];
	int		iFlags;

	///////////////////////////////////////////////////////////////////////////
	// check parameters
	if (iSockFd <= 0)
	{
		return DRIVER_ERROR_PARAMETER;
	}

	///////////////////////////////////////////////////////////////////////////
	// set iSockFd is non-block
	iFlags = fcntl(iSockFd, F_GETFL, 0);
	fcntl(iSockFd, F_SETFL, iFlags | O_NONBLOCK);

	///////////////////////////////////////////////////////////////////////////
	// start to empty
	while(1)
	{
		iReturn = 0;
		memset(acRecvBuf, 0, sizeof(acRecvBuf));
		iReturn = recv(iSockFd, acRecvBuf, sizeof(acRecvBuf), 0);
		if (-1 == iReturn)
		{
			if (errno == EAGAIN)
			{
				DRIVER_PRINT("[%s][%d]: success clean the recv buffer.\n", __FUNCTION__, __LINE__);
				return DRIVER_SUCCESS;
			}
			else if (errno == EINTR)
			{
				continue;
			}
			else
			{
				perror("SockRecv failed: \n");
				DRIVER_PRINT("[%s][%d]: recv return -1.\n", __FUNCTION__, __LINE__);
				return DRIVER_ERROR_SOCK_RECV;
			}
		}
		else if (0 == iReturn)
		{
			DRIVER_PRINT("[%s][%d]: recv return 0.\n", __FUNCTION__, __LINE__);
			return DRIVER_ERROR_SOCK_RECV;
		}
		DRIVER_PRINT("[%s][%d]: success discard %d Bytes.\n", __FUNCTION__, __LINE__, iReturn);
	}

	DRIVER_PRINT("[%s][%d]: fatal error.\n", __FUNCTION__, __LINE__);
	
	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  DrivCommTcpRestart
* Description:
* Parameter  :
*
* Return     :
*
*
* Explain    :
******************************************************************************/
int DrivCommTcpRestart(int iSocket, char *pcIpAddr, unsigned short  usPort)
{
	int iNewSocket;

	if(iSocket < 0)
	{
		DRIVER_PRINT("DrivCommTcpRestart input parameter is wrong!\n");
  	return DRIVER_ERROR_PARAMETER;
	}

	if(NULL == pcIpAddr)
  {
  	DRIVER_PRINT("DrivCommTcpConnect input IP Addr is wrong!\n");
  	return DRIVER_ERROR_PARAMETER;
  }

	close(iSocket);

	sleep(2);

	iNewSocket = DrivCommTcpConnect(pcIpAddr, usPort);

	if(iNewSocket < 0)
	{
		DRIVER_PRINT("Driver connect %s failed.\n", pcIpAddr);
	}

	return iNewSocket;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////TCP-TPKT-ISO//////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int DrivCommTcpISOConnectTimeOut(const char *pcIP, 
                                 unsigned short usPort, 
                                 unsigned int uiTimeOutSecond,
                                 unsigned short usLocalTSAP,         // C2
                                 unsigned short usRemoteTSAP)        // C1
{
	int iClientSocket;
	int iRet;
	unsigned char aucRecvBuf[64] = { 0 };
	unsigned char aucSendBuf[64] = { 0 }; 
	unsigned short usRecvLen = 0;
	unsigned short usSendLen = 0;
	ST_TCP_IP_TPKT			*pstTPKTSend	= NULL;
	ST_TCP_IP_TPKT			*pstTPKTRecv	= NULL;
	ST_TCP_IP_ISO_CR		*pstISOSend		= NULL;
	ST_TCP_IP_ISO_CC		*pstISORecv		= NULL;

	///////////////////////////////////////////////////////////////////////////
	// init
	pstTPKTSend = (ST_TCP_IP_TPKT*)aucSendBuf;
	pstISOSend	= (ST_TCP_IP_ISO_CR*)(aucSendBuf + sizeof(ST_TCP_IP_TPKT));
	usSendLen	= sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_CR);

	pstTPKTRecv = (ST_TCP_IP_TPKT*)aucRecvBuf;
	pstISORecv	= (ST_TCP_IP_ISO_CC*)(aucRecvBuf + sizeof(ST_TCP_IP_TPKT));
	usRecvLen	= sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_CC);

	///////////////////////////////////////////////////////////////////////////
	// TCP socket connect firstly.
	iClientSocket = DrivCommTcpConnectTimeOut(pcIP, usPort, uiTimeOutSecond);
	if (iClientSocket <= 0)
	{
		DRIVER_PRINT("[%s][%d] TCP connect failed.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_SOCK_CONNECT;
	}

	///////////////////////////////////////////////////////////////////////////
	// packet the CR 
	// TPKT 
	pstTPKTSend->ucVersion	= TCP_TPKT_VERSION;
	pstTPKTSend->ucReserved = 0;
	pstTPKTSend->usDataLen	= htons(usSendLen);

	// ISO CR
	pstISOSend->ucLen		= sizeof(ST_TCP_IP_ISO_CR)-1;
	pstISOSend->ucCode		= TCP_ISO_CODE_CR;
	pstISOSend->usDstRef	= 0x0000;
	pstISOSend->usSrcRef	= htons(TCP_ISO_SRC_REF);
	pstISOSend->ucClass		= 0;

	pstISOSend->ucP1_code	= TCP_ISO_PARAM_1;
	pstISOSend->ucP1_len	= 0x02;
	pstISOSend->usP1		= htons(usRemoteTSAP);

	pstISOSend->ucP2_code	= TCP_ISO_PARAM_2;
	pstISOSend->ucP2_len	= 0x02;
	pstISOSend->usP2		= htons(usLocalTSAP);

	pstISOSend->ucP0_code	= TCP_ISO_PARAM_0;
	pstISOSend->ucP0_len	= 0x01;
	pstISOSend->ucP0		= TCP_ISO_TPDU_SIZE_512B;


	///////////////////////////////////////////////////////////////////////////
	// send CR packet
	iRet = DrivCommDeviceDataWrite(iClientSocket, aucSendBuf, usSendLen);
	if (iRet != DRIVER_SUCCESS)
	{
		DRIVER_PRINT("[%s][%d] TCP connect send CR failed.\n", __FUNCTION__, __LINE__);
		
		shutdown(iClientSocket, 2);
		close(iClientSocket);
		return DRIVER_ERROR_SOCK_SEND;
	}

	///////////////////////////////////////////////////////////////////////////
	// recv CC packet
	memset(aucRecvBuf, 0, sizeof(aucRecvBuf));
	iRet = DrivCommDeviceDataRead(iClientSocket, aucRecvBuf, usRecvLen, 2000, 1000);
	if ((iRet <= 0 ) || (iRet != usRecvLen))
	{
		DRIVER_PRINT("[%s][%d] TCP connect recv CC failed.%d\n", __FUNCTION__, __LINE__, iRet);
		
		// send DR and exit
		pstISOSend->ucCode		= TCP_ISO_CODE_DR;
		DrivCommDeviceDataWrite(iClientSocket, aucSendBuf, usSendLen);
		
		shutdown(iClientSocket, 2);
		close(iClientSocket);
		return DRIVER_ERROR_SOCK_RECV;
	}

	///////////////////////////////////////////////////////////////////////////
	// check the recv packet
	if ((pstISORecv->ucCode != TCP_ISO_CODE_CC) ||
		(pstISORecv->usDstRef != pstISOSend->usSrcRef))
	{
		DRIVER_PRINT("[%s][%d] TCP connect CC check failed.\n", __FUNCTION__, __LINE__);
		
		// send DR and exit
		pstISOSend->ucCode		= TCP_ISO_CODE_DR;
		DrivCommDeviceDataWrite(iClientSocket, aucSendBuf, usSendLen);
		
		shutdown(iClientSocket, 2);
		
		close(iClientSocket);
		return DRIVER_ERROR_SOCK_RECV;
	}

	return iClientSocket;
}

int DrivCommTcpISODisconnect(int iSocket, unsigned short usLocalTSAP, unsigned short usRemoteTSAP)
{
	int iRet;
	unsigned char aucSendBuf[64] = { 0 }; 
	unsigned short usSendLen = 0;
	ST_TCP_IP_TPKT			*pstTPKTSend	= NULL;
	ST_TCP_IP_ISO_CR		*pstISOSend		= NULL;	
	
	if (iSocket <= 0)
	{
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// init
	pstTPKTSend = (ST_TCP_IP_TPKT*)aucSendBuf;
	pstISOSend	= (ST_TCP_IP_ISO_CR*)(aucSendBuf + sizeof(ST_TCP_IP_TPKT));
	usSendLen	= sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_CR);		

	///////////////////////////////////////////////////////////////////////////
	// packet the DR 
	// TPKT 
	pstTPKTSend->ucVersion	= TCP_TPKT_VERSION;
	pstTPKTSend->ucReserved = 0;
	pstTPKTSend->usDataLen	= htons(usSendLen);

	// ISO CR
	pstISOSend->ucLen		= sizeof(ST_TCP_IP_ISO_CR)-1;
	pstISOSend->ucCode		= TCP_ISO_CODE_DR;
	pstISOSend->usDstRef	= 0x0000;
	pstISOSend->usSrcRef	= htons(TCP_ISO_SRC_REF);
	pstISOSend->ucClass		= 0;

	pstISOSend->ucP1_code	= TCP_ISO_PARAM_1;
	pstISOSend->ucP1_len	= 0x02;
	pstISOSend->usP1		= htons(usRemoteTSAP);

	pstISOSend->ucP2_code	= TCP_ISO_PARAM_2;
	pstISOSend->ucP2_len	= 0x02;
	pstISOSend->usP2		= htons(usLocalTSAP);

	pstISOSend->ucP0_code	= TCP_ISO_PARAM_0;
	pstISOSend->ucP0_len	= 0x01;
	pstISOSend->ucP0		= TCP_ISO_TPDU_SIZE_512B;

	///////////////////////////////////////////////////////////////////////////
	// send CR packet
	iRet = DrivCommDeviceDataWrite(iSocket, aucSendBuf, usSendLen);	
	if (iRet != DRIVER_SUCCESS)
	{
		DRIVER_PRINT("[%s][%d] TCP connect send DR failed.\n", __FUNCTION__, __LINE__);		
	}

	shutdown(iSocket, 2);
	close(iSocket);
	
	return DRIVER_SUCCESS;
}

int DrivCommTcpISODataWrite(const int iDevFd, const unsigned char *pucBuf, unsigned int uiDataLen)
{
	int iRet;
	unsigned char	aucSendBuf[TCP_ISO_TPDU_MAX_SIZE + sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM)+1] = { 0 };
	unsigned short	usSendDataLen = 0;

	ST_TCP_IP_TPKT		*pstTPKT = NULL;
	ST_TCP_IP_ISO_COM	*pstISO = NULL;

	///////////////////////////////////////////////////////////////////////////
	// check input parameter
	if (uiDataLen > TCP_ISO_TPDU_MAX_SIZE)
	{
		DRIVER_PRINT("[%s][%d] packet is too long.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_DATA_TOO_LONG;
	}
	if (iDevFd <= 0 || NULL == pucBuf)
	{
		DRIVER_PRINT("[%s][%d] packet is too long.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_PARAMETER;
	}

	///////////////////////////////////////////////////////////////////////////
	// init
	pstTPKT = (ST_TCP_IP_TPKT*)aucSendBuf;
	pstISO	= (ST_TCP_IP_ISO_COM*)(aucSendBuf + sizeof(ST_TCP_IP_TPKT));
	usSendDataLen = sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM)+uiDataLen;

	///////////////////////////////////////////////////////////////////////////
	// packet
	pstTPKT->ucVersion	= TCP_TPKT_VERSION;
	pstTPKT->ucReserved = 0;
	pstTPKT->usDataLen	= htons(usSendDataLen);

	pstISO->ucLen		= 0x02;
	pstISO->ucCode		= TCP_ISO_CODE_DT;
	pstISO->ucNumEOT	= 0x80;

	memcpy(aucSendBuf + sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM), pucBuf, uiDataLen);

	///////////////////////////////////////////////////////////////////////////
	// send data
	iRet = DrivCommDeviceDataWrite(iDevFd, aucSendBuf, usSendDataLen);

	return iRet;
}


int DrivCommTcpISODataRead(int iDevFd, unsigned char *pucBuf, unsigned int uiDataLen, unsigned int uiTimeOutMs, unsigned int uiIntervalTimeMs)
{
	int iRet;
	ST_TCP_IP_TPKT		*pstTPKT = NULL;
	ST_TCP_IP_ISO_COM	*pstISO = NULL;
	unsigned char		*pucUserData = NULL;
	unsigned int		uiUserDataLen = 0;
	unsigned char		aucRecvBuf[TCP_ISO_TPDU_MAX_SIZE + sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM)+1] = { 0 };

	pstTPKT		= (ST_TCP_IP_TPKT*)aucRecvBuf;
	pstISO		= (ST_TCP_IP_ISO_COM*)(aucRecvBuf + sizeof(ST_TCP_IP_TPKT));
	pucUserData = aucRecvBuf + sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM);

	///////////////////////////////////////////////////////////////////////////
	// check input parameter
	if (uiDataLen > TCP_ISO_TPDU_MAX_SIZE)
	{
		DRIVER_PRINT("[%s][%d] read too long.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_DATA_TOO_LONG;
	}

	///////////////////////////////////////////////////////////////////////////
	// read header (TPKT+ISO)
	iRet = DrivCommDeviceDataRead(iDevFd, aucRecvBuf, sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM), uiTimeOutMs, uiIntervalTimeMs);
	if (iRet != (sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM)))
	{
		DRIVER_PRINT("[%s][%d] ISO header read failed.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_SOCK_RECV;
	}

	//////////////////////////////////////////////////////////////////////////////
	// check data
	if ((pstTPKT->ucVersion		!= TCP_TPKT_VERSION) ||
		(pstTPKT->ucReserved	!= 0			   ) ||
		(pstISO->ucLen			!= 0x02			   ) ||
		(pstISO->ucCode			!= TCP_ISO_CODE_DT ) ||
		(pstISO->ucNumEOT		!= 0x80			   ) )
	{
		DRIVER_PRINT("[%s][%d] ISO read check failed.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_SOCK_RECV;
	}

	uiUserDataLen  = (unsigned int)(ntohs(pstTPKT->usDataLen));
	uiUserDataLen -= sizeof(ST_TCP_IP_TPKT)+sizeof(ST_TCP_IP_ISO_COM);
	uiUserDataLen  = (uiUserDataLen>uiDataLen)?(uiDataLen):(uiUserDataLen);

	///////////////////////////////////////////////////////////////////////////
	// read user data
	iRet = DrivCommDeviceDataRead(iDevFd, pucUserData, uiUserDataLen, uiTimeOutMs, uiIntervalTimeMs);
	if (iRet != uiUserDataLen)
	{
		DRIVER_PRINT("[%s][%d] ISO header read failed.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_SOCK_RECV;
	}
	///////////////////////////////////////////////////////////////////////////
	// return data
	memcpy(pucBuf, pucUserData, iRet);

	return iRet;
}


int	Str2Hex	(unsigned int *uiDest, unsigned char *pucSrc, unsigned char ucSrcSize)
{
	unsigned char	ucNum  = 0;
	unsigned int	uiTemp = 0;

	if ( (NULL == uiDest) || (NULL == pucSrc) || (ucSrcSize == 0) || (ucSrcSize > 2*sizeof(unsigned int)) )
	{
		return -1;
	}

	for (ucNum=0; ucNum<ucSrcSize; ucNum++)
	{
		uiTemp <<= 4;

		if ( (*(pucSrc+ucNum) >= '0') && (*(pucSrc+ucNum) <= '9') )
		{
			uiTemp += *(pucSrc+ucNum) - '0';
		}
		else if ( (*(pucSrc+ucNum) >= 'a') && (*(pucSrc+ucNum) <= 'f') )
		{
			uiTemp += 10 + *(pucSrc+ucNum) - 'a';
		}
		else if ( (*(pucSrc+ucNum) >= 'A') && (*(pucSrc+ucNum) <= 'F') )
		{
			uiTemp += 10 + *(pucSrc+ucNum) - 'A';
		}
		else
		{
			DRIVER_PRINT("[%s][%d] wrong String(HEX) fromat!\n", __FUNCTION__, __LINE__);
			return -1;
		}
	}

	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function    : DrivCommTcpSockRecvBufClean
* Description :
*               empty the recv buffer
* Parameter   :
*               NO
* Return      : NO
* Explain     : Leander.zhang add this function in 20151024
*******************************************************************************/
int  DrivCommUdpSockRecvBufClean (int iSockFd, struct	sockaddr_in *pstAddr)
{
	///////////////////////////////////////////////////////////////////////////
	// define temp parameter
	int		iReturn;
	char	acRecvBuf[100];
	int		iLen = 0;
	
	///////////////////////////////////////////////////////////////////////////
	// check parameters
	if (iSockFd <= 0)
	{
		DRIVER_PRINT("Wrong Parameter!\n");
		return DRIVER_ERROR_PARAMETER;
	}

	///////////////////////////////////////////////////////////////////////////
	// start to empty
	while(1)
	{
		iReturn = 0;
		memset(acRecvBuf, 0, sizeof(acRecvBuf));
		iReturn = recvfrom(iSockFd, acRecvBuf, sizeof(acRecvBuf), 0, (struct sockaddr *)pstAddr, (socklen_t *)&iLen);
		if (-1 == iReturn)
		{
			if (errno == EAGAIN)
			{
				DRIVER_PRINT("[%s][%d]: success clean the recv buffer.\n", __FUNCTION__, __LINE__);
				return DRIVER_SUCCESS;
			}
			else if (errno == EINTR)
			{
				DRIVER_PRINT("[%s][%d]: error is EINTR.\n", __FUNCTION__, __LINE__);
				continue;
			}
			else
			{
				perror("SockRecv failed: \n");
				DRIVER_PRINT("[%s][%d]: recv return -1.\n", __FUNCTION__, __LINE__);
				return DRIVER_ERROR_SOCK_RECV;
			}
		}
		else if (0 == iReturn)
		{
			DRIVER_PRINT("[%s][%d]: recv return 0.\n", __FUNCTION__, __LINE__);
			return DRIVER_ERROR_SOCK_RECV;
		}
		DRIVER_PRINT("[%s][%d]: success discard %d Bytes.\n", __FUNCTION__, __LINE__, iReturn);
	}

	DRIVER_PRINT("[%s][%d]: fatal error.\n", __FUNCTION__, __LINE__);
	
	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function    : DrivCommUdpConnectTimeOut
* Description :
*               server: receive data from all SU
* Parameter   :
*               NO
* Return      : NO
* Explain     : Leander.zhang add this function in 20151023
*******************************************************************************/
int DrivCommUdpConnectTimeOut(const char *pcIP, unsigned short usPort, struct	sockaddr_in *pstAddr)
{
	///////////////////////////////////////////////////////////////////////////
	// define temp parameter
	int			iSocket			= -1;			// function will return it
	int			iFlags			= -1;

	///////////////////////////////////////////////////////////////////////////
	// check parameters
	if (NULL == pcIP || usPort == 0)
	{
		DRIVER_PRINT("param error!\n");
		return DRIVER_ERROR_PARAMETER;
	}

	DRIVER_PRINT("[%s][%d] prepare to connect (%s:%d)\n", __FUNCTION__, __LINE__, pcIP, usPort);
	
	///////////////////////////////////////////////////////////////////////////
	// create socket file
	iSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (iSocket <= 0)
	{
		DRIVER_PRINT("[%s][%d] create socket failed.\n", __FUNCTION__, __LINE__);
		return DRIVER_ERROR_SOCK_CREATE;
	}

	///////////////////////////////////////////////////////////////////////////
	// set iSockFd is non-block
	iFlags = fcntl(iSocket, F_GETFL, 0);
	fcntl(iSocket, F_SETFL, iFlags | O_NONBLOCK);

	pstAddr->sin_family = AF_INET;
	pstAddr->sin_port   = htons(usPort);
	pstAddr->sin_addr.s_addr = inet_addr(pcIP);
	
	DRIVER_PRINT("[%s][%d] sock create and connect success: %d.\n", __FUNCTION__, __LINE__, iSocket);
	
	return iSocket;
}

/******************************************************************************
* Function   :  DeviceDataWrite
* Description:  向iDevFd设备中写入uiDataLen个字节的起始位置为pucBuf的数据，
* Parameter  :  iDevFd:设备文件描述符
                pucBuf:待写入的数据
				uiDataLen:待写入的数据长度

*
* Return     :  当数据全部写入时，返回SUCCESS 0
*
*
* Explain    :  此函数会将uiDataLen长度的数据写入到iDevFd设备中，直到数据全部写入或者出错
******************************************************************************/
int DrivCommUdpDeviceDataWrite(const int iDevFd,
				          const unsigned char *pucBuf,
				          unsigned int uiDataLen,
				          struct sockaddr_in *pstAddr)
{
	int iRet = 0;
	unsigned int  uiLeft;
	unsigned char *pucSend;

	if(NULL == pucBuf)
	{	
		DRIVER_PRINT("param error!!!!\n");
		return DRIVER_ERROR_PARAMETER;
	}

	pucSend = (unsigned char*)pucBuf;
	uiLeft  = uiDataLen;

#ifdef DRIVER_DEBUG
	int iTemp;
    DRIVER_PRINT ("[%s][%s][%d] Write(%d):\n", __FILE__, __FUNCTION__, __LINE__, uiLeft);
    for (iTemp=0; iTemp<uiLeft; iTemp++)
    {
        DRIVER_PRINT ("0x%02X ", pucSend[iTemp]);
    }
    DRIVER_PRINT ("\n[%s][%s][%d] Finish.\n", __FILE__, __FUNCTION__, __LINE__);
#endif // DRIVER_DEBUG

	while(uiLeft > 0)
	{
		iRet = sendto(iDevFd, pucSend, uiLeft, 0, (struct sockaddr *)pstAddr, sizeof(struct sockaddr));
		if (iRet <= 0)
		{
			if (errno == EINTR)
			{
				continue;
			}

			DRIVER_PRINT ("[%s][%s][%d] uart write failed.\n", __FILE__, __FUNCTION__, __LINE__);

			return DRIVER_ERROR_WRITE;
		}

		pucSend += iRet;
		uiLeft  -= iRet;
	}

	return DRIVER_SUCCESS;
}

/******************************************************************************
* Function   :  DeviceDataRead
* Description:
* Parameter  :  iDevFd:设备描述符
                uiDataLen:希望读取的数据长度
				pucBuf:存放数据的Buf
				uiTimeOutMs:读超时时间，以ms为单位
				uiIntervalTimeMs:间隔时间，以ms为单位，当超出此时间认为本次接收数据完毕

*
* Return     :  读取到的字节数
*
*
* Explain    :  本函数用于从iDevFd所指定的设备中读取数据，在以下3中情况下此函数返回：
                1. 从设备中读取的字节数量达到uiDataLen个字节
				2. 在uiTimeOutMs毫秒时间内未读取到任何数据
				3. 从设备中读取到少于uiDataLen个字节的数据后，超过uiIntervalTimeMs毫秒未读取到新的数据
******************************************************************************/
int DrivCommUdpDeviceDataRead(int iDevFd,
				          unsigned char *pucBuf,
				          unsigned int uiDataLen,
				          unsigned int uiTimeOutMs,
				          unsigned int uiIntervalTimeMs,
				          struct sockaddr_in *pstAddr)
{
	struct timeval  stTimeOut;
	fd_set stFds;
	int    uiReadLen = 0;
	char   cEndFlag  = 0;
	int    iRet;
	int	   iLen;

	if (iDevFd < 0 || NULL == pucBuf)
	{
		return DRIVER_ERROR_PARAMETER;
	}
	DRIVER_PRINT("Enter DrivCommUdpDeviceDataRead\n");

	memset(&stTimeOut, 0, sizeof(stTimeOut));

	stTimeOut.tv_sec  = uiTimeOutMs/1000;
	stTimeOut.tv_usec = (uiTimeOutMs%1000)*1000;

	while(1)
	{
		FD_ZERO(&stFds);
		FD_SET(iDevFd, &stFds);

		iRet = select ((iDevFd+1), &stFds, (fd_set *)NULL, (fd_set *)NULL, &stTimeOut);
		if (0 == iRet)
		{
			DRIVER_PRINT ("[%s][%s][%d] select time out(%d).\n", __FILE__, __FUNCTION__, __LINE__, cEndFlag);
			if (1 == cEndFlag)
			{
				return ((int)uiReadLen);
			}
			else
			{
				return DRIVER_ERROR_READ_TIMEOUT;
			}
		}
		else if (-1 == iRet)
		{
			if (errno == EINTR)
			{
				stTimeOut.tv_sec  = uiTimeOutMs/1000;
				stTimeOut.tv_usec = (uiTimeOutMs%1000)*1000;

				DRIVER_PRINT("select return -1 and errno == EINTR\n");

			    continue;
			}
			else
			{
		        return DRIVER_ERROR_READ;
			}
		}

		if (FD_ISSET(iDevFd, &stFds))
		{
			iRet = recvfrom(iDevFd, pucBuf+uiReadLen , uiDataLen - uiReadLen, 0, (struct sockaddr *)pstAddr, (socklen_t *)&iLen);
			if (iRet <= 0)
			{
			    perror("read");
			    DRIVER_PRINT("select read = %d\n", iRet);
				return DRIVER_ERROR_READ;
			}

			uiReadLen += iRet;

			if (uiReadLen >= uiDataLen)
			{
				return uiReadLen;
			}
			else
			{
				stTimeOut.tv_sec  = uiIntervalTimeMs/1000;
				stTimeOut.tv_usec = (uiIntervalTimeMs%1000)*1000;
				cEndFlag = 1;
				continue;
			}
		}
	}

	return (int)uiReadLen;
}


