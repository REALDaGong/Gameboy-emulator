#pragma once
#include "GB.h"
#include <iostream>
class Timer:public Hardware
{
public:
	inline void TimerInc(GB_BY delta) {
		rDIV += delta;
		Sub += delta;
		if (TimerEnable) {
			if (Sub == TACtable[CurrentTACselect]) {
				Sub = 0;
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
			if (overflowed) {//i guess?....
				if (Sub == 4) { rTIMA = rTMA; _InterHandler->Send(INTERRUPT_TIMER); }
				
				if (Sub == 5) {
					rTIMA = rTMA;
					overflowed = 0;
				}
			}
		}
	}
	Timer(Hardware *interHandler) :_InterHandler(interHandler) { Sub = 0; }
	~Timer() {};
	void Init();
	void TimerWrite(GB_DB ad, GB_BY val) {
		switch (ad) {
		case DIV:
			if ((rTAC & 0x4) && (rDIV & (TACtable[CurrentTACselect] >> 1))) {
			//obscure1:
			//clear DIV when multiplexer selected bit is 1 will make the output become zero,causing 
			//a fall edge and the TIMA will increase.
			rTIMA++;
			}

			rDIV = 0;
			Sub = 0;//see the picture in .cpp,Sub is just the low bits(which ones are set by TAC) of the timer,DIV is the high 8 bits.
			break;
		case TIMA:
			if(overflowed){
				if (Sub < 4) {
					overflowed = 0;
					rTIMA = val;
				}
				else
					break;
			}
			rTIMA = val;
			break;
		case TAC:
		{
			//obscure2,3
			//same as 1,if you produce a falling edge for Timer,TIMA will increased.
			

			GB_BY oldSelect = rTAC & 0x3;
			GB_BY oldEnable = rTAC & 0x4;
			rTAC = val;
			CurrentTACselect = val & 0x3;
			TimerEnable = val & 0x4;
			Sub &= (TACtable[CurrentTACselect] - 1);
			//if old selected bit is high and new selected bit is low:
			if ((rDIV&(TACtable[oldSelect]>>1)) && ((rDIV&(TACtable[CurrentTACselect]>>1)) == 0)) {
				rTIMA++;
				return;
			}
			//if old selected bit is high and disable it:
			if ((rDIV&(TACtable[oldSelect]>>1)) && oldEnable == 1 && TimerEnable == 0) {
				rTIMA++;
				return;
			}
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
	GB_DB Sub;
	GB_DB rDIV;
	GB_DB rTIMA;
	GB_BY rTAC;
	GB_BY rTMA;

	GB_BY CurrentTACselect;
	GB_BY TimerEnable;
	//for Timer strange behaviours
	GB_BY overflowed;//0=no 1=just 2=last
	GB_DB oldDIV;



	Hardware* _InterHandler;
	
	static const uint32_t TACtable[4];//store the selectable frequency.

};
