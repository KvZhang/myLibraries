/*
 * LaputaComm.h
 *
 *  Created on: 2015��3��17��
 *      Author: ZhangYu
 *      Need SoftwareSerial.h
 *      D2---->Rx
 *      D3---->Tx
 */

#ifndef LAPUTACOMMSW_H_
#define LAPUTACOMMSW_H_



typedef unsigned char 		Uint8;
typedef char 				Int8;
typedef unsigned int 		Uint16;
typedef int 				Int16;
typedef unsigned long		Uint32;
typedef long 				Int32;
typedef unsigned long long 	Uint64;
typedef long long 			Int64;
//extern HardwareSerial Serial;

#define FLAME_HEADER (0x7f)//֡��ʼ
#define VERSION (02)//��2��

#define MY_ADDR_AT_EEPROM ((int)0x60)
#define SLAVE_ACCOUNT_ADDR ((int)0xA0)
#define MASTER_ADRESS (0x00)
#define MAX_DATA_SIZE (0x10)//mass data��ഫ��16���ֽڣ���Ȼ����checksum
#define FRAME_LEN (10)
//frame structure
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
//response(not used)
typedef enum
{
	CMD_OK = 0xAF,			//���յ�����checksum��ȷ���ҿ���ִ��
	CMD_NO_SENSE = 0xBE,		//���յ�����checksum��ȷ����������ִ��
	CMD_WRONG_CHKSUM = 0xCD,	//���յ�����checksum����ȷ
	CMD_UNKOWN_CMD = 0xDC,		//���յ�ָ�ʶ��

}_COMM_ACK;
//error type
typedef enum
{
	NONE_ERROR,
	ERROR1,
	ERROR2,

}_REPORT_ERROR;
//battery type
typedef enum
{
	PASSIVE_NO_BATTERY = 0xAA,	//�������ر����޵��
	PASSIVE_BATTERY = 0xBB,	//�������ر����е��
	ACTIVE_NO_BATTERY = 0xCC,	//�����ر����޵��
	ACTIVE_BATTERY = 0xDD,	//�����ر����е��
}_SLAVE_TYPE;
// supported CMD
typedef enum
{
	/*0x00*/
	SCAN_SLAVE = 0,	//�㲥Ѱ�Ҵӻ������������ã�ʹ��������ַ���ӻ�ACKʱ���ظ������ʹ���Լ���ַ���ر�����
	/*0x01*/
	REPORTING_ERROR,	//��ȡ�ӻ��Ĵ�����Ϣ
	/*0x02*/
	READ_STATE,			//��ȡ�ӻ���ǰ״̬
	/*0x03*/
	STOP_REPORT,	//ֹͣ�����㱨�ʹӻ��Ļ㱨��Ϊ, if data!=0 then stop report.
	/*0x04*/
	FALLING_SLEEP,	//�����㱨�ʹӻ��������㱨�Լ���������˯��
	/*0x05*/
	CHANGE_ADDR,	//���ĵ�ַ��������������Ч
	/*0x06*/
	READ_VOLTAGE,	//��ȡ�ӻ���ǰ��ѹ
	/*0x07*/
	EEPROM_READ,	//��ȡEEPROM������,DataHΪ��ȡ�ֽ�����DataLΪ��ȡ�׵�ַ
	/*0x08*/
	EEPROM_WRITE,	//д��EEPROM�����ݣ�DataHΪд��ĵ�ַ��DataLΪд�������
	/*0x09*/
	MASS_DATA,		//һ���Դ���������ݣ�����DataBuffer��CMDָ���ĵ�һ���ֽ�Ϊ���ݳ���(cmd ֮���ܵ��ֽ�����һ������������λ)�����16
	/*0x0A*/
	GOTO_BOOTLOADER,//jump to boot loader
	/*0x10*/
//	UNIQ_ACTION = 0x10,	//���ӻ�֮����Ҫͨ�ŵ���Ϣ�����ڴӻ�

	/*0xFF*/
	NONE_SENSE = 0xFF,	//�����壬��ʼֵ
}_CMD_LIST;
typedef struct
{
	unsigned char header;
	unsigned char version;
	unsigned char ADDR1;	//����ĵ�ַ
	unsigned char ADDR2;	//�����ڱ�����ĵ�ַ��0����Ϊ�������صĵ�ַ
	unsigned char CMD;
	unsigned char dataH;	//�߰�λ
	unsigned char dataL;	//�Ͱ�λ
	unsigned char chksumH;
	unsigned char chksumL;
	unsigned char end;
} CommStruct;

typedef struct
{
	unsigned char isReceiving :1;
	unsigned char isNeedProcess :1;
	unsigned char isWaitingForACK :1;
	unsigned char isStopAutoReporting :1;
	unsigned char chksumWrong :1;
	unsigned char getMassData :1;
	unsigned char rsvd6 :1;
	unsigned char rsvd7 :1;
} byte_val;

typedef union
{
	unsigned char val;
	byte_val bit;
} byteFlag;
typedef struct{
	unsigned char CMD;
	unsigned int data;
}rxContent;
class LaputaComm
{
public:
	unsigned long receiveMoment;	//��¼����ɨ���ʱ�䣬���ݵ�ַ��ȷ���Լ��Ļر�ʱ��
	byteFlag flag;
	unsigned char myType;
	CommStruct txRegister;
	CommStruct rxRegister;
	Uint8 dataBuffer[MAX_DATA_SIZE];
	Uint8 dataPtr;
	unsigned char lastCMD;
	unsigned int lastData;
	LaputaComm(unsigned char addr1, unsigned char addr2);
	void begin(long BDR);
	//������ص�����
	void sendCMD(unsigned char obAddr, unsigned char cmd, unsigned int data);
	void sendCMD(unsigned char themeAddr, unsigned char obAddr, unsigned char cmd, unsigned int data);
	//������ص�����

	void receiveCMD();	//�����ڻ�������δ���������ʱ�����ô˺���
	bool isNeedProcessing(void);	//�Ƿ��д���������
	bool cmdHandler(rxContent* rev);	//����0ʱ��˵����ϵͳ
//	void clearBuffer(unsigned char *reg);
	virtual ~LaputaComm();
	unsigned char myThemeAddress;
	unsigned char myAddress;

private:

	unsigned char maxSlaveNum;
};

#endif /* LAPUTACOMM_H_ */
