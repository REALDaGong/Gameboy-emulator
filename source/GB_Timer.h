#pragma once
#include "GB.h"
#include "GB_MEMORY.h"
class Timer {
private:
	GB_BY _Sub, _Div, _Main;
	void _TimerCheck();
	void _TimerStep();
	Memory& _Memory;
public:
	void TimerInc(GB_BY delta);
	Timer(Memory& memory) : _Memory(memory) {};
	~Timer() {};
};
