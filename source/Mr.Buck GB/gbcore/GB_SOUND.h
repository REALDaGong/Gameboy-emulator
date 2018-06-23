#pragma once
#include "GB.h"



//not use at all. 




class SoundTimer {
private:
	uint32_t Timer;
	
public:
	uint32_t Limit;
	SoundTimer() {
		Limit = 16384;
		Timer = 0;
	}
	~SoundTimer() {};
	GB_BY ClockUp(GB_BY delta) {
		Timer += delta;
		if (Timer >= Limit)
		{
			Timer -= Limit;
			return 1;
		}
			
		else
			return 0;
	}
};
class FrameSequencer {
private:
	SoundTimer _Timer;
	LengthCounter LC[4];
	Envelope Ev[3];
	Sweep Sw;
	GB_BY Clock;
public:
	void Count();
	void ActiveLC();
	void ActiveEv();
	void ActiveSw();
};


class LengthCounter{//16384
private:
	SoundTimer _Timer;
	GB_BY Data;
	GB_BY Duty;
	GB_BY Length;
	GB_BY Enabled;
public:
	void load(GB_BY data);
};
class Envelope {//65536
private:
	SoundTimer _Timer;
	GB_BY Data;
	GB_BY Behaviour;
	GB_BY Enabled;
public:
	void newVolume();
	void load();
};
class Sweep {//32768
private:
	SoundTimer _Timer;
	GB_BY Data;
	GB_BY Enabled;
public:
	void trigger();
	void load();
	void checkOverflow();
	void analyze();
};
class NoiseChannal {

};
class Channal {

};