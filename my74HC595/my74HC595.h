/*
 * my74HC595.h
 *
 *  Created on: 2015Äê3ÔÂ14ÈÕ
 *      Author: ZhangYu
 */

#ifndef MY74HC595_H_
#define MY74HC595_H_

class my74HC595
{
public:
	my74HC595(char SCKpin, char RCKpin, char Datapin);
	my74HC595(char rowSCKpin, char rowRCKpin, char rowDatapin,
			char columnSCKpin, char columnRCKpin, char columnDatapin);
	virtual ~my74HC595();
	void init(void);
	void Write(unsigned char data);
	void Write(unsigned int data);
	void Write(unsigned long data);
	void ScanLine(unsigned char row, unsigned char data);
	void ScanLine(unsigned char row, unsigned int data);
	void ScanLine(unsigned char row, unsigned long data);

private:
	char SCKPin;
	char RCKPin;
	char DataPin;
	char rowSCKPin;
	char rowRCKPin;
	char rowDataPin;
	char columnSCKPin;
	char columnRCKPin;
	char columnDataPin;
	char myType;
};

#endif /* MY74HC595_H_ */
