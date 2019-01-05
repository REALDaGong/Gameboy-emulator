
/*
Timer details
     ______________
    /     DIV	   \					<---	DIV_write will reset them all
    |7 6 5 4 3 2 1 0| 7 6 5 4 3 2 1 0<----SYS clock
	             |    |   |   |
				 0    3   2   1   <---multiplexer-------TAC_FreqChoose
				 --------------
				        | multiplexer output
						|				
					   AND<-----------------------------TAC_enable
					    | output
						|
				if dectected falling edge--->TIMA++
*/


#include "GB_Timer.h"


const uint32_t Timer::TACtable[4] = { 1024,16,64,256 };

void Timer::Init() {
	rDIV = 0;
	rTMA = 0;
	rTIMA = 0;
	rTAC = 0;
	Sub = 0;
	CurrentTACselect = 0;
	TimerEnable = 0;
	overflowed = 0;
}

