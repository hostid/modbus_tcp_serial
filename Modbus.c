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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DriverComm.h"
#include "Modbus.h"

/******************************************************************************
* Macros.
******************************************************************************/
#define AEC_DRIVER_ERROR_PRINTF(fmt, args...) \
do \
{ \
	printf("[%s][%s][%s][%d]:","ERROR",__FILE__, __FUNCTION__, __LINE__); \
	printf(fmt,##args); \
}while(0); 



#define READ_INPUT_DISCRETE					0x02
#define READ_COIL							0x01
#define WRITE_SIGNAL_COIL					0x05
#define WRITE_MUL_COIL						0x0F

#define	READ_INPUT_REG						0x04
#define READ_MUL_REG						0x03
#define WRITE_SIGNAL_REG					0x06
#define	WRITE_MUL_REG						0x10
#define RW_MUL_REG							0x17
#define SHIELD_WRITE_REG					0x16

/******************************************************************************
* Variables.
******************************************************************************/
static const unsigned char g_aucCrcLow[] =
{
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const unsigned char g_aucCrcHigh[] =
{
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

/******************************************************************************
* functions.
******************************************************************************/

/******************************************************************************
* Function   : LrcError
* Description: lrc check.
* Parameter  :
*              IN  char *pcBuf:
*                    the check string(exclude :,islave+ifunction+data).
*                  int iLen:
*                    The string length (exclude lrc)
*
* Return     : The check result.
*              0:success
*              DRIVER_ERROR_PARAMETER   : wrong parameter.
*              DRIVER_ERROR_MODBUS_LRC  : CHECK FAILED.
*
* Explain    : NO.
******************************************************************************/
static int LrcError(char *pcBuf, int iLen)
{
	int i, iHigh, iLow, iVal, iLrc = 0;

	if (NULL == pcBuf || iLen < 0)
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }

	pcBuf++; // exclude starting ':' and trailing <CR><LF>
	for (i=1; i<iLen+2; i+=2)
    {                       
    	// len is without lrc -> len+2
    	sscanf((const char *) pcBuf++, "%1X", &iHigh);
    	sscanf((const char *) pcBuf++, "%1X", &iLow);
    	iVal = iHigh*16 + iLow;
    	iLrc += iVal;
    }
	if ((iLrc && 0xFF) != 0)  //if lrc check wrong.(if iLrc==0. success)
    {
    	AEC_DRIVER_ERROR_PRINTF("LRC Error!\n");
    	return DRIVER_ERROR_MODBUS_LRC; // lrc wrong
    }
	
	return DRIVER_SUCCESS;
}


/******************************************************************************
* Function   : CrcError
* Description: crc check.
* Parameter  :
*              IN  unsigned char *pucBuf:
*                    the check string(islave+ifunction+data+crc).
*                  int iLen:
*                    The string length
*
* Return     : The check result.
*              0:success
*              DRIVER_ERROR_PARAMETER   : wrong parameter.
*              DRIVER_SUCCESS           : CHECK FAILED.
*
* Explain    : NO.
******************************************************************************/
static int CrcError(unsigned char *pucBuf, int iLen)
{
	int i;
	unsigned char ucCrcHigh, ucCrcLow;
	unsigned char ucIndex = 0;

	if (NULL == pucBuf || iLen < 0)
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }

	ucCrcHigh = 0xff;
	ucCrcLow  = 0xff;
	for (i=0; i<iLen-2; i++)
    {
    	ucIndex   = ucCrcLow ^ pucBuf[i];
    	ucCrcLow  = ucCrcHigh ^ g_aucCrcLow[ucIndex];
    	ucCrcHigh = g_aucCrcHigh[ucIndex];
    }

	if ((ucCrcLow  != pucBuf[iLen-2]) ||
        (ucCrcHigh != pucBuf[iLen-1])  )
    {
    	AEC_DRIVER_ERROR_PRINTF("CRC Error!\n");
    	return DRIVER_ERROR_MODBUS_CRC;
    }

	return DRIVER_SUCCESS;
}


/******************************************************************************
* Function   : CrcInsert
* Description: insert crc into string tail.
* Parameter  :
*              IN  unsigned char *pucBuf:
*                    the check string.
*                  int iLen:
*                    The string length
* Return     : The check result.
*              0:success
*              DRIVER_ERROR_PARAMETER    : wrong parameter.
*
* Explain    : The data will insert into the tail of string, so we should make
*              sure the buffer length is enough.
******************************************************************************/
static int CrcInsert(unsigned char *pucBuf,int iLen)
{
	int i;
	unsigned char ucCrcHigh, ucCrcLow;
	unsigned char ucIndex = 0;

	if (NULL == pucBuf || iLen < 0)
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }

	ucCrcHigh = 0xff;
	ucCrcLow  = 0xff;

	for (i=0; i<iLen; i++)
    {
    	ucIndex   = ucCrcLow ^ pucBuf[i];
    	ucCrcLow  = ucCrcHigh ^ g_aucCrcLow[ucIndex];
    	ucCrcHigh = g_aucCrcHigh[ucIndex];
    }
	pucBuf[iLen]   = ucCrcLow;
	pucBuf[iLen+1] = ucCrcHigh;

	return DRIVER_SUCCESS;
}


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
*               OUT char *pucReqString:
*                     The full packeted modbus request string.
*                  int *piNum:
*                     The length of packet data
* Return     : The packet result.
*              0:success
*              DRIVER_ERROR_PARAMETER    : wrong parameter.
*              DRIVER_ERROR_MODBUS_WRONG_DATA_FORMAT        : packet fail.
*
* Explain    : NO.
******************************************************************************/
int ModbusPacket(int iSlave, int iFunction, unsigned char *pucData, int iDataLen, char cModbusMode, unsigned char *pucReqString, int *piNum)
{
	int iNum = 0;

	unsigned char cLrc = 0;
	int iHigh,iLow,iVal,i;

	if (NULL == pucData)
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }

	if ((cModbusMode != MODBUS_ASCII) && (cModbusMode != MODBUS_RTU))
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_MODBUS_MODE;
    }

	if (MODBUS_ASCII == cModbusMode)
    {
    	pucReqString[iNum++] = ':';
    	sprintf((char *) &pucReqString[iNum], "%02X", iSlave);
    	iNum += 2;
    	sprintf((char *) &pucReqString[iNum], "%02X", iFunction);
    	iNum += 2;
    	for (i=0; i<iDataLen; i++)
        {
        	sprintf((char *) &pucReqString[iNum], "%02X",(int) pucData[i]);
        	iNum += 2;
        }

        //Insert LRC check code
    	for (i=1; i<iNum; i+=2) // exclude starting ':' and trailing <CR><LF>
        {
        	sscanf((const char *) &pucReqString[i],   "%1X", &iHigh);
        	sscanf((const char *) &pucReqString[i+1], "%1X", &iLow);
        	iVal = iHigh*16 + iLow;
        	cLrc += iVal;
        }
    	cLrc = ((unsigned char)(-((char) cLrc)));
    	sprintf((char *) &pucReqString[iNum],"%02X",(unsigned int) cLrc);
    	iNum += 2;
    	pucReqString[iNum++] = 0x0d;
    	pucReqString[iNum++] = 0x0a;

    }
	else if (MODBUS_RTU == cModbusMode)
    {
    	pucReqString[iNum++] = (unsigned char) iSlave;
    	pucReqString[iNum++] = (unsigned char) iFunction;
    	for (i=0; i<iDataLen; i++)
        {
        	pucReqString[iNum++] = pucData[i];
        }

        //insert CRC check code
    	CrcInsert(pucReqString, iNum);

    	iNum += 2;
    }

    *piNum = iNum;  //the packeted data length

	return DRIVER_SUCCESS;
}

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
* Explain    : Only support RTU, not support ASCII.
******************************************************************************/

int ModbusPacket03(unsigned char ucSlave, char cModbusMode, unsigned short usStartAddr, unsigned short usRegNum, unsigned char *pucReqString, unsigned int *piNum)
{
	int iNum;

	if (NULL == pucReqString || NULL == piNum)
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }

	if ((cModbusMode != MODBUS_RTU))    // only support RTU.
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_MODBUS_MODE;
    }
	
    if (MODBUS_RTU == cModbusMode)
    {
        iNum = 0;
    	pucReqString[iNum++] = ucSlave;
    	pucReqString[iNum++] = READ_MUL_REG;
    	pucReqString[iNum++] = usStartAddr >> 8;  //register start address (MSB)
    	pucReqString[iNum++] = usStartAddr &0xFF;      //(LSB)
    	pucReqString[iNum++] = usRegNum >> 8;  //register number: number =usPointNum
    	pucReqString[iNum++] = usRegNum &0xFF;

        //insert CRC check code
    	CrcInsert(pucReqString, iNum);
    	iNum += 2;
    }

    *piNum = iNum;  //the packeted data length

	return DRIVER_SUCCESS;
}

int ModbusPacketFuncCode(unsigned char ucSlave, unsigned char ucFunction, char cModbusMode, unsigned short usStartAddr, unsigned short usRegNum, unsigned char *pucReqString, unsigned int *piNum)
{
	int iNum;

	if (NULL == pucReqString || NULL == piNum)
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }

	if ((cModbusMode != MODBUS_RTU))    // only support RTU.
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_MODBUS_MODE;
    }
	
    if (MODBUS_RTU == cModbusMode)
    {
        iNum = 0;
    	pucReqString[iNum++] = ucSlave;
    	pucReqString[iNum++] = ucFunction;
    	pucReqString[iNum++] = usStartAddr >> 8;  //register start address (MSB)
    	pucReqString[iNum++] = usStartAddr &0xFF;      //(LSB)
    	pucReqString[iNum++] = usRegNum >> 8;  //register number: number =usPointNum
    	pucReqString[iNum++] = usRegNum &0xFF;

        //insert CRC check code
    	CrcInsert(pucReqString, iNum);
    	iNum += 2;
    }

    *piNum = iNum;  //the packeted data length

	return DRIVER_SUCCESS;
}

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
int ModbusPacket04(unsigned char ucSlave, char cModbusMode, unsigned short usStartAddr, unsigned short usRegNum, unsigned char *pucReqString, unsigned int *piNum)
{
	int iNum;

	if (NULL == pucReqString || NULL == piNum)
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }

	if ((cModbusMode != MODBUS_RTU))    // only support RTU.
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_MODBUS_MODE;
    }
	
    if (MODBUS_RTU == cModbusMode)
    {
        iNum = 0;
    	pucReqString[iNum++] = ucSlave;
    	pucReqString[iNum++] = READ_INPUT_REG;
    	pucReqString[iNum++] = usStartAddr >> 8;  //register start address (MSB)
    	pucReqString[iNum++] = usStartAddr &0xFF;      //(LSB)
    	pucReqString[iNum++] = usRegNum >> 8;  //register number: number =usPointNum
    	pucReqString[iNum++] = usRegNum &0xFF;

        //insert CRC check code
    	CrcInsert(pucReqString, iNum);
    	iNum += 2;
    }

    *piNum = iNum;  //the packeted data length

	return DRIVER_SUCCESS;
}

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
*              DRIVER_ERROR_PARAMETER    : wrong parameter.
*              DRIVER_ERROR_MODBUS_WRONG_DATA_FORMAT        : packet fail.
*
* Explain    : NO.
******************************************************************************/
int ModbusResponse(int iFd, char cModbusMode, int iReadDataLen, unsigned int uiTimeOutMs, unsigned int uiIntervalTimeMs,
                   unsigned char *pucSlaveId, unsigned char *pucFunction, unsigned char *pucData, int *iDataLen)
{
	unsigned char *pucTmp = NULL;
	int iLen = 0, iByteCount, iData, iVal, iTmp;
	unsigned char aucBuffer[MODBUS_MAX_RESPONSE_LENGTH] = {0};
	int iSlave, iFunction;
	unsigned char ucSlave,ucFunction;
	unsigned char *pucStartBuf = NULL;

    // check parameter.
	if (NULL == pucSlaveId || NULL == pucFunction || NULL == pucData || NULL == iDataLen)
    {
        AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_PARAMETER;
    }
	if ((cModbusMode != MODBUS_ASCII) && (cModbusMode != MODBUS_RTU))
    {
    	AEC_DRIVER_ERROR_PRINTF("Wrong Parameter!\n");
    	return DRIVER_ERROR_MODBUS_MODE;
    }

    // init data.
    *iDataLen = 0;

    //read from device
	iTmp = DrivCommDeviceDataRead(iFd, aucBuffer, iReadDataLen, uiTimeOutMs, uiIntervalTimeMs);
	if (iTmp < 0)
    {
    	AEC_DRIVER_ERROR_PRINTF("DrivCommDeviceDataRead failed. and return=%d\n", iTmp);
    	return iTmp;
    }
	printf("iTmp is %d\n\n\n\n\n", iTmp);
	aucBuffer[iTmp] = '\0';  //end the request string

#ifdef DRIVER_DEBUG

    DRIVER_PRINT("[Modbus][%s][%d] Receive Data(%d,%d):\n", __FUNCTION__, __LINE__, iFd, iTmp);

        //此处将modbus回过来的数据进行了打印。
    for (iLen=0; iLen<iTmp; iLen++)
    {
	    DRIVER_PRINT("0x%02X ", aucBuffer[iLen]);
    }
    DRIVER_PRINT("\n[Modbus][%s][%d] Finish.\n", __FUNCTION__, __LINE__);
#endif // DRIVER_DEBUG

	if (MODBUS_ASCII == cModbusMode)
    {
    	pucTmp = (unsigned char *) strchr((const char *) aucBuffer, ':');
    	if (NULL == pucTmp)
        {
            AEC_DRIVER_ERROR_PRINTF("[Modbus]Modbus Wrong Data Format, not find ':'.\n");
        	return DRIVER_ERROR_MODBUS_WRONG_DATA_FORMAT;
        }
    	pucStartBuf = pucTmp;
    	iLen = 0;
    	iLen++;
    	sscanf((char *) &pucTmp[iLen],"%02X",&iSlave);
    	iLen += 2;
    	sscanf((char *) &pucTmp[iLen],"%02X", &iFunction);
    	iLen += 2;
    	AEC_DRIVER_ERROR_PRINTF("[Modbus]iSlave:%d, iFunction:%d.\n", iSlave, iFunction);
    	*pucSlaveId  = (unsigned char)iSlave;
    	*pucFunction = (unsigned char)iFunction;
    	switch (iFunction)
        {
    	case 2:
    	case 3: //  response :data-length  h-data l-date ...
    	case 4:
        	sscanf((char *) &pucTmp[iLen],"%02X",&iByteCount);
        	iLen += 2;
        	AEC_DRIVER_ERROR_PRINTF("[Modbus]iByteCount:%d.\n", iByteCount);
        	for (iData=0; iData<iByteCount; iData++)
            {
            	sscanf((const char *) &pucTmp[iLen], "%02X", &iVal);
            	iLen += 2;
            	pucData[iData] = iVal;
            }
        	iTmp = LrcError((char *)pucStartBuf, iLen); //lrc check
        	if (iTmp < 0)
            {
                AEC_DRIVER_ERROR_PRINTF("[Modbus]LrcError check failed(%d).\n", iTmp);
            	return DRIVER_ERROR_MODBUS_LRC;
            }
            *iDataLen = iByteCount;
        	break;

        case 5:  // for MX28A and AO4A.
    	case 6:  //function code :0x06   response :h-addr l-addr h-data l-date
    	case 16: //0x10:  response :h-addr l-addr h-register numner l-register number
        	for (iData=0; iData<4; iData++)
            {
            	sscanf((const char *) &pucTmp[iLen], "%02X", &iVal);
            	iLen += 2;
            	pucData[iData] = iVal;
            }
        	iTmp = LrcError((char *)pucStartBuf, iLen); //lrc check
        	if (iTmp < 0)
            {
            	AEC_DRIVER_ERROR_PRINTF("[Modbus]LrcError check failed(%d).\n", iTmp);
            	return DRIVER_ERROR_MODBUS_LRC;
            }
            *iDataLen = iByteCount;
        	break;

    	default:
            AEC_DRIVER_ERROR_PRINTF("[Modbus]Unsupport function(%d).\n", iFunction);
            return DRIVER_ERROR_MODBUS_UNSUPPORT_FUNCTION;
        }
    }
	else if (MODBUS_RTU == cModbusMode)
    {
        iLen       = 0;
    	ucSlave     = aucBuffer[iLen++];
    
    	ucFunction  = aucBuffer[iLen++];
 
    	*pucSlaveId  = ucSlave;
    	*pucFunction = ucFunction;
    	switch (ucFunction)
        {
            //这块区分，当时读PLC,功能码为03 4 254： 数据处理分支。
            // 回的数据不包含设备地址，仅仅包含读取到的数据字节个数。
    	case 3:
    	case 4:
		case 254: // 0xFE
        	iByteCount = aucBuffer[iLen++];
        	memcpy(pucData, &aucBuffer[iLen], iByteCount); //ByteCount
        	iLen += (iByteCount+2); // with 2 bytes crc lens
        	iTmp = CrcError(aucBuffer, iLen);  //crc check
        	if (iTmp < 0)
            {
            	AEC_DRIVER_ERROR_PRINTF("[Modbus]LrcError check failed(%d).\n", iTmp);
            	return DRIVER_ERROR_MODBUS_LRC;
            }
            *iDataLen = iByteCount; // only return data lens
        	break;

            //这块是写PLC，功能码为5 6 16，PLC回执的数据，回执数据里面包含了设备地址。
        case 5:
    	case 6:  //function code :0x06   response :h-addr l-addr h-data l-date
    	case 16: //0x10:  response :h-addr l-addr h-register numner l-register number
        	memcpy(pucData, &aucBuffer[iLen], 4);
        	iLen += (4+2);   // (data length)+2(crc)
        	iTmp = CrcError(aucBuffer, 8);  //crc check
        	if (iTmp < 0)
            {
            	AEC_DRIVER_ERROR_PRINTF("[Modbus]LrcError check failed(%d).\n", iTmp);
            	return DRIVER_ERROR_MODBUS_LRC;
            }
            *iDataLen = 4;
        	break;
    	case 0xB8:
        	memcpy(pucData, &aucBuffer[iLen], 9);   //ByteCount + 2(CRC)
        	iTmp = CrcError(aucBuffer, 11);         //crc check
        	if (iTmp < 0)
            {
            	AEC_DRIVER_ERROR_PRINTF("[Modbus]LrcError check failed(%d).\n", iTmp);
            	return DRIVER_ERROR_MODBUS_LRC;
            }
            *iDataLen = 7;
        	break;
    	default:
            AEC_DRIVER_ERROR_PRINTF("[Modbus]Unsupport function(%d).\n", iFunction);
            return DRIVER_ERROR_MODBUS_UNSUPPORT_FUNCTION;
        }
    }

	return DRIVER_SUCCESS;
}

