1、本源码示例完成linux下对Modbus协议交互API的封装以及测试。

       协议层： modbus.c    	modbustcp.c
	          |	                         |
-----------------------------------------------------
       数据链路： 	     drivercomm.c

modbus.c : 封装了modbus串行链路协议相关的API

modbustcp.c:  封装了modbus_tcp协议相关的API

drivercomm.c: 封装了对文件的读写API,不论是modbus串行还是tcp协议都将调用通用的drivercomm.c对文件进行读写管理。


2、测试文件main.c
    
     对源码进行modbus通信协议测试，测试辅助软件：modbus poll 与 modbus slave。
     

