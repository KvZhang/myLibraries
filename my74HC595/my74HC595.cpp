/*
 * my74HC595.cpp
 *
 *  Created on: 2015Äê3ÔÂ14ÈÕ
 *      Author: ZhangYu
 */

#include <my74HC595.h>
#include <Arduino.h>

my74HC595::my74HC595(char SCKpin, char RCKpin, char Datapin)
{
	// TODO Auto-generated constructor stub
	myType = 0;
	this->DataPin = Datapin;
	this->RCKPin = RCKpin;
	this->SCKPin = SCKpin;
	this->rowDataPin = 13;
	this->rowRCKPin = 13;
	this->rowSCKPin = 13;
	this->columnDataPin = 13;
	this->columnRCKPin = 13;
	this->columnSCKPin = 13;
}
my74HC595::my74HC595(char rowSCKpin, char rowRCKpin, char rowDatapin,
		char columnSCKpin, char columnRCKpin, char columnDatapin)
{
	// TODO Auto-generated constructor stub
	myType = 1;
	this->DataPin = 13;
	this->RCKPin = 13;
	this->SCKPin = 13;
	this->rowDataPin = rowDatapin;
	this->rowRCKPin = rowRCKpin;
	this->rowSCKPin = rowSCKpin;
	this->columnDataPin = columnDatapin;
	this->columnRCKPin = columnRCKpin;
	this->columnSCKPin = columnSCKpin;
}

my74HC595::~my74HC595()
{
	// TODO Auto-generated destructor stub
}

void my74HC595::init(void)
{
	if (this->myType == 0)
	{
		pinMode(this->DataPin, OUTPUT);
		pinMode(this->RCKPin, OUTPUT);
		pinMode(this->SCKPin, OUTPUT);
	}
	else
	{
		pinMode(this->rowDataPin, OUTPUT);
		pinMode(this->rowRCKPin, OUTPUT);
		pinMode(this->rowSCKPin, OUTPUT);
		pinMode(this->columnDataPin, OUTPUT);
		pinMode(this->columnRCKPin, OUTPUT);
		pinMode(this->columnSCKPin, OUTPUT);
	}

}
void my74HC595::Write(unsigned char data)
{
	this->Write((unsigned long) data);
}
void my74HC595::Write(unsigned int data)
{
	this->Write((unsigned long) data);
}
void my74HC595::Write(unsigned long data)
{
	digitalWrite(this->RCKPin, LOW);
	for (char i = 0; i < 32; i++)
	{
		digitalWrite(this->SCKPin, LOW);
//		delayMicroseconds(1);
		if (data & (1 << (31 - i)))
			digitalWrite(this->DataPin, HIGH);
		else
			digitalWrite(this->DataPin, LOW);
		digitalWrite(this->SCKPin, HIGH);
//		delayMicroseconds(1);
	}
	digitalWrite(this->SCKPin, LOW);
	digitalWrite(this->RCKPin, HIGH);
//	delayMicroseconds(1);
	digitalWrite(this->RCKPin, LOW);
}
void my74HC595::ScanLine(unsigned char row, unsigned char data)
{
	this->ScanLine(row, (unsigned long) data);
}
void my74HC595::ScanLine(unsigned char row, unsigned int data)
{
	this->ScanLine(row, (unsigned long) data);
}
void my74HC595::ScanLine(unsigned char row, unsigned long data)
{
	unsigned long temp = 0;
	temp = (1 << row);
//	digitalWrite(this->rowRCKPin, LOW);
//	digitalWrite(this->columnRCKPin, LOW);
	for (char i = 0; i < 24; i++)
	{
		digitalWrite(this->rowSCKPin, LOW);
		digitalWrite(this->columnSCKPin, LOW);
		//		delayMicroseconds(1);
		if (data & (1 << (23 - i)))
			digitalWrite(this->columnDataPin, HIGH);
		else
			digitalWrite(this->columnDataPin, LOW);
		if (temp & (1 << (23 - i)))
			digitalWrite(this->rowDataPin, HIGH);
		else
			digitalWrite(this->rowDataPin, LOW);
		digitalWrite(this->rowSCKPin, HIGH);
		digitalWrite(this->columnSCKPin, HIGH);
		//		delayMicroseconds(1);
	}
	digitalWrite(this->rowSCKPin, LOW);
	digitalWrite(this->rowRCKPin, HIGH);
	//	delayMicroseconds(1);
	digitalWrite(this->rowRCKPin, LOW);
	digitalWrite(this->columnSCKPin, LOW);
	digitalWrite(this->columnRCKPin, HIGH);
	//	delayMicroseconds(1);
	digitalWrite(this->columnRCKPin, LOW);
}
