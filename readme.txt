1����Դ��ʾ�����linux�¶�ModbusЭ�齻��API�ķ�װ�Լ����ԡ�

       Э��㣺 modbus.c    	modbustcp.c
	          |	                         |
-----------------------------------------------------
       ������·�� 	     drivercomm.c

modbus.c : ��װ��modbus������·Э����ص�API

modbustcp.c:  ��װ��modbus_tcpЭ����ص�API

drivercomm.c: ��װ�˶��ļ��Ķ�дAPI,������modbus���л���tcpЭ�鶼������ͨ�õ�drivercomm.c���ļ����ж�д����


2�������ļ�main.c
    
     ��Դ�����modbusͨ��Э����ԣ����Ը��������modbus poll �� modbus slave��
     

