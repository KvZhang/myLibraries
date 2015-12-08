/*
 * LaputaMp3.cpp
 *
 *  Created on: 2015Äê4ÔÂ14ÈÕ
 *      Author: ZhangYu
 */

#include <LaputaMp3.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

LaputaMp3::LaputaMp3()
{
	// TODO Auto-generated constructor stub
	this->txBuffer[0] = 0x7e;
	this->txBuffer[1] = 0xff;
	this->txBuffer[2] = 0x06;
	this->txBuffer[3] = 0x00;
	this->txBuffer[4] = 0x00;
	this->txBuffer[5] = 0x00;
	this->txBuffer[6] = 0x00;
	this->txBuffer[7] = 0x00;
	this->txBuffer[8] = 0x00;
	this->txBuffer[9] = 0xef;
	mySerial.begin(9600);
}

LaputaMp3::~LaputaMp3()
{
	// TODO Auto-generated destructor stub

}
void LaputaMp3::write(unsigned char cmd, unsigned int data)
{
	unsigned int checkSum = 0;
	this->txBuffer[3] = cmd;
	this->txBuffer[5] = data / 256;
	this->txBuffer[6] = data % 256;
	for (unsigned i = 1; i <= 6; i++)
	{
		checkSum += this->txBuffer[i];
	}
	checkSum = ~checkSum + 1;
	this->txBuffer[7] = checkSum / 256;
	this->txBuffer[8] = checkSum % 256;
	mySerial.write(this->txBuffer, 10);


}
