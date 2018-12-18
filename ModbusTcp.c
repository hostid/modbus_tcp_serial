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
#include "ModbusTcp.h"


/******************************************************************************
* Macros.
******************************************************************************/
#define AEC_DRIVER_ERROR_PRINTF(fmt, args...) \
do \
{ \
	printf("[%s][%s][%s][%d]:","ERROR",__FILE__, __FUNCTION__, __LINE__); \
	printf(fmt,##args); \
}while(0); 


#define MODBUS_FC_READ_COILS                0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS      0x02
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FC_READ_INPUT_REGISTERS      0x04
#define MODBUS_FC_WRITE_SINGLE_COIL         0x05
#define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06
#define MODBUS_FC_READ_EXCEPTION_STATUS     0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10
#define MODBUS_FC_REPORT_SLAVE_ID           0x11
#define MODBUS_FC_MASK_WRITE_REGISTER       0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17


#define MODBUS_BROADCAST_ADDRESS    0

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */

#define MODBUS_MAX_READ_BITS              2000
#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_WR_WRITE_REGISTERS      121
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* The size of the MODBUS PDU is limited by the size constraint inherited from
 * the first MODBUS implementation on Serial Line network (max. RS485 ADU = 256
 * bytes). Therefore, MODBUS PDU for serial line communication = 256 - Server
 * address (1 byte) - CRC (2 bytes) = 253 bytes.
 */
#define MODBUS_MAX_PDU_LENGTH              253

#define MSG_LENGTH_UNDEFINED -1

#define _MODBUS_TCP_HEADER_LENGTH      7
/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes
 */
#define MODBUS_TCP_MAX_ADU_LENGTH  260
#define _MIN_REQ_LENGTH				12


#define DRIVER_DATA_READ_TIME_OUT_MS			200
#define DRIVER_DATA_READ_INTERVAL_TIME_MS		100
/* Max between RTU and TCP max adu length (so TCP) */
#define MAX_MESSAGE_LENGTH 260


typedef enum {
    _STEP_FUNCTION,
    _STEP_META,
    _STEP_DATA
} _step_t;


/* The goal of this program is to check all major functions of
   libmodbus:
   - write_coil
   - read_bits
   - write_coils
   - write_register
   - read_registers
   - write_registers
   - read_registers

   All these functions are called with random values on a address
   range defined by the following defines.
*/


/***************************************************************************
* Function   : ModbusTcpPacket
* Description: Packet Modbus/TCP protocol data frame base on input parameter and output pucReq
* Parameter  :
*              [Input  Parameter]
*             		pusTranID      @Modbus/TCP MBAP's Trans indentifier field
                    ucDevAddr      @Device address
                    iFunction      @Modbus protocol function code
                    iAddr          @Regiter start address 
                    iLen           @Number of  register                     
*              [Output Parameter]
*             		pucReq         @Modbus/TCP protocol data frame 
                                    [pucReq Parameter not initial pucReq[4] and pucReq[5]]
* Return     :                         
                    12             @Fix data
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
***************************************************************************/

int ModbusTcpPacket(unsigned short * pusTranID,
		unsigned char ucDevAddr,
		int iFunction,
		int iAddr,
		int iLen,
		unsigned char* pucReq)
{
    /* Increase transaction ID */
	if (NULL == pusTranID)
		return DRIVER_ERROR_PARAMETER;
	
    if (*pusTranID < UINT16_MAX)
        (*pusTranID)++;
    else
        *pusTranID = 0;

    //pusTranID : 传输标识。 
    pucReq[0] = *pusTranID >> 8;
    pucReq[1] = *pusTranID & 0x00ff;

    /* Protocol Modbus */
    pucReq[2] = 0;
    pucReq[3] = 0;

    /* Length will be defined later by set_req_length_tcp at offsets 4
       and 5 */

    pucReq[6] = ucDevAddr;
    pucReq[7] = iFunction;
    pucReq[8] = iAddr >> 8;
    pucReq[9] = iAddr& 0x00ff;
    pucReq[10] = iLen>> 8;
    pucReq[11] = iLen& 0x00ff;

    return 12;		//The values here are unchanged

}


/***************************************************************************
* Function   : ModbusTcpSendMsg
* Description: Send message by ModbusTcp protocol
* Parameter  :
*              [Input  Parameter]
*             		iFd            @Socket file handle
*             		pucReq         @Modbus/TCP Data frame
                    iLen           @the length of pucReq                   
*              [Output Parameter]
*             		NO
* Return     :      
                    0              @write success 
                    other          @write fail
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
***************************************************************************/

int ModbusTcpSendMsg(int iDevFd, unsigned char* pucReq,int iLen)
{
    //iLen 参数的调用函数传递的是固定的值12
	int iMbapLen = iLen - 6;/*数据总长度-报文长度*/
	int iRet = 0;
    pucReq[4] = iMbapLen >> 8;
    pucReq[5] = iMbapLen & 0x00FF;
	
	iRet = DrivCommDeviceDataWrite (iDevFd, pucReq, iLen);
    
	if (DRIVER_SUCCESS != iRet)
	{
		AEC_DRIVER_ERROR_PRINTF("write to device failed(%d).\n", iRet);
		return DRIVER_ERROR_WRITE;
	}
	return iRet;
}



/***************************************************************************
* Function   : compute_meta_length_after_function
* Description: Get the number after Modbus protocol function code 
* Parameter  :
*              [Input  Parameter]
*             		function            @Modbus protocol function code         
*              [Output Parameter]
*             		NO
* Return     :      
                    length              @the next read number 
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
***************************************************************************/

static uint8_t compute_meta_length_after_function(int function)
{
    int length;
	#if 0
    if (msg_type == MSG_INDICATION) {
        if (function <= MODBUS_FC_WRITE_SINGLE_REGISTER) {
            length = 4;
        } else if (function == MODBUS_FC_WRITE_MULTIPLE_COILS ||
                   function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS) {
            length = 5;
        } else if (function == MODBUS_FC_MASK_WRITE_REGISTER) {
            length = 6;
        } else if (function == MODBUS_FC_WRITE_AND_READ_REGISTERS) {
            length = 9;
        } else {
            /* MODBUS_FC_READ_EXCEPTION_STATUS, MODBUS_FC_REPORT_SLAVE_ID */
            length = 0;
        }
    } else 
	#endif
	{

        //根据功能码获取下一字段的字节个数，详细数据帧格式参见《modbus协议》

        /* MSG_CONFIRMATION */
        switch (function) {
        case MODBUS_FC_WRITE_SINGLE_COIL:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            length = 4;
            break;
        case MODBUS_FC_MASK_WRITE_REGISTER:
            length = 6;
            break;
        default:
            length = 1;
        }
    }

    return length;
}

/* Computes the length to read after the meta information (address, count, etc) */
static int compute_data_length_after_meta(unsigned char *msg)
{
    int function = msg[_MODBUS_TCP_HEADER_LENGTH];
    int length;
	#if 0
    if (msg_type == MSG_INDICATION) {
        switch (function) {
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            length = msg[_MODBUS_TCP_HEADER_LENGTH + 5];
            break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
            length = msg[_MODBUS_TCP_HEADER_LENGTH + 9];
            break;
        default:
            length = 0;
        }
    } else 
	#endif
	{
        /* MSG_CONFIRMATION */
        if (function <= MODBUS_FC_READ_INPUT_REGISTERS ||
            function == MODBUS_FC_REPORT_SLAVE_ID ||
            function == MODBUS_FC_WRITE_AND_READ_REGISTERS)
            {
            length = msg[_MODBUS_TCP_HEADER_LENGTH + 1];

			
			AEC_DRIVER_ERROR_PRINTF("function(%d).length:%d\n", function,length);
        } else {
            length = 0;
			
			//AEC_DRIVER_ERROR_PRINTF("shit here is wrong\n");
        }
    }
	
    return length;
}



/***************************************************************************
* Function   : ModbusTcpRecvMsg
* Description: read data by ModbusTcp protocol
* Parameter  :
*              [Input  Parameter]
*             		iFd            @Socket file handle                
*              [Output Parameter]
*             		pucRsp         @Response data from slave
* Return     :      
                   iMsgLength      @the total len of Response data
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
*           2018/11/23  jiangfeng.zhang :corrects printf bug in 383 Line  
***************************************************************************/

int ModbusTcpRecvMsg(int iDevFd,unsigned char *pucRsp)
{
	int iRet = 0;
	int iLengthToRead; /*要读的数据长度*/
    int iMsgLength = 0; /*读取到的数据长度*/
	_step_t step;
	step = _STEP_FUNCTION;/*循环读取处理数据的状态位*/
	iLengthToRead = _MODBUS_TCP_HEADER_LENGTH + 1; /*报头长度+1个字节的功能码*/

    int i = 0,iPrintCount = 0; //j用来控制是否是第一次打印，如果不是需要再次重新处理。


#if 0


    iRet = DrivCommDeviceDataRead(iDevFd, pucRsp+iMsgLength, 20, DRIVER_DATA_READ_TIME_OUT_MS, DRIVER_DATA_READ_INTERVAL_TIME_MS);

	for (i=0;i<iRet;i++)
		{
		    printf("0x%02x ",pucRsp[i]);

	    }
	printf("\n");
    
#endif

#if 1
    /****************************
        循环读取
        1、先读8字节的报头+功能码  
        2、根据功能码获取下一次要读取的字节长度这一字段
        3、根据字节长度字段向后读取多少个数据
    ****************************/
	while(iLengthToRead != 0)
	{
		iRet = DrivCommDeviceDataRead(iDevFd, pucRsp+iMsgLength, iLengthToRead, DRIVER_DATA_READ_TIME_OUT_MS, DRIVER_DATA_READ_INTERVAL_TIME_MS);

        if (iRet <= 0 )
		{
			AEC_DRIVER_ERROR_PRINTF("DrivCommDeviceDataRead error\n");
			return DRIVER_ERROR_READ;
		}

	
	
	AEC_DRIVER_ERROR_PRINTF("recv is(iLengthToRead:%d):\n",iLengthToRead);

    //mark   @every timer printf data start with pucRsp[0]
       #if 0 
        for (i=0;i<iRet;i++)
    		{
    		    printf("0x%02x ",pucRsp[i]);

    	    }
    	printf("\n");
       #endif

      #if 1
        for (i=0;i<iRet;i++)
    		{
               if(!iPrintCount)
               {
    		      printf("0x%02x ",pucRsp[i]);
                  iPrintCount = !iPrintCount;
               }
               else
                 printf("0x%02x ",pucRsp[iMsgLength+i]); //需要打印的起始地址后移  
    	    }
    	printf("\n");
      #endif  
    
	AEC_DRIVER_ERROR_PRINTF("\n");

		
		iMsgLength += iRet;
		iLengthToRead -= iRet;  //iLengthToRead重新置0

		AEC_DRIVER_ERROR_PRINTF("iLengthToRead:%d iMsgLength:%d iRet:%d step:%d\n",iLengthToRead,iMsgLength,iRet,step);
		
		if (iLengthToRead == 0) 
		{		
	        switch (step) 
                {
	        	case _STEP_FUNCTION:
	                /* Function code position */
	                iLengthToRead = compute_meta_length_after_function(pucRsp[_MODBUS_TCP_HEADER_LENGTH]);
					
					AEC_DRIVER_ERROR_PRINTF("_STEP_FUNCTION(iLengthToRead:%d):\n",iLengthToRead);

					if (iLengthToRead != 0) 
					{
	                    step = _STEP_META;
	                    break;
	                } /* else switches straight to the next step */
	            case _STEP_META:
	                iLengthToRead = compute_data_length_after_meta(pucRsp);
					
					AEC_DRIVER_ERROR_PRINTF("_STEP_META(iLengthToRead:%d):\n",iLengthToRead);
					if ((iMsgLength + iLengthToRead) > (int)MODBUS_TCP_MAX_ADU_LENGTH) 
					{
	                    AEC_DRIVER_ERROR_PRINTF("too many data");
	                    return -1;
	                }
	                step = _STEP_DATA;
	                break;
	            default:
	                break;
	        }
	    }
	}
#endif	
	return iMsgLength;
}



/***************************************************************************
* Function   : compute_response_length_from_request
* Description: compute the response data length by Modbus TCP send message
* Parameter  :
*              [Input  Parameter]
*             		req            @Modbus TCP send message               
* Return     :      
                  offset + length  @be supposed to  Response data length
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
***************************************************************************/

static unsigned int compute_response_length_from_request(unsigned char *req)
{
    int length;
    const int offset = _MODBUS_TCP_HEADER_LENGTH;

    switch (req[offset]) {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS: {
        /* Header + nb values (code from write_bits) */

        int nb = (req[offset + 3] << 8) | req[offset + 4];  //获取发送消息的读取的寄存器个数的字段。
        
        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);   //功能码(1)+字节数(1)+由寄存器个数推算出来的返回的字节数
                                                      //length包函数从机响应的Modbus数据帧中除报头字段为的数据长度
    }
        break;
    case MODBUS_FC_WRITE_AND_READ_REGISTERS:
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_READ_INPUT_REGISTERS:
        /* Header + 2 * nb values */
        length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
        break;
    case MODBUS_FC_READ_EXCEPTION_STATUS:
        length = 3;
        break;
    case MODBUS_FC_REPORT_SLAVE_ID:
        /* The response is device specific (the header provides the
           length) */
        return MSG_LENGTH_UNDEFINED;
    case MODBUS_FC_MASK_WRITE_REGISTER:
        length = 7;
        break;
    default:
        length = 5;
    }

    return offset + length;
}




/***************************************************************************
* Function   : CheckConfirmation
* Description: compare data field's length in Modbus Tcp response frame =? Modbus Tcp Send message compute length
* Parameter  :
*              [Input  Parameter]
*             		req            @Modbus Tcp send data frame 
                    rsp            @Modbus Tcp response data frame 
                    rsp_length     @Modbus Tcp response toal len 
*              [Output Parameter]
*             		NO
* Return     :      
                   rc              @only the Response data field length
                                   @-1 [fail]
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
***************************************************************************/

static int CheckConfirmation(unsigned char *req,
                              unsigned char *rsp, int rsp_length)
{
    int rc;
    int rsp_length_computed;
    const int offset = _MODBUS_TCP_HEADER_LENGTH;
    const int function = rsp[offset];

	int i = 0;
	
	AEC_DRIVER_ERROR_PRINTF("data is:\n");
	for (i=0;i<rsp_length;i++)
		{
		    printf("0x%02x ",rsp[i]);  //打印Modbus Tcp response data frame
	    }
	printf("\n");

    
        //mark   @req[0] is step increase in ModbusTcpPacket API function???
     if (req[0] != rsp[0] && req[0] != MODBUS_BROADCAST_ADDRESS) 
     {
		AEC_DRIVER_ERROR_PRINTF("ModbusTcpRecvMsg req[0]:0x%02x error\n",req[0]);
        return DRIVER_ERROR_SYSTEM;
     } 

    rsp_length_computed = compute_response_length_from_request(req);

    /* Exception code */
    if (function >= 0x80) {
		
		AEC_DRIVER_ERROR_PRINTF("ModbusTcpRecvMsg Exception\n");
    	return DRIVER_ERROR_SYSTEM;
    }

	

    /* Check length */
    if ((rsp_length == rsp_length_computed ||
         rsp_length_computed == MSG_LENGTH_UNDEFINED) &&
        function < 0x80) 
        {
        int req_nb_value;
        int rsp_nb_value;

        /* Check function code */
        if (function != req[offset]) {
			
			AEC_DRIVER_ERROR_PRINTF("function:0x%02x req[%d]:0x%02x\n",function, offset,req[offset]);
            return DRIVER_ERROR_SYSTEM;
        }

        /* Check the number of values is corresponding to the request */
        switch (function) {
        case MODBUS_FC_READ_COILS:
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            /* Read functions, 8 values in a byte (nb
             * of values in the request and byte count in
             * the response. */

            /* 根据发送字节的读寄存器的个数，反推出应该响应的字节个数*/
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            req_nb_value = (req_nb_value / 8) + ((req_nb_value % 8) ? 1 : 0);

            rsp_nb_value = rsp[offset + 1]; 
            /* 
                根据功能码提取响应buf中真正读取到的字节个数，
                当然这个字节个数仅仅包含读取的真正的数据的个数。
            */
            break;
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
            /* Read functions 1 value = 2 bytes */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 1] / 2);
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            /* N Write functions */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 3] << 8) | rsp[offset + 4];
            break;
        case MODBUS_FC_REPORT_SLAVE_ID:
            /* Report slave ID (bytes received) */
            req_nb_value = rsp_nb_value = rsp[offset + 1];
            break;
        default:
            /* 1 Write functions & others */
            req_nb_value = rsp_nb_value = 1;
        }


        /* 进行比对读取到的数据的字节数是否一致*/
        if (req_nb_value == rsp_nb_value) 
        {
            rc = rsp_nb_value;   //如果一致，返回真正的数据域部分的数据字节个数。
        } 
        else 
         {
            rc = -1;        //否则返回-1
        }
    } else {
  
        rc = -1;
    }
    return rc;
}




/***************************************************************************
* Function   : ModbusTcpReadIoStatus
* Description: Read IO Status,IO Status store in dest
* Parameter  :
*              [Input  Parameter]
*             		iFd            @Socket file handle
*             		pusTranID      @Modbus/TCP MBAP's Trans indentifier field
                    iFunction      @Modbus protocol function code
                    ucDevAddr      @Device address
                    iAddr          @Regiter start address 
                    iLen           @Number of  register                    
*              [Output Parameter]
*             		dest           @each register data ,vaild data number is iLen
                                    dest's contet is 0x01 or 0x00.
* Return     :      
                    iRspLen        @only the Response data field length 
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
***************************************************************************/

int ModbusTcpReadIoStatus(int iFd,unsigned short * pusTranID,
			int iFunction,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen, 
			unsigned char *dest)
{
	int iRet;

	unsigned char aucReq[_MIN_REQ_LENGTH] = {0}; /*request send data*/
    unsigned char aucRsp[MODBUS_TCP_MAX_ADU_LENGTH] = {0};/*modbus tcp receive data*/

	int iReqLen = 0,iRspLen = 0;

	if (NULL == pusTranID)
		return DRIVER_ERROR_PARAMETER;

    if (iLen > MODBUS_MAX_READ_BITS) {
        return DRIVER_ERROR_PARAMETER;
    }

	iReqLen = ModbusTcpPacket(pusTranID,ucDevAddr,iFunction,iAddr,iLen,aucReq);

	iRet = ModbusTcpSendMsg(iFd,aucReq,iReqLen);

    if (DRIVER_SUCCESS == iRet)
	{
		int i, temp, bit;
        int pos = 0;
        int offset;
        int offset_end;

        iRet = ModbusTcpRecvMsg(iFd,aucRsp);

        
        if (iRet < 0)
        {
			AEC_DRIVER_ERROR_PRINTF("ModbusTcpRecvMsg error\n");
            return DRIVER_ERROR_SYSTEM;
        }
        
        iRspLen = CheckConfirmation(aucReq, aucRsp,iRet);
        
        if (iRspLen < 0)    
        {
			AEC_DRIVER_ERROR_PRINTF("CheckConfirmation error\n");
			return DRIVER_ERROR_SYSTEM;
        }

        /*offset @针对回应数据帧的运算*/
        /*为什么+2 ：因为ModbusTcpReadIoStatus函数就是单bit寄存器的函数，所以返回数据帧中字节个数位端肯定1字节*/
        offset = _MODBUS_TCP_HEADER_LENGTH + 2; //2 = 功能码 + 返回数据的字节个数
        offset_end = offset + iRspLen;

        //mark  @算法没太看懂！！
        for (i = offset; i < offset_end; i++) 
        {
            /* Shift reg hi_byte to temp */
            temp = aucRsp[i];  

                                            //iLen读取的寄存器个数，对于位变量寄存器来说
                                            //1个bit就是1个寄存器。
            for (bit = 0x01; (bit & 0xff) && (pos < iLen);) 
             {
                //分别取出每1个bit,然后赋值给dest，直到取了iLen
                //所以此处dest数组有效数据的个数 = ilen
                //并且dest数组的内容不是0x01,就是0x00
                dest[pos++] = (temp & bit) ? TRUE : FALSE;
                bit = bit << 1;
             }

        }

	}

     return iRspLen;
}

//0x01
int ModbusTcpReadBits(int iFd,unsigned short * pusTranID,
				unsigned char ucDevAddr,
				int iAddr, 
				int iLen, 
				unsigned char *dest)
{
	int iRet;

	if ((NULL == pusTranID) || (NULL == dest)) {
		return DRIVER_ERROR_PARAMETER;
	}

	if (iLen > MODBUS_MAX_READ_BITS) {
		return DRIVER_ERROR_PARAMETER;
	}

	iRet = ModbusTcpReadIoStatus(iFd,pusTranID,MODBUS_FC_READ_COILS,ucDevAddr,iAddr,iLen,dest);
	
	if (iRet == -1)
		return -1;
	else
		return iRet;
}



/***************************************************************************
* Function   : ModbusTcpReadInputBits
* Description: Creat socket、connect server
* Parameter  :
*              [Input  Parameter]
*             		iFd            @Socket file handle
*             		pusTranID      @Modbus/TCP MBAP's Trans indentifier field
                    ucDevAddr      @Device address
                    iAddr          @Regiter start address 
                    iLen           @Number of  register                     
*              [Output Parameter]
*             		dest           @each register data ,vaild data number is iLen
                                    dest's contet is 0x01 or 0x00.
* Return     :      
                    iLen           @Number of  register  
                    -1             @[fail]
* Change Logs:
*           2018/11/22  jiangfeng.zhang :add comment
***************************************************************************/
int ModbusTcpReadInputBits(int iFd,unsigned short*pusTranID,
				unsigned char ucDevAddr,
				int iAddr, 
				int iLen, 
				unsigned char *dest)
{
	int iRet;

	if ((NULL == pusTranID) || (NULL == dest)) {
		return DRIVER_ERROR_PARAMETER;
	}

	if (iLen > MODBUS_MAX_READ_BITS) {
		return DRIVER_ERROR_PARAMETER;
	}

	iRet = ModbusTcpReadIoStatus(iFd,pusTranID,MODBUS_FC_READ_DISCRETE_INPUTS,ucDevAddr,iAddr,iLen,dest);
	
	if (iRet == -1)
		return -1;
	else
		return iLen;
}
                

static int readRegisters(int iFd,unsigned short * pusTranID,
			int iFunction,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen, 
			unsigned short *usDest)
{
	int iRet;
    int iRspLen = 0,iReqLen = 0;
    

	unsigned char aucReq[_MIN_REQ_LENGTH] = {0};
    unsigned char aucRsp[MODBUS_TCP_MAX_ADU_LENGTH] = {0};

    if (iLen > MODBUS_MAX_READ_REGISTERS) {
        return DRIVER_ERROR_PARAMETER;
    }

	iReqLen = ModbusTcpPacket(pusTranID,ucDevAddr,iFunction,iAddr,iLen,aucReq);
	iRet = ModbusTcpSendMsg(iFd,aucReq,iReqLen);
	if (DRIVER_SUCCESS == iRet)
	{
        int offset;
        int i;
		iRet = ModbusTcpRecvMsg(iFd,aucRsp);
        if (iRet < 0)
            return DRIVER_ERROR_SYSTEM;

        iRspLen = CheckConfirmation(aucReq, aucRsp,iRet);
        if (iRspLen < 0)    
			return DRIVER_ERROR_SYSTEM;

        offset = _MODBUS_TCP_HEADER_LENGTH;

        for (i = 0; i < iRspLen; i++) {
            /* shift reg hi_byte to temp OR with lo_byte */
            usDest[i] = (aucRsp[offset + 2 + (i << 1)] << 8) |
                aucRsp[offset + 3 + (i << 1)];
        }
    }

    return iRspLen;
}

//0x03
int ModbusTcpReadRegisters(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen, 
			unsigned short *usDest)
{
	int iRet;

	if ((NULL == pusTranID) || (NULL == usDest)) {
		return DRIVER_ERROR_PARAMETER;
	}

	if (iLen > MODBUS_MAX_READ_REGISTERS) {
		return DRIVER_ERROR_PARAMETER;
	}

	iRet = readRegisters(iFd,pusTranID,MODBUS_FC_READ_HOLDING_REGISTERS,ucDevAddr,iAddr,iLen,usDest);
	
	return iRet;
}

//0x04
int ModbusTcpReadInputRegisters(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen, 
			unsigned short *usDest)
{
	int iRet;

	if ((NULL == pusTranID) || (NULL == usDest)) {
		return DRIVER_ERROR_PARAMETER;
	}

	if (iLen > MODBUS_MAX_READ_REGISTERS) {
		return DRIVER_ERROR_PARAMETER;
	}

	iRet = readRegisters(iFd,pusTranID,MODBUS_FC_READ_INPUT_REGISTERS,ucDevAddr,iAddr,iLen,usDest);
	
	return iRet;
}

static int writeSingle(int iFd,unsigned short * pusTranID,
			int iFunction,
			unsigned char ucDevAddr,
			int iAddr, 
			int iValue)
{
	int iRet = 0;

    int iReqLen = 0,iRspLen = 0;
    unsigned char aucReq[_MIN_REQ_LENGTH] = {0};
	unsigned char aucRsp[MODBUS_TCP_MAX_ADU_LENGTH] = {0};

	if (NULL == pusTranID) {
        return DRIVER_ERROR_PARAMETER;
    }

	iReqLen = ModbusTcpPacket(pusTranID,ucDevAddr,iFunction,iAddr,iValue,aucReq);

	printf("writeSingle ModbusTcpPacket is:\n\n");

	printf("ucDevAddr is:0x%02x\n",ucDevAddr);	
	printf("iFunction is:0x%02x\n",iFunction);
	printf("iAddr is:0x%08x\n",iAddr);
	printf("iValue is:0x%08x\n",iValue);

	
	for (iRet = 0;iRet<12;iRet++)
		{
		printf("0x%02x ",aucReq[iRet]);
	}
	printf("\n");
	
	iRet = ModbusTcpSendMsg(iFd,aucReq,iReqLen);
	if (DRIVER_SUCCESS == iRet)
	{
		iRet = ModbusTcpRecvMsg(iFd,aucRsp);
        if (iRet < 0)
        {
			AEC_DRIVER_ERROR_PRINTF("ModbusTcpRecvMsg error");
            return DRIVER_ERROR_SYSTEM;
        }
        iRspLen = CheckConfirmation(aucReq, aucRsp,iRet);
        if (iRspLen < 0)   
        {   	
			AEC_DRIVER_ERROR_PRINTF("CheckConfirmation error");
			return DRIVER_ERROR_SYSTEM;
        }
	}
    
    return iRspLen;
}


//0x05
int ModbusTcpWriteBit(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iCoilAddr, 
			int status)
{
	if (NULL == pusTranID) {
		AEC_DRIVER_ERROR_PRINTF("pusTranID is Null");
        return DRIVER_ERROR_PARAMETER;
    }

	return writeSingle(iFd,pusTranID, MODBUS_FC_WRITE_SINGLE_COIL, ucDevAddr,iCoilAddr,
                        status ? 0xFF00 : 0);
}

//0x06
int ModbusTcpWriteRegister(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int reg_addr, 
			int value)
{
	if (NULL == pusTranID) {
        return DRIVER_ERROR_PARAMETER;
    }

	return writeSingle(iFd,pusTranID, MODBUS_FC_WRITE_SINGLE_REGISTER, ucDevAddr,reg_addr,
                        value);
}

//0x0f
int ModbusTcpWriteBits(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen,  
			const unsigned char *pucSrc)
{
	int iRet=0;
    int i=0;
    int byte_count=0;
    int iReqLen = 0,iRspLen=0;
    int bit_check = 0;
    int pos = 0;
    unsigned char aucReq[MAX_MESSAGE_LENGTH] = {0};
	unsigned char aucRsp[MAX_MESSAGE_LENGTH] = {0};

    if (NULL == pusTranID) {
        return DRIVER_ERROR_PARAMETER;
    }

    if (iLen > MODBUS_MAX_WRITE_BITS) {
        return DRIVER_ERROR_PARAMETER;
    }

	iReqLen = ModbusTcpPacket(pusTranID,ucDevAddr,MODBUS_FC_WRITE_MULTIPLE_COILS,iAddr,iLen,aucReq);
	
    byte_count = (iLen / 8) + ((iLen % 8) ? 1 : 0);
    aucReq[iReqLen++] = byte_count;

    for (i = 0; i < byte_count; i++) {
        int bit;

        bit = 0x01;
        aucReq[iReqLen] = 0;

        while ((bit & 0xFF) && (bit_check++ < iLen)) {
            if (pucSrc[pos++])
                aucReq[iReqLen] |= bit;
            else
                aucReq[iReqLen] &=~ bit;

            bit = bit << 1;
        }
        iReqLen++;
    }

	iRet = ModbusTcpSendMsg(iFd,aucReq,iReqLen);
	if (DRIVER_SUCCESS == iRet)
	{
		iRet = ModbusTcpRecvMsg(iFd,aucRsp);
        if (iRet < 0)
            return DRIVER_ERROR_SYSTEM;

        iRspLen = CheckConfirmation(aucReq, aucRsp,iRet);
        if (iRspLen < 0)    
			return DRIVER_ERROR_SYSTEM;
	}

    return iRspLen;
}

//0x10
int ModbusWriteRegisters(int iFd,unsigned short * pusTranID,
			unsigned char ucDevAddr,
			int iAddr, 
			int iLen,  
			const unsigned short *pusSrc)
{
	int iRet = 0;
    int i = 0;
    int byte_count = 0;
    int iReqLen = 0,iRspLen=0;
    unsigned char aucReq[MAX_MESSAGE_LENGTH] = {0};
	unsigned char aucRsp[MAX_MESSAGE_LENGTH] = {0};

    if (NULL == pusTranID) {
        return DRIVER_ERROR_PARAMETER;
    }

    if (iLen > MODBUS_MAX_WRITE_REGISTERS) {
        return DRIVER_ERROR_PARAMETER;
    }

	iReqLen = ModbusTcpPacket(pusTranID,ucDevAddr,MODBUS_FC_WRITE_MULTIPLE_REGISTERS,iAddr,iLen,aucReq);
	
    byte_count = iLen * 2;
    aucReq[iReqLen++] = byte_count;

    for (i = 0; i < iLen; i++) {
        aucReq[iReqLen++] = pusSrc[i] >> 8;
        aucReq[iReqLen++] = pusSrc[i] & 0x00FF;
    }
	
	iRet = ModbusTcpSendMsg(iFd,aucReq,iReqLen);
	if (DRIVER_SUCCESS == iRet)
	{
		iRet = ModbusTcpRecvMsg(iFd,aucRsp);
        if (iRet < 0)
            return DRIVER_ERROR_SYSTEM;

        iRspLen = CheckConfirmation(aucReq, aucRsp,iRet);
        if (iRspLen < 0)    
			return DRIVER_ERROR_SYSTEM;
	}

    return iRspLen;
}
