#include "GB_SERIALIO.h"

void SerialPort::addClock(GB_BY delta) {
	Clock += delta;
	if (Clock >= Limit) {
		Clock -= Limit;
		Data <<= 1;
		Data |= 1;//didnt get any message at all.
		ShiftBit++;
		if (ShiftBit == 8) {
			ShiftBit = 0;
			State = 0;
		}//automatically end after 1 byte "transfered"
	}
}
