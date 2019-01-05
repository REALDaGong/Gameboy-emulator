#include "GB_APU.h"
const GB_BY APU::ReadTable[0x20] = {
	0x80,0x3F,0x00,0xFF,0xBF,
	0xFF,0x3F,0x00,0xFF,0xBF,
	0x7F,0xFF,0x9F,0xFF,0xBF,
	0xFF,0xFF,0x00,0x00,0xBF,
	0x00,0x00,0x70,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
const GB_BY APU::WaveTable[4][8] = {
	{0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,1,1},
	{0,1,1,1,1,1,1,0}
};
const GB_BY APU::NoiseDiv[8] = {8,16,32,48,64,80,96,112};
const GB_BY APU::WaveVol[4] = { 4,0,1,2 };
void APU::APUWrite(GB_DB ad, GB_BY val) {
	
	if (ad < 0xFF30) {
		if (Power == 0) {
			switch (ad) {//On DMG,the LengthCounter can still working while power off,but not clocked.
			case NR52://go forward.
				break;
			case NR11:
				LengthCounter[0] = 64 - (val & 0x3F);
				return;
			case NR21:
				LengthCounter[1] = 64 - (val & 0x3F);
				return;
			case NR31:
				LengthCounter[2] = 256 - val;
				return;
			case NR41:
				LengthCounter[3] = 64 - (val & 0x3F);
			default:
				return;
			}
		}
		switch (ad) {
		case NR10:
			//Reg[NR10 - 0xFF10] = val;
			SweepPeriod = (val & 0x70) >> 4;
			Negate = val & 0x8;
			SweepShift = val & 0x7;
			if (Negated &&(Negate == 0)) {
				Trigger[0] = 0;
				SweepEnable = 0;
				
			}
			break;
		case NR11:
			//Reg[NR11 - 0xFF10] = val;
			Duty[0] = (val&0xC0)>>6;
			LengthLoad[0] = val & 0x3F;
			LengthCounter[0] = 64- LengthLoad[0];
			break;
		case NR12:
			//Reg[NR12 - 0xFF10] = val;
			VolSet[0] = val >> 4;
			VolEnvMode[0] = (val & 0x8) >> 3;
			
			VolEnvPeriod[0] = (val & 0x7);
			if (VolSet[0] == 0 && VolEnvMode[0] == 0) {
				DACEnable[0] = 0;//disabled DAC should disable the channel immadiately.
				Trigger[0] = 0;
			}
			else DACEnable[0] = 1;
			break;
		case NR13:
			//Reg[NR13 - 0xFF10] = val;
			Freq[0] = val|(Freq[0]&0x700);
			break;
		case NR14:
			if ((val & 0x40) >> 6) {
				Trigger[0] = 1;//enable length also enable channel.disable length cant disable channel.
			}
			if (LengthEnable[0] == 0 && (val & 0x40) >> 6 && (step&1)==0) {
				if (LengthCounter[0] != 0) {
					LengthCounter[0]--;
					if (LengthCounter[0] == 0 && (val & 0x80) == 0) {
						Trigger[0] = 0;
						
					}
				}
			}
			//Reg[NR14 - 0xFF10] = val;
			LengthEnable[0] = (val & 0x40) >> 6;//once be set,it will be used.
			
			Freq[0] = (Freq[0] & 0xFF) | ((val & 0x7) << 8);
			if ((val & 0x80) >> 7) {

				Trigger[0] = 1;
				if (LengthCounter[0] == 0) {
					if ((step & 1) == 0 && LengthEnable[0]) {
						LengthCounter[0] = 63;
					}
					else {
						LengthCounter[0] = 64;
					}
				}
				_Timer[0].limit = (2048-(Freq[0]&0x7FF))<<2;
				_Timer[0].current = 0;//in fact,the last 2 bits are consistent.
				EnvTimer[0] = VolEnvPeriod[0];
				EnvStop[0] = 0;

				VolOut[0] = VolSet[0];
				Negated = 0;
				Shadow = Freq[0];
				if (SweepPeriod != 0) {
					SweepTimer = SweepPeriod;
				}
				else {
					SweepTimer = 8;
				}
				if (SweepShift == 0 && SweepPeriod == 0) {
					SweepEnable = 0;//wont stop Channel,just stop sweep.
				}
				else {
					SweepEnable = 1;//needed?
					if (SweepShift != 0) {
						if (Negate) {
							Negated = 1;
							if (Shadow - (Shadow >> SweepShift) > 2047)Trigger[0] = 0;
						}
						else {
							if (Shadow + (Shadow >> SweepShift) > 2047)Trigger[0] = 0;
						}
					}
				}
				Ptr[0] = 7;
				//once overflow,disable the channel.
				if (DACEnable[0] == 0)Trigger[0] = 0;// disabled DAC allow data updated,but still disables the channel.
			}
			
			break;
		case NR21:
			//Reg[NR21 - 0xFF10] = val;
			Duty[1] = (val & 0xC0) >> 6;
			LengthLoad[1] = val & 0x3F;
			LengthCounter[1] = 64-LengthLoad[1];
			break;
		case NR22:
			//Reg[NR22 - 0xFF10] = val;
			VolSet[1] = val >> 4;
			VolEnvMode[1] = (val & 0x8) >> 3;
			VolEnvPeriod[1] = (val & 0x7);
			if (VolSet[1] == 0 && VolEnvMode[1] == 0) {
				DACEnable[1] = 0;
				Trigger[1] = 0;
			}
			else DACEnable[1] = 1;
			break;
		case NR23:
			//Reg[NR23 - 0xFF10] = val;
			Freq[1] = val | (Freq[1] & 0x700);
			break;
		case NR24:
			//Reg[NR24 - 0xFF10] = val;
			if ((val & 0x40) >> 6) {
				Trigger[1] = 1;
			}
			if (LengthEnable[1] == 0 && (val & 0x40) >> 6 && (step & 1) == 0) {
				if (LengthCounter[1] != 0) {
					LengthCounter[1]--;
					if (LengthCounter[1] == 0 && (val & 0x80)==0) {
						Trigger[1] = 0;
					}
				}
			}
			LengthEnable[1] = (val & 0x40) >> 6;
			
			Freq[1] = (Freq[1] & 0xFF) | ((val & 0x7) << 8);
			
			if ((val & 0x80) >> 7) {
				Trigger[1] = 1;
				if (LengthCounter[1] == 0) {
					if ((step & 1) == 0 && LengthEnable[1]) {
						LengthCounter[1] = 63;
					}
					else {
						LengthCounter[1] = 64;
					}
				}
				_Timer[1].limit = (2048 - (Freq[1] & 0x7FF))<<2;
				_Timer[1].current = 0;
				EnvTimer[1] = VolEnvPeriod[1];
				EnvStop[1] = 0;

				VolOut[1] = VolSet[1];
				Ptr[1] = 7;
				if (DACEnable[1] == 0)Trigger[1] = 0;
			}
			
			break;
		case NR30:
			//Reg[NR30 - 0xFF10] = val;
			if (val & 0x80)DACEnable[2] = 1;//Wave channel DAC is not controlled by volume set.
			else {
				DACEnable[2] = 0;
				Trigger[2] = 0;
			}
			break;
		case NR31:
			//Reg[NR31 - 0xFF10] = val;
			LengthLoad[2] = val;
			LengthCounter[2] = 256- val;
			break;
		case NR32:
			//Reg[NR32 - 0xFF10] = val;
			VolSet[2] = (val & 0x60) >> 5;
			break;
		case NR33:
			//Reg[NR33 - 0xFF10] = val;
			Freq[2] = val | (Freq[2] & 0x700);
			break;
		case NR34:
			if ((val & 0x40) >> 6) {
				Trigger[2] = 1;
			}
			if (LengthEnable[2] == 0 && (val & 0x40) >> 6 && (step & 1) == 0) {
				if (LengthCounter[2] != 0) {
					LengthCounter[2]--;
					if (LengthCounter[2] == 0 && (val & 0x80) == 0) {
						Trigger[2] = 0;
					}
				}
			}
			//Reg[NR34 - 0xFF10] = val;
			LengthEnable[2] = (val & 0x40) >> 6;
			Freq[2] = (Freq[2] & 0xFF) | ((val & 0x7) << 8);
			
			if ((val & 0x80) >> 7) {
				Trigger[2] = 1;
				if (LengthCounter[2] == 0) {
					if ((step & 1) == 0 && LengthEnable[2]) {
						LengthCounter[2] = 255;
					}
					else {
						LengthCounter[2] = 256;
					}
				}
				_Timer[2].limit = (2048 - (Freq[2] & 0x7FF))<<1;
				_Timer[2].current = 0;
				VolOut[2] = VolSet[2];

				Ptr[2] = PtrHead;
				Sample.empty();//?
				if (DACEnable[2] == 0)Trigger[2] = 0;
			}
			
			break;
		case NR41:
			//Reg[NR41 - 0xFF10] = val;
			LengthLoad[3] = val & 0x3F;
			LengthCounter[3] = 64- LengthLoad[3];
			break;
		case NR42:
			//Reg[NR42 - 0xFF10] = val;
			VolSet[3] = val >> 4;
			VolEnvMode[2] = (val & 0x8) >> 3;
			VolEnvPeriod[2] = (val & 0x7);
			if (VolSet[3] == 0 && VolEnvMode[2] == 0) {
				DACEnable[3] = 0;
				Trigger[3] = 0;
			}
			else DACEnable[3] = 1;
			break;
		case NR43:
			//Reg[NR43 - 0xFF10] = val;
			DivPtr = val & 0x7;
			WidthMode = val & 0x8;
			Shift = val & 0xF0;
			Freq[3] = NoiseDiv[DivPtr] << (Shift >> 4);
			break;
		case NR44:
			if ((val & 0x40) >> 6) {
				Trigger[3] = 1;
			}
			if (LengthEnable[3] == 0 && (val & 0x40) >> 6 && (step & 1) == 0) {
				if (LengthCounter[3] != 0) {
					LengthCounter[3]--;
					if (LengthCounter[3] == 0 && (val & 0x80) == 0) {
						Trigger[3] = 0;
					}
				}
			}
			//Reg[NR44 - 0xFF10] = val;
			LengthEnable[3] = (val & 0x40) >> 6;
			
			if ((val & 0x80) >> 7) {
				Trigger[3] = 1;
				if (LengthCounter[3] == 0) {
					if ((step & 1) == 0 && LengthEnable[3]) {
						LengthCounter[3] = 63;
					}
					else {
						LengthCounter[3] = 64;
					}
				}
				_Timer[3].limit = 2048 - (Freq[1] & 0x7FF);
				_Timer[3].current = 0;
				EnvTimer[2] = VolEnvPeriod[2];
				EnvStop[2] = 0;
				LFSR = 0xFF;
				if (WidthMode)WidthModeOn = 1;
				else WidthModeOn = 0;
				VolOut[3] = VolSet[3];
				Ptr[3] = 0;
				if (DACEnable[3] == 0)Trigger[3] = 0;
			}
			break;
		case NR50://for cartiage,in future.
			//Reg[NR50 - 0xFF10] = val;
			LVin = (val & 0xF0) >> 4;
			RVin = val & 0xF;
			break;
		case NR51:
			//Reg[NR51 - 0xFF10] = val;
			Lopen = (val & 0xF0)>>4;
			Ropen = val & 0xF;
			break;
		case NR52:
			//Reg[NR52 - 0xFF10] = val;
			if (val & 0x80) {
				if (Power == 0) {
					Power = 0x80;
					PowerON();
				}
			}
			else {
				if (Power == 0x80) {
					Power = 0;
					PowerOFF();
				}
			}
			break;
		default:
			Reg[ad - offset] = val;
		}
		
		
		
	}/*
	else {
		 PatternTable[ad - 0xFF30]=val;
	}*/
	else if (Trigger[2] == 0) {
		if (ad == 0xFF30)PtrHead = Ptr[2];
		GB_DB Loc = ((ad - 0xFF30)<<1) + PtrHead;
		if (Loc >= 32)Loc -= 32;
		PatternTable[Loc] = val>>4;
		PatternTable[Loc + 1] = val&0xF;
		
	}
}
GB_BY APU::APURead(GB_DB ad) {
	
	if (ad < 0xFF30) {
		switch (ad) {
		case NR10:
		{
			//GB_BY valOri = Reg[NR10 - 0xFF10] | 0x80;;
			//GB_BY val2= (SweepPeriod << 4) | Negate | SweepShift | 0x80;
			//if (valOri != val2) {
			//	int a=0;
			//	a++;//Dont go here.
			//}
			//return Reg[NR10 - 0xFF10] | 0x80;
			return (SweepPeriod << 4) | Negate | SweepShift | 0x80;
		}
			break;
		case NR11:
			//return Reg[NR11 - 0xFF10] | 0x3F;
			return (Duty[0] << 6) | 0x3F;
			break;
		case NR12:
			//return Reg[NR12 - 0xFF10];
			return (VolSet[0] << 4) | (VolEnvMode[0] << 3) | VolEnvPeriod[0];
			break;
		case NR14:
			//return Reg[NR14 - 0xFF10] | 0xBF;
			return (LengthEnable[0] << 6) | 0xBF;
			break;
		case NR21:
			//return Reg[NR21 - 0xFF10] | 0x3F;
			return (Duty[1] << 6) | 0x3F;
			break;
		case NR22:
			//return Reg[NR22 - 0xFF10];
			return (VolSet[1] << 4) | (VolEnvMode[1] << 3) | VolEnvPeriod[1];
			break;
		case NR24:
			//return Reg[NR24 - 0xFF10] | 0xBF;
			return (LengthEnable[1] << 6) | 0xBF;
			break;
		case NR30:
			//return Reg[NR30 - 0xFF10] | 0x7F;
			return (DACEnable[2] << 7) | 0x7F;
			break;
		case NR32:
			//return Reg[NR32 - 0xFF10] | 0x9F;
			return (VolSet[2] << 5) | 0x9F;
			break;
		case NR34:
			//return Reg[NR34 - 0xFF10] | 0xBF;
			return (LengthEnable[2] << 6) | 0xBF;
			break;
		case NR42:
			//return Reg[NR42 - 0xFF10];
			return (VolSet[3] << 4) | (VolEnvMode[2] << 3) | VolEnvPeriod[2];
			break;
		case NR43:
			//return Reg[NR43 - 0xFF10];
			return Shift | WidthMode | DivPtr;
			break;
		case NR44:
			//return Reg[NR44 - 0xFF10]|0xBF;
			return (LengthEnable[3] << 6) | 0xBF;
			break;
		case NR50:
			//return Reg[NR50 - 0xFF10];
			return (LVin << 4) | RVin;
			break;
		case NR51:
			//return Reg[NR51 - 0xFF10];
			return  (Lopen << 4) | Ropen;
			break;
		case NR52:
			//return Reg[NR52 - 0xFF10] | 0x70;
			return Power | Trigger[0] | (Trigger[1] << 1) | (Trigger[2] << 2) | (Trigger[3] << 3)|0x70;
			break;
		default:
			return 0xFF;
		}
	}/*
	else {
		return PatternTable[ad - 0xFF30];
	}*/
	
	else if (Trigger[2] == 0) {
		GB_DB Loc = PtrHead + ((ad - 0xFF30)<<1);
		if (Loc >= 32)Loc -= 32;
		return (PatternTable[Loc]<<4)|PatternTable[Loc+1];
	}
	else
		return (PatternTable[Ptr[2]]<<4)|PatternTable[Ptr[2]+1];
		
}
void APU::Init() {
	for (int i = 0; i < 4; i++) {
		_Timer[i].limit = SYS / 2048;
		_Timer[i].current = 0;
		DACEnable[i] = 0;
	}
	//frame sequencer
	
	step = 0;//or 7?
	Ptr[0] = Ptr[1] = 7;
	Ptr[2] = Ptr[3] = 0;
	_Timer[4].current = 0;
	_Timer[4].limit = SYS / 512;
	Power = 0x80;
}
void APU::PowerON() {
	Ptr[0] = Ptr[1] = 7;
	Ptr[2] = Ptr[3] = 0;
	step = 7;
	for (int i = 0; i < 16; i++) {
		PatternTable[i] = 0;
	}
}
void APU::PowerOFF() {
	//all set to zero.
	//for length counters:
	// On CGB, length counters are reset when powered up.
	// On DMG, they are unaffected, and not clocked.
	for (int i = 0; i < 20; i++) {
		Reg[i] = 0;
	}
	for (int i = 0; i < 4; i++) {
		LengthLoad[i]=0;
		//for DMG,comment it.
		//LengthCounter[i]=0;
		Trigger[i]=0;
		LengthEnable[i]=0;
		VolSet[i]=0;
		VolOut[i]=0;
		Freq[i]=0;
		DACEnable[i]=0;
	}
	for (int i = 0; i < 3; i++) {
		VolEnvPeriod[i]=0;
		EnvTimer[i]=0;
		EnvStop[i]=0;
		VolEnvMode[i]=0;
	}
	Duty[0] = 0;
	Duty[1] = 0;

	SweepPeriod=0;
	SweepTimer=0;
	Negate=0;
	SweepShift=0;
	SweepEnable=0;
	Shadow=0;
	
	WidthMode=0;
	WidthModeOn=0;//?
	LFSR=0;
	Shift=0;
	DivPtr=0;
	Lopen=Ropen=0;
	LVin=RVin=0;
	outputL=0;
	outputR=0;
	PtrHead=0;
}
void APU::SendClock(GB_BY delta) {
	if (DACEnable[0] == 1) {//well,i think it doesn't matter,but it will save some cpu cycles...
		_Timer[0].current += delta;
		if (_Timer[0].current >= _Timer[0].limit) {
			_Timer[0].current -= _Timer[0].limit;
			Square1();
		}
	}
	if (DACEnable[1] == 1) {
		_Timer[1].current += delta;
		if (_Timer[1].current >= _Timer[1].limit) {
			_Timer[1].current -= _Timer[1].limit;
			Square2();
		}
	}
	if (DACEnable[2] == 1) {
		_Timer[2].current += delta;
		if (_Timer[2].current >= _Timer[2].limit) {
			_Timer[2].current -= _Timer[2].limit;
			Wave();
		}
	}
	if (DACEnable[3] == 1) {
		_Timer[3].current += delta;
		if (_Timer[3].current >= _Timer[3].limit) {
			_Timer[3].current -= _Timer[3].limit;
			Noise();
		}
	}
	_Timer[4].current += delta;
	if (_Timer[4].current >= _Timer[4].limit) {
		_Timer[4].current -= _Timer[4].limit;
		Sequence();
	}
}
void APU::Sequence() {
	step++;
	if (step == 8) {
		step = 0;
	}
	if (Power) {
		if ((step & 0x1)==0) {
			if(Power)//DMG.
			LengthCtr();
		}
		if (step == 7) {
			VolEnv();
		}
		if (step == 2 || step == 6) {
			if(SweepTimer!=0)
			SweepTimer--;
			if (SweepTimer == 0) {
				if (SweepPeriod != 0) {
					SweepTimer = SweepPeriod;
				}
				else {
					SweepTimer = 8;//0 will be treated as 8,same as envlope timers.
				}
				if (SweepEnable &&SweepPeriod!=0)
					Sweep();
			}
			
		}
	}
}
inline void APU::LengthCtr() {//disabled channel should still clock length.
	
	for (int i = 0; i < 4; i++) {
		if (LengthEnable[i]) {
			if (LengthCounter[i] != 0)
				LengthCounter[i]--;
			if (LengthCounter[i] == 0) {
				Trigger[i] = 0;
			}
		}
	}
	
}
inline void APU::VolEnv() {
	if (EnvStop[0] == 0 && VolEnvPeriod[0] != 0) {
		EnvTimer[0]--;
		
		if (EnvTimer[0] == 0) {
			if (VolEnvPeriod[0] != 0) {
				EnvTimer[0] = VolEnvPeriod[0];
			}
			else {
				EnvTimer[0] = 8;
			}
			if (VolEnvMode[0] == 0) {
				if (VolOut[0] != 0)
					VolOut[0]--;
				else {
					EnvStop[0] = 1;
					DACEnable[0] = 0;
				}
			}
			else {
				if (VolOut[0] != 15)
					VolOut[0]++;
				else
					EnvStop[0] = 1;
			}
		}

	}
	if (EnvStop[1] == 0&& VolEnvPeriod[1] != 0) {
		EnvTimer[1]--;
		
		if (EnvTimer[1] == 0) {
			if (VolEnvPeriod[1] != 0) {
				EnvTimer[1] = VolEnvPeriod[1];
			}
			else {
				EnvTimer[1] = 8;
			}
			if (VolEnvMode[1] == 0) {
				if (VolOut[1] != 0)
					VolOut[1]--;
				else {
					EnvStop[1] = 1;
					DACEnable[1] = 0;
				}
			}
			else {
				if (VolOut[1] != 15)
					VolOut[1]++;
				else
					EnvStop[1] = 1;
			}
		}

	}
	if (EnvStop[2] == 0 && VolEnvPeriod[2] != 0) {
		EnvTimer[2]--;
		if (EnvTimer[2] == 0) {
			if (VolEnvPeriod[2] != 0) {
				EnvTimer[2] = VolEnvPeriod[2];
			}
			else {
				EnvTimer[2] = 8;
			}
			if (VolEnvMode[2] == 0) {
				if (VolOut[3] != 0)
					VolOut[3]--;
				else {
					EnvStop[2] = 1;
					DACEnable[3] = 0;
				}
			}
			else {
				if (VolOut[3] != 15)
					VolOut[3]++;
				else
					EnvStop[2] = 1;
			}
		}

	}
}
inline void APU::Sweep() {
	
		
		
	if (Negate) {
		GB_DB newFreq = Shadow - (Shadow >> SweepShift);
		Negated = 1;
		if (newFreq < 2048) {
			if (SweepShift != 0) {
				Freq[0] = newFreq;
				Shadow = Freq[0];
				if (Shadow - (Shadow >> SweepShift) > 2047) {
					SweepEnable = 0;
					Trigger[0] = 0;
				}
			}
		}else {
			SweepEnable = 0;
			Trigger[0] = 0;
		}
		
	}
	else {
		GB_DB newFreq = Shadow + (Shadow >> SweepShift);
		if (newFreq < 2048) {
			if (SweepShift != 0) {
				Freq[0] = newFreq;
				Shadow = Freq[0];
				if (Shadow + (Shadow >> SweepShift) > 2047) {
					SweepEnable = 0;
					Trigger[0] = 0;
				}
			}
		}else {
			SweepEnable = 0;
			Trigger[0] = 0;
		}
		
		
	}
	
		
		
	
}
//its a little too bruce to even simulate the waves,a better way
//is just sending commands directly to soundhardware,but its more
//diffcult actually.
inline void APU::Square1() {
	Ptr[0]++;
	if (Ptr[0] == 8)Ptr[0] = 0;
	Output[0] = WaveTable[Duty[0]][Ptr[0]]*VolOut[0];
}
inline void APU::Square2() {
	Ptr[1]++;
	if (Ptr[1] == 8)Ptr[1] = 0;
	Output[1] = WaveTable[Duty[1]][Ptr[1]]*VolOut[1];
}
inline void APU::Wave() {
	//yes,it is.
	Output[2] = PatternTable[Ptr[2]] >> WaveVol[VolOut[2]];
	Ptr[2]++;
	if (Ptr[2] == 32)Ptr[2] = 0;
}
inline void APU::Noise() {
	if (Shift > 13) {
		Output[3] = ((LFSR & 0x1) == 1) ? 0 : VolOut[3];
	}
	else {
		GB_DB re = ((LFSR & 0x1) ^ ((LFSR >> 1) & 0x1));
		if (WidthModeOn) {
			LFSR >>= 1;
			LFSR |= re << 6;
		}
		else {
			LFSR >>= 1;
			LFSR |= re << 15;
		}
		Output[3] = ((LFSR & 0x1) == 1) ? 0 : VolOut[3];
	}
}