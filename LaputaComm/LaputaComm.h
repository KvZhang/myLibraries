/*
 * LaputaComm.h
 *
 *  Created on: 2015年3月17日
 *      Author: ZhangYu
 */

#ifndef LAPUTACOMM_H_
#define LAPUTACOMM_H_
typedef unsigned char 		Uint8;
typedef char 				Int8;
typedef unsigned int 		Uint16;
typedef int 				Int16;
typedef unsigned long		Uint32;
typedef long 				Int32;
typedef unsigned long long 	Uint64;
typedef long long 			Int64;
//extern HardwareSerial Serial;
extern unsigned int gState;
extern unsigned int batteryVolt;//真实值放大1000倍
extern unsigned int gError;

#define FLAME_HEADER (0x7f)//帧起始
#define VERSION (01)//第一版

#define MY_ADDRESS ((int)0x60)
#define SLAVE_ACCOUNT_ADDR ((int)0xA0)
#define MASTER_ADRESS (0x00)
#define MAX_DATA_SIZE (0x10)//mass data最多传递16个字节，依然计算checksum
#define FRAME_LEN (10)

typedef enum
{
	STATUS_HEADER,
	STATUS_VERSION,
	STATUS_ADDR1,
	STATUS_ADDR2,
	STATUS_CMD,
	STATUS_DATA_H,
	STATUS_DATA_L,
	STATUS_CHKSUM_H,
	STATUS_CHKSUM_L,
	STATUS_END,
	STATUS_MASS_DATA,
}_REV_STATUS;
typedef enum
{
	CMD_OK = 0xAF,			//接收的数据checksum正确，且可以执行
	CMD_NO_SENSE = 0xBE,		//接收的数据checksum正确，但不可以执行
	CMD_WRONG_CHKSUM = 0xCD,	//接收的数据checksum不正确
	CMD_UNKOWN_CMD = 0xDC,		//接收的指令不识别

}_COMM_ACK;
typedef enum
{
	NONE_ERROR,
	ERROR1,
	ERROR2,

}_REPORT_ERROR;
typedef enum
{
	PASSIVE_NO_BATTERY = 0xAA,	//不主动回报，无电池
	PASSIVE_BATTERY = 0xBB,	//不主动回报，有电池
	ACTIVE_NO_BATTERY = 0xCC,	//主动回报，无电池
	ACTIVE_BATTERY = 0xDD,	//主动回报，有电池
}_SLAVE_TYPE;
typedef enum
{
	/*0x00*/SCAN_SLAVE = 0,	//广播寻找从机，仅主机可用，使用主机地址，从机ACK时需重复此命令，使用自己地址及回报类型
	/*0x01*/
	REPORTING_ERROR,	//读取从机的错误信息
	/*0x02*/
	READ_STATE,			//读取从机当前状态
	/*0x03*/
	STOP_REPORT,	//停止主动汇报型从机的汇报行为
	/*0x04*/
	FALLING_SLEEP,	//主动汇报型从机向主机汇报自己即将进入睡眠
	/*0x05*/
	CHANGE_ADDR,	//更改地址，包括主机均有效
	/*0x06*/
	READ_VOLTAGE,	//读取从机当前电压
	/*0x07*/
	EEPROM_READ,	//读取EEPROM的内容,DataH为读取字节数，DataL为读取首地址
	/*0x08*/
	EEPROM_WRITE,	//写入EEPROM的内容，DataH为写入的地址，DataL为写入的内容
	/*0x09*/
	MASS_DATA,		//一次性传输大量数据，存于DataBuffer，CMD指令后的第一个字节为数据长度(cmd 之后总的字节数减一，即不包括此位)，最大16
	/*0xA0*/
	UNIQ_ACTION = 0x10,	//主从机之间需要通信的信息，用于从机

	/*0xFF*/
	NONE_SENSE = 0xFF,	//无意义，初始值
}_CMD_LIST;
typedef struct _COMM_STRUCT
{
	unsigned char header;
	unsigned char version;
	unsigned char ADDR1;	//主题的地址
	unsigned char ADDR2;	//机关在本主题的地址，0保留为主题主控的地址
	unsigned char CMD;
	unsigned char dataH;	//高八位
	unsigned char dataL;	//低八位
	unsigned char chksumH;
	unsigned char chksumL;
	unsigned char end;
} CommStruct;

typedef struct _byte_val
{
	unsigned char isReceiving :1;
	unsigned char isNeedProcess :1;
	unsigned char isWaitingForACK :1;
	unsigned char isPermitAutoReporting :1;
	unsigned char chksumWrong :1;
	unsigned char getMassData :1;
	unsigned char rsvd6 :1;
	unsigned char rsvd7 :1;
} byte_val;

typedef union _byte_flag
{
	unsigned char val;
	byte_val bit;
} byteFlag;

class LaputaComm
{
public:
	unsigned long receiveMoment;	//记录主机扫描的时间，根据地址，确定自己的回报时间
	byteFlag flag;
	unsigned char myType;
	CommStruct txRegister;
	CommStruct rxRegister;
	Uint8 dataBuffer[MAX_DATA_SIZE];
	Uint8 dataPtr;
	LaputaComm(unsigned char addr1, unsigned char addr2, unsigned char meType);
	//发送相关的命令
	void sendCMD(unsigned char obAddr, unsigned char cmd, unsigned int data);
	void sendMassData(Uint8 obAddr, Uint8 *massData, Uint8 len);
	unsigned char getACK(unsigned int timeOut);
	unsigned char getACK(void);
	//接收相关的命令
	void receiveCMD();	//当串口缓冲区有未处理的数据时，调用此函数
	bool isNeedProcessing(void);	//是否有待处理的命令？
	bool isNewMassDataReceive(void);	//是否有待处理的命令？
	void clearDataBuffer(void);
	unsigned char cmdHandler();	//返回0时，说明是系统
	unsigned char cmdHandler(Uint8 *revBuffer);	//返回0时，说明是系统
	virtual ~LaputaComm();
private:

	unsigned char myThemeAddress;
	unsigned char myAddress;
	unsigned char maxSlaveNum;
};

#endif /* LAPUTACOMM_H_ */
