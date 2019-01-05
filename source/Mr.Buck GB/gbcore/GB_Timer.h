#pragma once
#include "GB.h"
//#include "GB_MEMORY.h"
//#include "GB_InterHandler.h"
class Timer:public Hardware
{
public:
	inline void TimerInc(GB_BY delta) {
		rDIV += delta;
		Sub += delta;
		if (TimerEnable) {
			while (Sub >= TACtable[CurrentTACselect]) {//while?
				Sub -= TACtable[CurrentTACselect];
				rTIMA++;
				if (rTIMA > 0xFF) {
					rTIMA = 0;
					overflowed = 1;
				}
			}
			/*
			all units are machine cycles
			A  B
			SYS  FD FE FF|00|01|02 03 04
			TIMA FF FF FF|00|23|23 23 23
			TMA  23 23 23|23|23|23 23 23
			IF   E0 E0 E0|E0|E4|E4 E4 E4
			1.TIMA reload will happen at B,so interrupt sending does.
			2.if write to TIMA when A,TMA will not be loaded at B,and interrupt wont be sent.
			3.during B,the TIMA cant be written by mannual
			its a little hard to implement the last two rules,so i just leave them alone.
			*/
			if (overflowed && Sub >= 4) {//i guess?....
				overflowed = 0;
				_InterHandler->Send(INTERRUPT_TIMER);
				rTIMA = rTMA;
			}
		}
	}
	Timer(Hardware *interHandler):_InterHandler(interHandler){}
	~Timer() {};
	void Init();
	void TimerWrite(GB_DB ad, GB_BY val) {
		switch (ad) {
		case DIV:
			//if (rDIV & (TACtable[CurrentTACselect] >> 1)) {
			//obscure1:
			//change DIV when multiplexer selected bit is 1 will make the output become zero,causing 
			//a fall edge and the TIMA will increase.
			//rTIMA++;
			//}

			rDIV = 0;
			break;
		case TIMA:
			rTIMA = val;
			break;
		case TAC:
		{
			//obscure2,3
			//same as 1,if you produce a falling edge for Timer,TIMA will increased.
			rTAC = val;

			//GB_BY oldSelect = rTAC & 0x3;
			CurrentTACselect = val & 0x3;

			//GB_BY oldEnable = val & 0x4;
			TimerEnable = val & 0x4;
			Sub &= (TACtable[CurrentTACselect] - 1);

			//if ((rDIV&TACtable[oldSelect] > 0) && (rDIV&TACtable[CurrentTACselect] == 0)) {
			//	rTIMA++;
			//	return;
			//}
			//if(r)
			//
			//
			//no! something wrong! i change all timer after the ops are excuted,but when does the glitch happen?
		}
		break;
		case TMA:
			rTMA = val;
			break;
		}
	}

	inline GB_BY TimerRead(GB_DB ad) {
		switch (ad) {
		case TIMA:
			return (GB_BY)(rTIMA & 0xFF);
			break;
		case TAC:
			return rTAC | 0xF8;
			break;
		case DIV:
			return (GB_BY)((rDIV & 0xFF00) >> 8);
			break;
		case TMA:
			return rTMA;
			break;
		}
	}
	void Send(GB_BY val){}
private:
	GB_DB Sub;//"small" timer,when overflow generate a real timer clock.
	GB_DB rDIV;
	GB_DB rTIMA;
	GB_BY rTAC;
	GB_BY rTMA;

	GB_BY CurrentTACselect;
	GB_BY TimerEnable;
	//for Timer strange behaviours
	GB_BY overflowed;//0=no 1=just 2=last
	



	Hardware* _InterHandler;
	//InterHandler &_InterHandler;
	static const uint32_t TACtable[4];//store the selectable frequency.

};
