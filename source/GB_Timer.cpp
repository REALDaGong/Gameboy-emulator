#include "GB_Timer.h"
const uint32_t Timer::TACtable[4] = { 1024,16,64,256 };
void Timer::TimerInc(GB_BY delta) {
	_Div += delta;
	if (_Div >= 256) {
		_Div -= 256;
		(*ptrDIV)++;
	}
	if ((*ptrTAC) & 0x4) {
		_Sub += delta;
		while (_Sub >= TACtable[(*ptrTAC) & 0x3]) {
			_Sub -= TACtable[(*ptrTAC) & 0x3];
			(*ptrTIMA)++;
			if (*ptrTIMA == 0) {
				_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF) | 0x4);
				*ptrTIMA = _Memory.MemoryRead(TMA);
			}
		}
	}
}