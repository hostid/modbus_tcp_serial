/******************************************************************************
* Copyright (c) 2013, NULL, All rights reserved.
*
* File Name  : main.c
* Description: The functions for test modbus protocol.
* Author     : jiangfeng.zhang(retries@126.com)
* Date       : 2018/12/18
* Version    : 1.0
*
* Change Logs:
*   2018/12/18 jiangfeng.zhang : Create file.
******************************************************************************/

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <dlfcn.h>

#include "Modbus.h"    /*modbus  serial*/
#include "ModbusTcp.h" /*modbus  tcp*/


int main(int argc, char **argv)
{
    
#if 0
    int iRet = 0;
    int iDeviceFd = 0;
    int iSocket = 0;
    /*1、modbus serial test*/
    iDeviceFd = open("tty0", O_NONBLOCK|O_RDWR, S_IRWXU);
    iRet = DrivCommUartParaSet(pstDllHandle->iConnectFd,
                             UART_MODE_485,
                             UART_BARD_RATE_9600,
                             UART_CHAR_WID_8,
                             UART_CHECK_BIT_NONE,
                             UART_STOP_BIT_1);


    iRet = ModbusPacket(iDeviceFd, 0x06, aucReqString, iReqStringNum, MODBUS_RTU, aucReqPackage, &iReqPackageNum);
    iRet = DrivCommDeviceDataWrite (iDevFd, aucSendBuf, uiReqMsgLen);
    iRet = ModbusResponse( iDevFd,
                           MODBUS_RTU,
                           7,  //设备地址+  功能码+字节数目+数据+crc = 1+1+1+2+2 = 7 
                           DRIVER_DATA_READ_TIME_OUT_MS,
                           DRIVER_DATA_READ_INTERVAL_TIME_MS,
                           &ucSlaveId,
                           &ucFunction,
                           aucRecvBuf,
                           &iRspMsgLen   // 数据域中除数据地址外数据位有几个字节。

                      );

 


    iRet = ModbusPacket03(ucDevAddr, MODBUS_RTU, usStartAddr, usRegNum, aucSendBuf, &uiReqMsgLen);
    iRet = DrivCommDeviceDataWrite (iDevFd, aucSendBuf, uiReqMsgLen);
    iRet = ModbusResponse(  iDevFd,
                           MODBUS_RTU,
                           7,  //设备地址+  功能码+字节数目+数据+crc = 1+1+1+2+2 = 7 
                           DRIVER_DATA_READ_TIME_OUT_MS,
                           DRIVER_DATA_READ_INTERVAL_TIME_MS,
                           &ucSlaveId,
                           &ucFunction,
                           aucRecvBuf,
                           &iRspMsgLen   // 数据域中除数据地址外数据位有几个字节。
                       );

   


 


    /*2、modbus tcp  test*/
    memset((char *)&stAddr, 0, sizeof(stAddr)); /*create tcp client */
    memcpy((char *)&stAddr.sin_addr, stHp->h_addr, stHp->h_length);
    stAddr.sin_family = AF_INET;
    stAddr.sin_port   = htons(usPort);
    iSocket = socket(AF_INET, SOCK_STREAM, 0);
    iReturn = connect(iSocket, (struct sockaddr *)&stAddr, sizeof(stAddr));
 
    iRet = ModbusTcpWriteBit(iDeviceFd,&pstDevInfo->usTransId,ucDevAddr,iCoilAddr,ucData);
    iRet = ModbusWriteRegisters(iDeviceFd,&pstDevInfo->usTransId,ucDevAddr,iRegAddr,iLen,(const unsigned short *)pucData);
    iRet = ModbusTcpReadInputBits(iDevFd,&pstDeviceInfo->usTransId,ucDevAddr,32,85,aucRes);
    iRet = ModbusTcpReadRegisters(iDevFd,&pstDeviceInfo->usTransId,ucDevAddr,16,125,&ausRes);
#endif 
    return 0;
}

