#pragma once
//without any external gameboy actually.
//it not means you can play with two player,just make some games work correctly
//i.e. spy vs spy
#include "GB.h"

class SerialPort {
public:

	GB_BY State;//start or end.

	void addClock(GB_BY delta);

	GB_BY ReadData() { return Data; }

	void WriteData(GB_BY data) { Data = data; }

	SerialPort() { Limit = 512; };

	~SerialPort() {};
private:
	GB_DB Clock;
	GB_DB Limit;

	GB_BY Data;
	GB_BY ShiftBit;

};