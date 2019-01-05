#pragma once
//recieve all inter, send them to memory inter area,use this to spilt the hardware classes.
#include "GB.h"
#include <queue>
//still need to inplement obscure behaviours.
class APU:public Hardware 
{
public:
	void Send(GB_BY nothing) {};
	void Init();
	void APUWrite(GB_DB ad, GB_BY val);
	GB_BY APURead(GB_DB ad);
	void SendClock(GB_BY delta);
private:
	/*
	//square wave 1,2
	GB_BY _NR10, _NR20; //1:Sweep period,negate ,shift 2:not used				-PPP NSSS
	GB_BY _NR11, _NR21; //Duty,Length load (64-L)								DDLL LLLL
	GB_BY _NR12, _NR22;//starting volume,envelope add mode,period				VVVV APPP
	GB_BY _NR13, _NR23;//freq LSB												FFFF FFFF
	GB_BY _NR14, _NR24;//Trigger, Length enable, Frequency MSB					TL-- -FFF
	//pattern wave
	GB_BY _NR30;//DAC power														E--- ----
	GB_BY _NR31;//length load (256-L)											LLLL LLLL
	GB_BY _NR32;//volume code 00=0% 01=100% 10=50% 11=25%						-VV- ----
	GB_BY _NR33;//same as square wave
	GB_BY _NR34;//same
	//noise 
	GB_BY _NR41;//length load (64-L)
	GB_BY _NR42;//same as square
	GB_BY _NR43;//clock shift,width mode of LFSR,Divisor code					SSSS WDDD
	GB_BY _NR44;//trigger and length enable
	//control/status
	GB_BY _NR50;//Vin L enable,left vol,Vin B enable,right vol					ALLL BRRR
	GB_BY _NR51;//left enables,right enables										NW21 NW21
	GB_BY _NR52;//Power control/status,channel length statuses					P--- NW21
	*/
	const GB_DB offset = 0xFF10;
	GB_BY Reg[0x20];//base is 0xFF10,end at 0xFF2F useless now.
	GB_BY LengthLoad[4];
	GB_BY LengthCounter[4];
	GB_BY Duty[2];

	GB_BY VolSet[4];//for wave is 0-3,the others are 0-15
	GB_BY VolOut[4];

	GB_BY VolEnvPeriod[3];
	GB_BY EnvTimer[3];
	GB_BY EnvStop[3];
	GB_BY VolEnvMode[3];
	//No wave channel;

	GB_BY SweepPeriod;
	GB_BY SweepTimer;
	GB_BY Negate;
	GB_BY SweepShift;
	GB_BY SweepEnable;
	GB_DB Shadow;
	//for square 1 's sweep

	GB_BY Trigger[4];//used for marking a channel is enabled or not in my codes.
	GB_BY LengthEnable[4];

	GB_DB Freq[4];
	//noise channel is different

	GB_BY WidthMode;
	GB_BY WidthModeOn;//?
	GB_DB LFSR;
	GB_BY Shift;
	GB_BY DivPtr;

	GB_BY DACEnable[4];
	GB_BY PatternTable[0x10];
	GB_BY PtrHead;//0->High four,1->low four
	std::vector<GB_BY> Sample;
	struct Timer{
		GB_DB limit;
		GB_DB current;
	};
	Timer _Timer[5];

	inline void Square1();
	inline void Square2();
	inline void Wave();
	inline void Noise();//the outputs come out of these funcs.

	void Sequence();
	GB_BY step;



	GB_BY DeviceVersion;//for further use.
	GB_BY Power;
	GB_BY Lopen, Ropen;
	GB_BY LVin, RVin;
	void PowerON();
	void PowerOFF();

	inline void LengthCtr();
	inline void VolEnv();
	inline void Sweep();

	//for TimerEvents;
	static const GB_BY ReadTable[0x20];
	static const GB_BY WaveTable[4][8];
	static const GB_BY NoiseDiv[8];
	static const GB_BY WaveVol[4];
	GB_BY Output[4];
	GB_BY Ptr[4];
	int outputL;
	int outputR;
	//outputs
	void Mix();
};