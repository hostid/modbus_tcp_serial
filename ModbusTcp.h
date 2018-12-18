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



#ifndef _USMART_DRIVER_MODBUS_TCP_H
#define _USMART_DRIVER_MODBUS_TCP_H


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif




//0x01
int ModbusTcpReadBits(int iFd,unsigned short * pusTranID,
				unsigned char ucDevAddr,
				int iAddr, 
				int iLen, 
				unsigned char *dest);

//0x02
int ModbusTcpReadInputBits(int iFd,unsigned short*pusTranID,
				unsigned char ucDevAddr,
				int iAddr, 
				int iLen, 
				unsigned char *dest);
//0x03
int ModbusTcpReadRegisters(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen, 
			unsigned short *usDest);
//0x04
int ModbusTcpReadInputRegisters(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen, 
			unsigned short *usDest);
//0x05
int ModbusTcpWriteBit(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iCoilAddr, 
			int status);

//0x06
int ModbusTcpWriteRegister(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int reg_addr, 
			int value);
//0x0f
int ModbusTcpWriteBits(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen,  
			const unsigned char *pucSrc);
//0x10
int ModbusWriteRegisters(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen,  
			const unsigned short *pusSrc);




#endif

