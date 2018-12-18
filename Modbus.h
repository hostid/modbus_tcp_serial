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



#ifndef _DELTA_DRIVER_MODBUS_H
#define _DELTA_DRIVER_MODBUS_H


/******************************************************************************
* Macros.
******************************************************************************/

#define MODBUS_RTU                      2
#define MODBUS_ASCII                    1

#define MODBUS_MAX_RESPONSE_LENGTH      1024


/******************************************************************************
* Variables.
******************************************************************************/


/******************************************************************************
* functions.
******************************************************************************/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


/******************************************************************************
* Function   : ModbusPacket
* Description: packet the modbus request string.(iSlave+iFunction+data+crc(lrc))
* Parameter  :
*              IN  int iSlave:
*                    The slave port address.
*                  int iFunction:
*                    The function code
*                  unsigned char *pucData:
*                    The request string data(eg:address+register number).
*                  int iDataLen:
*                    The request data length.
*                  char cModbusMode:
*                    The modbus mode:ASCII or RTU.
*               OUT char *pcReqString:
*                     The full packeted modbus request string.
*                  int *piNum:
*                     The length of packet data
* Return     : The packet result.
*              0:success
*              ERROR_WRONGPARAMETER    : wrong parameter.
*              ERROR_MODBUS        : packet fail.
*
* Change Logs:
*   2013/05/02 Zhao Lin : Build function.
******************************************************************************/
int ModbusPacket(int iSlave, int iFunction, unsigned char *pucData, int iDataLen, char cModbusMode, unsigned char *pcReqString, int *piNum);


/******************************************************************************
* Function   : ModbusPacket03
* Description: packet the modbus request string.(iSlave+iFunction+data+crc(lrc))
* Parameter  :
*              IN  int iSlave:
*                    The slave port address.
*                  char cModbusMode:
*                    The modbus mode: RTU.
*                  int iStartAddr:
*                    The start address
*                  int iRegNum:
*                    The register number.
*               OUT char *pucReqString:
*                     The full packeted modbus request string.
*                  int *piNum:
*                     The length of packet data
* Return     : The packet result.
*              0:success
*              DRIVER_ERROR_PARAMETER    : wrong parameter.
*              DRIVER_ERROR_MODBUS_MODE  : wrong modbus mode.
*
* Change Logs:
*   2013/05/02 Zhao Lin : Build function.
******************************************************************************/
int ModbusPacket03(unsigned char ucSlave, char cModbusMode, unsigned short usStartAddr, unsigned short usRegNum, unsigned char *pucReqString, unsigned int *piNum);


/******************************************************************************
* Function   : ModbusPacket04
* Description: packet the modbus request string.(iSlave+iFunction+data+crc(lrc))
* Parameter  :
*              IN  int iSlave:
*                    The slave port address.
*                  char cModbusMode:
*                    The modbus mode: RTU.
*                  int iStartAddr:
*                    The start address
*                  int iRegNum:
*                    The register number.
*               OUT char *pucReqString:
*                     The full packeted modbus request string.
*                  int *piNum:
*                     The length of packet data
* Return     : The packet result.
*              0:success
*              DRIVER_ERROR_PARAMETER    : wrong parameter.
*              DRIVER_ERROR_MODBUS_MODE  : wrong modbus mode.
*
* Explain    : Only support RTU, not support ASCII.
******************************************************************************/
int ModbusPacket04(unsigned char ucSlave, char cModbusMode, unsigned short usStartAddr, unsigned short usRegNum, unsigned char *pucReqString, unsigned int *piNum);

int ModbusPacketFuncCode(unsigned char ucSlave, unsigned char ucFunction, char cModbusMode, unsigned short usStartAddr, unsigned short usRegNum, unsigned char *pucReqString, unsigned int *piNum);

/******************************************************************************
* Function   : ModbusResponse
* Description: read data from device in the mode of modbus.(function code:2,3,6,16)
* Parameter  :
*              IN  int iFd:
*                    The file handle.
*                  char cModbusMode:
*                    The modbus mode:ASCII or RTU.
*               OUT unsigned char *pucData:
*                     The data get from the device.
*                  int iDataLen:
*                     The length of data.
* Return     : The read result.
*              0:success
*              ERROR_WRONGPARAMETER    : wrong parameter.
*              ERROR_MODBUS        : packet fail.
*
* Change Logs:
*   2013/05/02 Zhao Lin : Build function.
******************************************************************************/
int ModbusResponse(int iFd, char cModbusMode, int iReadDataLen, unsigned int uiTimeOutMs, unsigned int uiIntervalTimeMs,
                   unsigned char *pucSlaveId, unsigned char *pucFunction, unsigned char *pucData, int *iDataLen);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif // _DELTA_DRIVER_MODBUS_H
