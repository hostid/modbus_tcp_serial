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


#ifndef _DELTA_DRIVER_COMMON_H
#define _DELTA_DRIVER_COMMON_H

#include "netinet/in.h"

/******************************************************************************
* Macros.
******************************************************************************/
#define DRIVER_DEBUG
#ifdef  DRIVER_DEBUG
#define DRIVER_PRINT                                if(1)printf
#else
#define DRIVER_PRINT                                if(0)printf
#endif

#define DRIVER_MAX_LENGTH_DEVICE_FILE               64
/******************************************************************************
* functions return values.
******************************************************************************/

#define DRIVER_SUCCESS                              0
#define DRIVER_ERROR_SYSTEM                         -1
#define DRIVER_ERROR_OUT_OF_MEMORY                  -11
#define DRIVER_ERROR_PARAMETER                      -101
#define DRIVER_ERROR_PARAMETER_DEVICE_FILE_OPEN     -102
#define DRIVER_ERROR_PARAMETER_UART_SPEED           -103
#define DRIVER_ERROR_PARAMETER_UART_CHAR            -104
#define DRIVER_ERROR_PARAMETER_UART_PARITY          -105
#define DRIVER_ERROR_PARAMETER_UART_STOP            -106
#define DRIVER_ERROR_ALREADY_CONNECT                -201
#define DRIVER_ERROR_ALREADY_DISCONNECT             -202
#define DRIVER_ERROR_OPEN                           -203
#define DRIVER_ERROR_READ                           -204
#define DRIVER_ERROR_WRITE                          -205
#define DRIVER_ERROR_READ_TIMEOUT                   -206
#define DRIVER_ERROR_SOCK_CREATE					-250
#define DRIVER_ERROR_SOCK_BIND						-251
#define DRIVER_ERROR_SOCK_LISTEN					-252
#define DRIVER_ERROR_SOCK_ACCEPT					-253
#define DRIVER_ERROR_SOCK_CONNECT					-254
#define DRIVER_ERROR_SOCK_RECV						-255
#define DRIVER_ERROR_SOCK_SEND						-256
#define DRIVER_ERROR_SOCK_SELECT_TIME_OUT			-257
#define DRIVER_ERROR_POINT_OUT_OF_RANGE             -301
#define DRIVER_ERROR_DATA_TOO_LONG                  -401
#define DRIVER_ERROR_DATA_TOO_SHORT                 -402
#define DRIVER_ERROR_DATA_WRONG_LENGTH              -403
#define DRIVER_ERROR_DATA_NOT_FIND                  -404
#define DRIVER_ERROR_WRONG_CHECK_SUM                -451
#define DRIVER_ERROR_WRONG_ADDR_CODE                -452
#define DRIVER_ERROR_WRONG_DATA_TYPE                -453
#define DRIVER_ERROR_WRONG_START_FLAG               -460
#define DRIVER_ERROR_WRONG_END_FLAG                 -461
#define DRIVER_ERROR_MODBUS_LRC                     -1001
#define DRIVER_ERROR_MODBUS_CRC                     -1002
#define DRIVER_ERROR_MODBUS_MODE                    -1003
#define DRIVER_ERROR_MODBUS_WRONG_DATA_FORMAT       -1004
#define DRIVER_ERROR_MODBUS_UNSUPPORT_FUNCTION      -1005
#define DRIVER_ERROR_USER_DEFINED                   -10000

/* Handel Mode */
#define DRIVER_HANDLE_INIT      1
#define DRIVER_HANDLE_NO_INIT   0

/* Uart Mode */
#define UART_MODE_232           0
#define UART_MODE_485           1

/* Uart Bard Rate */
#define UART_BARD_RATE_1200     0
#define UART_BARD_RATE_2400     1
#define UART_BARD_RATE_4800     2
#define UART_BARD_RATE_9600     3
#define UART_BARD_RATE_19200    4
#define UART_BARD_RATE_38400    5

/* Uart Direction */
#define UART_DIR_SEND_ONLY      1
#define UART_DIR_RECV_ONLY      2
#define UART_DIR_SEND_RECV      3

/* Uart Character Wid */
#define UART_CHAR_WID_8         0
#define UART_CHAR_WID_7         1
#define UART_CHAR_WID_5         2

/* Uart Check bit */
#define UART_CHECK_BIT_NONE     0
#define UART_CHECK_BIT_ODD      1
#define UART_CHECK_BIT_EVEN     2

/* Uart Stop bit */
#define UART_STOP_BIT_1         0
#define UART_STOP_BIT_1_5       1
#define UART_STOP_BIT_2         2

#define DATA_UCHAR				1
#define DATA_CHAR				2
#define DATA_USHORT				3
#define DATA_SHORT				4
#define DATA_UINT				5
#define DATA_INT				6
#define DATA_FLOAT				9

#define DRIVER_BIT0                     	(0x0001)
#define DRIVER_BIT1                     	(0x0002)
#define DRIVER_BIT2                     	(0x0004)
#define DRIVER_BIT3                     	(0x0008)
#define DRIVER_BIT4                    	 	(0x0010)
#define DRIVER_BIT5                    	 	(0x0020)
#define DRIVER_BIT6                     	(0x0040)
#define DRIVER_BIT7                     	(0x0080)
#define DRIVER_BIT8                     	(0x0100)
#define DRIVER_BIT9                     	(0x0200)
#define DRIVER_BIT10                     	(0x0400)
#define DRIVER_BIT11                    	(0x0800)
#define DRIVER_BIT12                     	(0x1000)
#define DRIVER_BIT13                     	(0x2000)
#define DRIVER_BIT14                     	(0x4000)
#define DRIVER_BIT15                     	(0x8000)

#define	DRIVER_UINT8_BIT0					(0x01)
#define	DRIVER_UINT8_BIT1					(0x02)
#define	DRIVER_UINT8_BIT2					(0x04)
#define	DRIVER_UINT8_BIT3					(0x08)
#define	DRIVER_UINT8_BIT4					(0x10)
#define	DRIVER_UINT8_BIT5					(0x20)
#define	DRIVER_UINT8_BIT6					(0x40)
#define	DRIVER_UINT8_BIT7					(0x80)

#define DEVICE_FILE_PARA_NAME				"--device_file"        /* file  */
#define PROTOCOL_ADDR_PARA_NAME				"--protocol_addr"

#define PLC_IP_PARAM_NAME					"plc_ip"
#define MODBUS_TCP_PORT						502

#define IPV4_PARAM_NAME						"ipv4_addr"
#define PORT_PARAM_NAME						"port"
#define DOMAIN_PARAM_NAME					"domain"

#define BACNET_BEGIN_INSTANCE_NAME			"instance_num"

#define SEM_S7_LOCAL_TSAP					"local_tsap"
#define SEM_S7_REMOTE_TSAP					"remote_tsap"

// ISO and TPKT
#define	TCP_ISO_CODE_CR						0xE0
#define	TCP_ISO_CODE_CC						0xD0
#define	TCP_ISO_CODE_DR						0x80
#define	TCP_ISO_CODE_DT						0xF0
#define	TCP_ISO_CODE_ED						0x10
#define TCP_ISO_PARAM_0						0xC0
#define TCP_ISO_PARAM_1						0xC1
#define TCP_ISO_PARAM_2						0xC2
#define TCP_ISO_SRC_REF						(0x0001)
#define TCP_TPKT_VERSION					0x03
#define	TCP_ISO_TPDU_SIZE_512B				0x09
#define	TCP_ISO_TPDU_SIZE_1KB				0x0a
#define	TCP_ISO_TPDU_MAX_SIZE				(512)

/******************************************************************************
* Variables.
******************************************************************************/
typedef struct _AEC_ST_DLL_HANDLE
{
	char cFlag;                // 0x01:already init/connect, 0x00:not init/connect.
	int  iConnectFd;           // connect handle.
	void *pvArg;               // the data used by driver itself.
} ST_DLL_HANDLE;

#pragma pack(push)
#pragma pack(1)
///////////////////////////////////////////////////////////////////////////////
/* TPKT AND ISO */
typedef struct
{
	unsigned char	ucVersion;		// 0x03 固定值
	unsigned char	ucReserved;		// 0x00 固定值
	unsigned short	usDataLen;		// 数据包的总长度，大端法存储
}ST_TCP_IP_TPKT;	// TCP/IP层之上的TPKT层

typedef struct
{
	unsigned char	ucLen;			// 0：		0x11 该结构体的后续长度
	unsigned char	ucCode;			// 1：		0xE0 TPDU类型，表示连接请求CR
	unsigned short	usDstRef;		// 2~3：		0x0000，目的ref,固定值
	unsigned short	usSrcRef;		// 4~5：		0x0001，源ref, 可为固定值
	unsigned char	ucClass;		// 6:		0x00，固定值

	unsigned char	ucP1_code;		// 7:		0xC1, 固定值，Romate-TSAP
	unsigned char	ucP1_len;		// 8:		0x02, 固定值，Romate-TSAP长度
	unsigned short	usP1;			// 9~10:	

	unsigned char	ucP2_code;		// 11:		0xC2, 固定值，Local-TSAP
	unsigned char	ucP2_len;		// 12:		0x02, 固定值，Local-TSAP长度
	unsigned short	usP2;			// 13~14:	

	unsigned char	ucP0_code;		// 15:		0xC0, 固定值，TPDU-SIZE
	unsigned char	ucP0_len;		// 16:		0x02, 固定值，TPDU-SIZE长度
	unsigned char	ucP0;			// 17:		0x09，2^9=512字节
}ST_TCP_IP_ISO_CR;	// TPKT层之上的ISO层，用于请求连接

typedef struct
{
	unsigned char	ucLen;			// 0：		0x11 该结构体的后续长度
	unsigned char	ucCode;			// 1：		0xD0 TPDU类型，表示连接确认CC
	unsigned short	usDstRef;		// 2~3：		0x0001，目的ref,等于CR中的usSrcRef
	unsigned short	usSrcRef;		// 4~5：		0x????，源ref, PLC的返回ref，可不理会
	unsigned char	ucClass;		// 6:		0x00，固定值

	unsigned char	ucP0_code;		// 7:		0xC0, 固定值，TPDU-SIZE
	unsigned char	ucP0_len;		// 8:		0x01, 固定值，TPDU-SIZE长度
	unsigned char	ucP0;			// 9:		0x09，等于CR中的ucP0

	unsigned char	ucP1_code;		// 10:		0xC1, 固定值，Romate-TSAP
	unsigned char	ucP1_len;		// 11:		0x02, 固定值，Romate-TSAP长度
	unsigned short	usP1;			// 12~13:	等于CR中的usP1

	unsigned char	ucP2_code;		// 14:		0xC2, 固定值，Local-TSAP
	unsigned char	ucP2_len;		// 15:		0x02, 固定值，Local-TSAP长度
	unsigned short	usP2;			// 16~17:	等于CR中的usP2
}ST_TCP_IP_ISO_CC;	// TPKT层之上的ISO层，用于连接确认

typedef struct
{
	unsigned char	ucLen;			// 0x02 该结构体的后续长度
	unsigned char	ucCode;			// TPDU类型，0xE表示连接请求CR；0xD表示连接确认CC；0x8表示断开连接请求DR；0xF表示数据帧DT；0x1表示扩展数据帧ED
	unsigned char	ucNumEOT;		// 固定为0x80即可。
}ST_TCP_IP_ISO_COM;	// TPKT层之上的ISO层，用于连接之后的正常通信

#pragma pack(pop)

/******************************************************************************
* functions.
******************************************************************************/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/******************************************************************************
* Function   :  DrivCommUartParaSet
* Description:  Set the parameter of UART interface
* Parameter  :  iDevFd: The file descriptor of the UART interface which will be configed in this function
*
* Return     :
*
*
* Explain    :
******************************************************************************/
int DrivCommUartParaSet(int iDevFd /** The file descriptor */,
				                unsigned char ucUartMode,
				                unsigned char ucBoadRate,
				                unsigned char ucCharWid,
				                unsigned char ucCheckBit,
				                unsigned char ucStopBit);

int DrivCommGetUartAddrCode(int iArgc, char **ppcArgv, unsigned char *pucAddrCode);

char * DrivCommGetDevFileName(int iArgc, char **ppcArgv);

int DrivCommGetArgv(int iArgc, char **ppcArgv, const char *pcArgName, char **ppcArgValue);

int DrivCommDeviceDataWrite(const int iDevFd,
					const unsigned char *pucBuf,
					const unsigned int uiDataLen);

int DrivCommDeviceDataRead(int iDevFd,
				          unsigned char *pucBuf,
				          unsigned int uiDataLen,
				          unsigned int uiTimeOutMs,
				          unsigned int uiIntervalTimeMs);

int DrivCommCleanLinkCache(int iDevFd, int iTimeSecond);

int DrivCommLinkDisconnectDataConstruct(unsigned char *pucOutData, unsigned int *puiOutDataLen);

int DrivCommLinkConnectDataConstruct(unsigned char *pucOutData, unsigned int *puiOutDataLen);

int DrivCommTcpRestart(int iSocket, char *pcIpAddr, unsigned short  usPort);

int DrivCommTcpConnect(char *pcIpAddr, unsigned short  usPort);

int	DrivCommTcpConnectTimeOut	(const char *pcIP, unsigned short usPort, unsigned int uiTimeOutSecond);

int	DrivCommTcpAcceptTimeout	(int iServerSock, unsigned int uiSeconds);

int	DrivCommTcpServerCreate		(unsigned short usPort, unsigned short usListenNum);

int	DrivCommTcpSockRecvBufClean (int iSockFd);

int DrivCommTcpISOConnectTimeOut(const char *pcIP, 
                                 unsigned short usPort, 
                                 unsigned int uiTimeOutSecond,
                                 unsigned short usLocalTSAP,         // C2
                                 unsigned short usRemoteTSAP);       // C1
                                 
int DrivCommTcpISODisconnect(int iSocket, unsigned short usLocalTSAP, unsigned short usRemoteTSAP);

int	DrivCommTcpISODataWrite(const int iDevFd, const unsigned char *pucBuf, unsigned int uiDataLen);

int	DrivCommTcpISODataRead(int iDevFd, unsigned char *pucBuf, unsigned int uiDataLen, unsigned int uiTimeOutMs, unsigned int uiIntervalTimeMs);

int	Str2Hex	(unsigned int *uiDest, unsigned char *pucSrc, unsigned char ucSrcSize);
int  DrivCommUdpSockRecvBufClean (int iSockFd, struct	sockaddr_in *pstAddr);
int DrivCommUdpConnectTimeOut(const char *pcIP, unsigned short usPort, struct	sockaddr_in *pstAddr);
int DrivCommUdpDeviceDataWrite(const int iDevFd,
				          const unsigned char *pucBuf,
				          unsigned int uiDataLen,
				          struct sockaddr_in *pstAddr);
int DrivCommUdpDeviceDataRead(int iDevFd,
				          unsigned char *pucBuf,
				          unsigned int uiDataLen,
				          unsigned int uiTimeOutMs,
				          unsigned int uiIntervalTimeMs,
				          struct sockaddr_in *pstAddr);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif  // _DELTA_DRIVER_COMMON_H

