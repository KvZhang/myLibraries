/*
 * LaputaTimer.h
 *
 *  Created on: 2015Äê11ÔÂ3ÈÕ
 *      Author: zhangyu
 */

#ifndef LAPUTATIMER_H_
#define LAPUTATIMER_H_

typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long QWORD;
typedef void (*_Pfn)(void);

#define MAX_SYS_TIMER (5)
typedef struct _TIMER{
	BYTE Active;
	BYTE type;//0 non repeat, 1 repeat
	_Pfn pTimerFunc;
	unsigned int Elapse;
	unsigned long StartTime;
}SYS_TIMER;

class LaputaTimer {
public:
	LaputaTimer();
	SYS_TIMER SysTimer[MAX_SYS_TIMER];
	void processTimer();
	BYTE SetTimer(WORD Elapse, _Pfn pTimerFunc, BYTE repeatType);
	BYTE SetTimer(WORD Elapse, _Pfn pTimerFunc);
	void KillTimer(BYTE *pTimerID);
	void RestartTimer(BYTE *pTimerID);
	virtual ~LaputaTimer();
};

#endif /* LAPUTATIMER_H_ */
