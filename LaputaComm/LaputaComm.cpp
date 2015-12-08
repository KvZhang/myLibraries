/*
 * LaputaComm.cpp
 *
 *  Created on: 2015年3月17日
 *      Author: ZhangYu
 */

#include "LaputaComm.h"
#include "Arduino.h"
#include "EEPROM.h"
LaputaComm::LaputaComm(unsigned char addr1, unsigned char addr2,
		unsigned char meType) {
	// TODO Auto-generated constructor stub
	this->myType = meType;
	this->myThemeAddress = addr1;
	this->maxSlaveNum = 15; //默认值
	this->receiveMoment = 0;
	this->flag.val = 0;
	if (EEPROM.read(MY_ADDRESS) == 0xFF) {
		this->myAddress = addr2;
	} else {
		this->myAddress = EEPROM.read(MY_ADDRESS);
	}
	this->txRegister.header = 0x7f;
	this->txRegister.version = VERSION;
	this->txRegister.ADDR1 = myThemeAddress;
	this->txRegister.ADDR2 = myAddress;
	this->txRegister.CMD = 0xff;
	this->txRegister.dataH = 0;
	this->txRegister.dataL = 0;
	this->txRegister.chksumH = 0;
	this->txRegister.chksumL = 0;
	this->txRegister.end = 0xfe;

	this->rxRegister.header = 0x7f;
	this->rxRegister.version = VERSION;
	this->rxRegister.ADDR1 = myThemeAddress;
	this->rxRegister.ADDR2 = myAddress;
	this->rxRegister.CMD = 0xff;
	this->rxRegister.dataH = 0;
	this->rxRegister.dataL = 0;
	this->rxRegister.chksumH = 0;
	this->rxRegister.chksumL = 0;
	this->rxRegister.end = 0xfe;
	for (byte i = 0; i < sizeof(dataBuffer); i++) {
		dataBuffer[i] = 0;
	}
	dataPtr = 0;
}
void LaputaComm::sendCMD(unsigned char obAddr, unsigned char cmd,
		unsigned int data) {
	unsigned char *ptr = (unsigned char*) &this->txRegister;
	unsigned int chksum = 0;

//	this->txRegister.version = VERION;
	this->txRegister.ADDR2 = obAddr;
	this->txRegister.CMD = cmd;
	this->txRegister.dataH = data / 256;
	this->txRegister.dataL = data % 256;
	for (unsigned char i = 1; i < 7; i++) {
		chksum += *(ptr + i);
	}
	chksum = ~chksum + 1;
	this->txRegister.chksumH = chksum / 256;
	this->txRegister.chksumL = chksum % 256;

	while(this->flag.bit.isReceiving){
		;
	}
	Serial.write(ptr, 10);
}
void LaputaComm::sendMassData(Uint8 obAddr, Uint8 *massData, Uint8 len) {
	unsigned char *ptr = (unsigned char*) &this->txRegister;
	unsigned int chksum = 0;

//	this->txRegister.version = VERION;
	this->txRegister.ADDR2 = obAddr;
	this->txRegister.CMD = MASS_DATA;
	for (unsigned char i = 1; i < 5; i++) {
		chksum += *(ptr + i);
	}
	chksum+=len;
	for(Uint8 i=0;i<len;i++){
		chksum+=*(massData+i);
	}
	chksum = ~chksum + 1;
	this->txRegister.chksumH = chksum / 256;
	this->txRegister.chksumL = chksum % 256;

	Serial.write(ptr, 5);
	Serial.write(&len,1);
	Serial.write(massData,len);
	Serial.write(ptr+7,3);
}
unsigned char LaputaComm::getACK(void) {
	return this->getACK(1000); //默认1s超时时间
}
unsigned char LaputaComm::getACK(unsigned int timeOut) {
	unsigned long moment = millis();
	unsigned char temp = 0;
	this->flag.bit.isWaitingForACK = 1;
	while ((millis() - moment) < timeOut) {
		if (Serial.available() > 0) {
			temp = Serial.read();
			this->flag.bit.isWaitingForACK = 0;
			return temp; //默认1s超时时间
		}
	}
	this->flag.bit.isWaitingForACK = 0;
	return 0; //time out

}
void LaputaComm::receiveCMD() {
	static unsigned char status = STATUS_HEADER;
	static unsigned int checksum = 0;
	unsigned temp = 0;

	if (this->flag.bit.isWaitingForACK)
		return;

	while (Serial.available() > 0) {
		temp = Serial.read();
		switch (status) {
		case STATUS_HEADER:
			if (temp == FLAME_HEADER) {
				status = STATUS_VERSION;
				this->flag.bit.isReceiving = 1;
			}
			break;
		case STATUS_VERSION:
			if (temp == VERSION) {
				checksum = temp;
				status = STATUS_ADDR1;
			} else {
				status = STATUS_HEADER;
				this->flag.bit.isReceiving = 0;
			}
			break;
		case STATUS_ADDR1:
			this->rxRegister.ADDR1 = temp;
			checksum += temp;
			status = STATUS_ADDR2;
			break;
		case STATUS_ADDR2:
			this->rxRegister.ADDR2 = temp;
			checksum += temp;
			status = STATUS_CMD;
			break;
		case STATUS_CMD:
			this->rxRegister.CMD = temp;
			checksum += temp;
			if (temp == MASS_DATA) {
				status = STATUS_MASS_DATA;
				dataPtr = 0;
			} else {
				status = STATUS_DATA_H;
			}
			break;
		case STATUS_DATA_H:
			this->rxRegister.dataH = temp;
			checksum += temp;
			status = STATUS_DATA_L;
			break;
		case STATUS_DATA_L:
			this->rxRegister.dataL = temp;
			checksum += temp;
			status = STATUS_CHKSUM_H;
			break;
		case STATUS_CHKSUM_H:
			this->rxRegister.chksumH = temp;
			status = STATUS_CHKSUM_L;
			break;
		case STATUS_CHKSUM_L:
			this->rxRegister.chksumL = temp;
			status = STATUS_END;
			break;
		case STATUS_END:
			status = STATUS_HEADER;
			this->flag.bit.isReceiving = 0;
			receiveMoment = millis();
			{
				if ((this->rxRegister.CMD == SCAN_SLAVE
						|| this->rxRegister.ADDR2 == this->myAddress)
						&& this->rxRegister.ADDR1 == myThemeAddress) {
					this->flag.bit.isNeedProcess = 1;
					if ((~checksum + 1)
							!= (unsigned int) (this->rxRegister.chksumL
									+ this->rxRegister.chksumH * 256))
						this->flag.bit.chksumWrong = 1;
					else
						this->flag.bit.chksumWrong = 0;
				}
			}
//			else Serial.write(CMD_WRONG_CHKSUM);
			break;
		case STATUS_MASS_DATA:
			this->dataBuffer[this->dataPtr] = temp;
			checksum += temp;

			if (dataPtr == this->dataBuffer[0]) {
				status = STATUS_CHKSUM_H;
			} else {
				dataPtr++;
			}

			break;
		default:
			break;
		}
	}
}
bool LaputaComm::isNeedProcessing(void) //是否有待处理的命令？
		{
	return this->flag.bit.isNeedProcess;
}
bool LaputaComm::isNewMassDataReceive(void) //是否有待处理的命令？
		{
	return this->flag.bit.getMassData;
}
void LaputaComm::clearDataBuffer(void){
	for(Uint8 i=0;i<sizeof(this->dataBuffer);i++){
		this->dataBuffer[i]=0;
	}
	this->flag.bit.getMassData=0;

}
unsigned char LaputaComm::cmdHandler() //返回0时，说明是系统
{
	this->flag.bit.isNeedProcess = 0;
	if (this->myAddress) //地址不为0，即为主机
	{
		switch (this->rxRegister.CMD) {
		case SCAN_SLAVE: {
			if (this->flag.bit.isPermitAutoReporting == 0
					&& this->flag.bit.chksumWrong == 0) {
				while ((millis() - this->receiveMoment)
						< (this->myAddress - 1) * 200)
					; //允许回应的时间间隔为（addr2-1）*200ms至addr2*200ms
				if ((millis() - this->receiveMoment)
						< (this->myAddress) * 200) {
					this->sendCMD(MASTER_ADRESS, SCAN_SLAVE, myType);
					unsigned char temp = this->getACK(200);
					if (temp == 0 || temp == CMD_OK) {
						this->flag.bit.isPermitAutoReporting = 1;
					}
				}
			}

			break;
		}

		case REPORTING_ERROR:
			if (this->flag.bit.chksumWrong == 0)
				this->sendCMD(MASTER_ADRESS, REPORTING_ERROR, gError);
			break;

		case READ_STATE:
			if (this->flag.bit.chksumWrong == 0)
				this->sendCMD(MASTER_ADRESS, READ_STATE, gState);
			break;
		case READ_VOLTAGE:
			if (this->flag.bit.chksumWrong == 0)
				this->sendCMD(MASTER_ADRESS, READ_VOLTAGE, batteryVolt);
			break;
		case STOP_REPORT:
			if (this->flag.bit.chksumWrong == 0) {
				this->flag.bit.isPermitAutoReporting = 0;
//				Serial.write(CMD_OK);
			} else
//				Serial.write(CMD_WRONG_CHKSUM);
				;
			break;
		case CHANGE_ADDR:
			if (this->flag.bit.chksumWrong == 0) {
				this->myAddress = this->rxRegister.dataL;
				EEPROM.write(MY_ADDRESS, this->myAddress);
//				Serial.write(CMD_OK);
			} else
//				Serial.write(CMD_WRONG_CHKSUM);
				;
			break;
		case EEPROM_READ:
			if (this->flag.bit.chksumWrong == 0) {
				for (unsigned char i = 0; i < this->rxRegister.dataH; i++) {
					this->sendCMD(MASTER_ADRESS, EEPROM_READ,
							EEPROM.read(this->rxRegister.dataL + i));
				}
			} else
//				Serial.write(CMD_WRONG_CHKSUM);
				;
			break;
		case EEPROM_WRITE:
			if (this->flag.bit.chksumWrong == 0) {
				EEPROM.write(this->rxRegister.dataH, rxRegister.dataL);
//				Serial.write(CMD_OK);
				;
			} else
//				Serial.write(CMD_WRONG_CHKSUM);
				;
			break;
		case MASS_DATA:
			if (this->flag.bit.chksumWrong == 0) {
				this->flag.bit.getMassData=1;
			} else
//				Serial.write(CMD_WRONG_CHKSUM);
				;
			break;
		case UNIQ_ACTION:
			if (this->flag.bit.chksumWrong == 0) {
//				Serial.write(CMD_OK);
				;
				return this->rxRegister.dataL;
			} else
				Serial.write(CMD_WRONG_CHKSUM);
			break;
		default:
			if (this->flag.bit.chksumWrong == 0) {
//				Serial.write(CMD_NO_SENSE);
				;
			} else
//				Serial.write(CMD_WRONG_CHKSUM);
				;
			break;
		}
	}

	return 0;
}
LaputaComm::~LaputaComm() {
// TODO Auto-generated destructor stub
}

