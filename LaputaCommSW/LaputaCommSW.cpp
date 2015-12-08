/*
 * LaputaComm.cpp
 *
 *  Created on: 2015年3月17日
 *      Author: ZhangYu
 */

#include "LaputaCommSW.h"
#include "Arduino.h"
#include "EEPROM.h"
#include"SoftwareSerial.h"

SoftwareSerial mySerial(2, 3); //D2 for RX and D3 for TX
LaputaComm::LaputaComm(unsigned char addr1, unsigned char addr2) {
	// TODO Auto-generated constructor stub
	this->myType = ACTIVE_NO_BATTERY;
	this->myThemeAddress = addr1;
	this->maxSlaveNum = 15; //默认值
	this->receiveMoment = 0;
	this->flag.val = 0;
	if (EEPROM.read(MY_ADDR_AT_EEPROM) == 0xFF) {
		this->myAddress = addr2;
	} else {
		this->myAddress = EEPROM.read(MY_ADDR_AT_EEPROM);
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
	this->lastCMD = 0xff;
	this->lastData = 0;
}
void LaputaComm::begin(long BDR) {
	mySerial.begin(BDR);
}
void LaputaComm::sendCMD(unsigned char obAddr, unsigned char cmd,
		unsigned int data) {
	unsigned char *ptr = (unsigned char*) &this->txRegister;
	unsigned int chksum = 0;

//	this->txRegister.version = VERION;
	this->txRegister.ADDR1=this->myThemeAddress;
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

	while (this->flag.bit.isReceiving) {
		;
	}
	if (this->flag.bit.isStopAutoReporting)
		return;
	mySerial.write(ptr, 10);
	this->txRegister.CMD = 0xff;
	this->txRegister.dataH = 0;
	this->txRegister.dataL = 0;
}
void LaputaComm::sendCMD(unsigned char themeAddr, unsigned char obAddr,
		unsigned char cmd, unsigned int data) {
	this->txRegister.ADDR1 = themeAddr;
	this->sendCMD(obAddr, cmd, data);
	this->txRegister.ADDR1 = this->myThemeAddress;
}
void LaputaComm::receiveCMD() {
	static unsigned char status = STATUS_HEADER;
	static unsigned int checksum = 0;
	unsigned temp = 0;

	//over time, a frame should not longer than 100ms
	if(this->receiveMoment>0 && (millis()-this->receiveMoment)>100){
		status = STATUS_HEADER;
		this->flag.bit.isReceiving = 0;
		this->receiveMoment=0;
	}
	while (mySerial.available() > 0) {
		temp = mySerial.read();
		switch (status) {
		case STATUS_HEADER:
			if (temp == FLAME_HEADER) {
				status = STATUS_VERSION;
				this->flag.bit.isReceiving = 1;
				this->receiveMoment = millis();//this frame is starting
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
			{
				if ((~checksum + 1)
						!= (unsigned int) (this->rxRegister.chksumL
								+ this->rxRegister.chksumH * 256))
					return;
				//handle stop cmd to prepare for boot loader
				if (this->rxRegister.ADDR2 == 0
						&& this->rxRegister.CMD == STOP_REPORT) {
					if (this->rxRegister.dataL || this->rxRegister.dataH)
						this->flag.bit.isStopAutoReporting = 1;
					else
						this->flag.bit.isStopAutoReporting = 0;
				}
				//handle boot loader cmd
				if((this->rxRegister.CMD == GOTO_BOOTLOADER
						|| this->rxRegister.ADDR2 == this->myAddress)
						&& this->rxRegister.ADDR1 == myThemeAddress){
					asm("JMP 15872 ");//0x3E00
				}
				//other cmd need to deal with
				if ((this->rxRegister.CMD == SCAN_SLAVE
						|| this->rxRegister.ADDR2 == this->myAddress)
						&& this->rxRegister.ADDR1 == myThemeAddress) {
						this->flag.bit.isNeedProcess = 1;
				}
			}
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
bool LaputaComm::cmdHandler(rxContent* rev) //返回0时，说明是系统
		{
	this->flag.bit.isNeedProcess = 0;
	if (this->myAddress) //地址不为0，即为主机
	{
		switch (this->rxRegister.CMD) {
		case SCAN_SLAVE: {
			;
			break;
		}

		case REPORTING_ERROR:
			;
			break;

		case READ_STATE:
			;
			break;
		case READ_VOLTAGE:
			;
			break;
		case STOP_REPORT:
			if (this->flag.bit.chksumWrong == 0) {
				if (this->rxRegister.dataL) {
					this->flag.bit.isStopAutoReporting = 1;
				} else {
					this->flag.bit.isStopAutoReporting = 0;
				}

			}
			break;
		case CHANGE_ADDR:
			if (this->flag.bit.chksumWrong == 0) {
				this->myAddress = this->rxRegister.dataL;
				EEPROM.write(MY_ADDR_AT_EEPROM, this->myAddress);
			}
			break;
		case EEPROM_READ:
			if (this->flag.bit.chksumWrong == 0) {
				for (unsigned char i = 0; i < this->rxRegister.dataH; i++) {
					this->sendCMD(MASTER_ADRESS, EEPROM_READ,
							EEPROM.read(this->rxRegister.dataL + i));
				}
			}
			break;
		case EEPROM_WRITE:
			if (this->flag.bit.chksumWrong == 0) {
				EEPROM.write(this->rxRegister.dataH, rxRegister.dataL);
			}
			break;
		case MASS_DATA:
			if (this->flag.bit.chksumWrong == 0) {
				this->flag.bit.getMassData = 1;
			}
			break;
		case NONE_SENSE:
			if (this->flag.bit.chksumWrong == 0) {
			}
			break;
		default:
			if (this->flag.bit.chksumWrong == 0
					&& this->rxRegister.CMD < 0xff) {
				rev->CMD = this->rxRegister.CMD;
				rev->data = this->rxRegister.dataH * 256
						+ this->rxRegister.dataL;
				this->rxRegister.CMD = 0xff;
				this->rxRegister.dataH = 0;
				this->rxRegister.dataL = 0;
				return 1;
			}
			break;
		}
	}
	this->rxRegister.CMD = 0xff;
	this->rxRegister.dataH = 0;
	this->rxRegister.dataL = 0;
	return 0;
}
LaputaComm::~LaputaComm() {
// TODO Auto-generated destructor stub
}

