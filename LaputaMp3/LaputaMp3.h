/*
 * LaputaMp3.h
 *
 *  Created on: 2015Äê4ÔÂ14ÈÕ
 *      Author: ZhangYu
 */

#ifndef LAPUTAMP3_H_
#define LAPUTAMP3_H_


typedef enum
{
	SINGLE_PLAY=0x03,
	SET_VOLUMN=0x06,
	REPEAT_PLAY=0x08,
	PAUSE_PLAY=0x0E,
}_MY_CMD;

typedef enum
{
	FINAL_SONG=0x01,
	BEEP_SONG,
	WRONG_SONG,

}_MP3_ORDER;

class LaputaMp3
{
public:
	LaputaMp3();
	void write(unsigned char cmd,unsigned int data);
	virtual ~LaputaMp3();
private:
	unsigned char txBuffer[10];
};

#endif /* LAPUTAMP3_H_ */
