#pragma once
#include "GB.h"
#include "GB_MEMORY.h"
class Timer {
private:
	GB_BY _Sub, _Div, _Main;
	GB_BY *ptrDIV;
	void _TimerCheck();
	void _TimerStep();
	Memory& _Memory;
public:
	void TimerInc(GB_BY delta);
	Timer(Memory& memory) : _Memory(memory) { ptrDIV = &_Memory._memory_mapio[0x4]; };
	~Timer() {};
};
