#pragma once
#include "GB.h"



//not use at all. 
/*


*/

class Sound {
public:
	friend class Memory;
	void ClockUp();
	void SoundWrite(GB_DB ad,GB_BY val);
	GB_BY SoundRead(GB_DB ad);
	Sound();
	~Sound() {};
private:
	GB_BY _NR10;
	//p41
	GB_BY _NR11;
	//p42
	GB_BY _NR12;
	GB_BY _NR13;
	GB_BY _NR14;
	//p43
	GB_BY _NR21;
	//p44
	GB_BY _NR22;
	GB_BY _NR23;
	GB_BY _NR24;
	//p45
	GB_BY _NR30;
	GB_BY _NR31;
	//p46
	GB_BY _NR32;
	GB_BY _NR33;
	GB_BY _NR34;
	//p47
	GB_BY _NR41;
	GB_BY _NR42;
	//p48
	GB_BY _NR43;
	GB_BY _NR44;
	//p49
	GB_BY _NR50;
	//p50
	GB_BY _NR51;
	//p51
	GB_BY _NR52;
	//FF30-3F Wave Pattern
	GB_BY Partten[16];
	struct Channel {
		GB_BY tick;
		GB_BY frequncy;
		GB_BY initvolume;
		GB_BY volume;
		GB_BY direction;
		GB_BY sweepNum;
	};
	Channel channel1, channel2;
};
