#pragma once
#include "GB.h"
#include "GB_MEMORY.h"
#include "GB_GPU.h"
#include "GB_Timer.h"
#define FLAG_ZERO 7
#define FLAG_NEGA 6//sub
#define FLAG_HACA 5
#define FLAG_CARY 4


class Z80 {
private:

	struct REG {
			GB_BY A, F;			//15...8 7...0
			GB_BY B, C;			//
			GB_BY D, E;			//
			GB_BY H, L;			//
			GB_DB SP;			//15.........0
			GB_DB PC;

								//Z N H C 0 0 0 0
			GB_BY IME;			//

	};
	REG _REG;
	inline void SetFlag(int cname, GB_BY val) {
		if (val == 0) {
			_REG.F &= ((GB_BY)1<<cname)^0xFF;
		}
		else {
			_REG.F |= ((GB_BY)1<<cname);
		}
	}
	
	inline GB_BY GetFlag(int cname) {
		if (_REG.F&(GB_BY)1 << cname)return (GB_BY)1; return (GB_BY)0;
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

	void BIT(GB_BY REG, GB_BY No);
	
	void RST();
	void JP();
	void CALL();

	void Interrupt(GB_BY IMEtype);
	
	Memory& _Memory;
	GPU& _GPU;
	Timer& _Timer;
public:
	explicit Z80(Memory& memory, GPU& GPU, Timer& Timer) :_Memory(memory), _GPU(GPU), _Timer(Timer){ Init(); };
	~Z80() {};
	
	void Step();
	
	void Init() {
		InitOpCodeList();
		_REG.PC = 0x100;
		_REG.A = 0x01;
		_REG.F = 0xB0;
		_REG.C = 0x13;
		_REG.E = 0xD8;
		_REG.H = 0x01;
		_REG.L = 0x4D;
		_REG.SP = 0xFFFE;
		_REG.IME = 1;
	}
	
	void InitOpCodeList();

	};
