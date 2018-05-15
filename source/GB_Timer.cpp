#include "GB_Timer.h"
void Timer::TimerInc(GB_BY delta) {
	;

	if ((_Sub += delta) >= 16) {
		_Main++;
		_Sub -= 16;
		_Div++;
		if (_Div == 16) {
			_Memory.MemoryWrite(DIV, (_Div + 1) & 255);
			_Div = 0;
		}
	}
	_TimerCheck();
}
void Timer::_TimerCheck() {
	int speed;
	if (_Memory.MemoryRead(TAC) & 4) {
		switch (_Memory.MemoryRead(TAC) & 3) {
		case 0:
			speed = 64;
			break;
		case 1:
			speed = 1;
			break;
		case 2:
			speed = 4;
			break;
		case 3:
			speed = 16;
			break;
		}
		if (_Main >= speed)_TimerStep();
	}
}
void Timer::_TimerStep() {
	_Main = 0;
	_Memory.MemoryWrite(TIMA, _Memory.MemoryRead(TIMA) + 1);
	if (_Memory.MemoryRead(TIMA) == 0) {
		_Memory.MemoryWrite(TIMA, _Memory.MemoryRead(TMA));
		_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF) | 0x4);
	}
}