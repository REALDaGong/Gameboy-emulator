#pragma once
#include "GB.h"
#include "GB_MEMORY.h"
#include "GB_GPU.h"
#define FLAG_ZERO 7
#define FLAG_NEGA 6//sub
#define FLAG_HACA 5
#define FLAG_CARY 4


class Z80 {
private:
		//CLOCK
	
	CLOCK_Val CLOCK;
	

	struct REG {
			GB_BY A, F;			//15...8 7...0
			GB_BY B, C;			//
			GB_BY D, E;			//
			GB_BY H, L;			//
			GB_DB SP;			//15.........0
			GB_DB PC;

			GB_BY FL;			//7.............0
								//Z N H C 0 0 0 0
			GB_BY IME;			//

	};
	REG _REG;
	void SetFlag(int cname, GB_BY val) {
		if (val == 0) {
			val = (GB_BY)1;
			val <<= cname;
			val ^= 0xFF;
			_REG.FL &= val;
				
		}
		else {
			val <<= cname;
			_REG.FL |= val;
		}
	}
	//void SetFlag(GB_DB Flag,GB_BY val);
	GB_BY GetFlag(int cname) {
		if (_REG.FL&(GB_BY)1 << cname)return (GB_BY)1; return (GB_BY)0;
	}
	bool isPause;
	void Pause() {
		if (!isPause)return;
	}
	bool isStop;
	void Stop() {
		if (!isStop)return;
	}
	void LDHL();
	
	void ADD(GB_BY REG);
	void ADC(GB_BY REG);
	void SUB(GB_BY REG);
	void SBC(GB_BY REG);
	void AND(GB_BY REG);
	void OR(GB_BY REG);
	void XOR(GB_BY REG);
	void CP(GB_BY REG);
	void INC(GB_BY &REG);
	void DEC(GB_BY &REG);
	void EXADD(GB_BY HREG, GB_BY LREG);
	
	void SWAP(GB_BY &REG);
	
	void RLC(GB_BY &REG);
	void RL(GB_BY &REG);
	void RRC(GB_BY &REG);
	void RR(GB_BY &REG);
	
	void SLA(GB_BY &REG);//MSB
	void SRA(GB_BY &REG);
	void SRL(GB_BY &REG);

	void BIT(GB_BY REG);
	
	void RST();
	void JP();
	void CALL();

	//void GPUStep();
	
	void Interrupt(GB_BY IMEtype);
	//CLOCK_Val GPU_CLOCK;
	Memory& _Memory;
	GPU& _GPU;
public:
	explicit Z80(Memory& memory, GPU& GPU) :_Memory(memory), _GPU(GPU) { Init(); };
	~Z80() {};
	
	void Step();
	
	void Init() {
		InitOpCodeList();
	}
	
	void InitOpCodeList();

	};
