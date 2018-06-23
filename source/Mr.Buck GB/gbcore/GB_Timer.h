#pragma once
#include "GB.h"
#include "GB_MEMORY.h"
class Timer {
public:
	void TimerInc(GB_BY delta);
	Timer(Memory& memory) : _Memory(memory) {
		ptrDIV = &_Memory._memoryMapio[0x4];
		ptrTIMA = &_Memory._memoryMapio[0x5];
		ptrTAC = &_Memory._memoryMapio[0x7];
		Sub = Div = 0;
	};
	~Timer() {};
private:
	GB_DB Sub, Div;//"small" timer,when overflow generate a real timer clock.
	GB_BY *ptrDIV;
	GB_BY *ptrTIMA;
	GB_BY *ptrTAC;
	Memory& _Memory;
	static const uint32_t TACtable[4];//store the selectable frequency.

};
