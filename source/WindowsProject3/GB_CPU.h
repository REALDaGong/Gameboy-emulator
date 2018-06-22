#pragma once

#include "GB.h"
#include "GB_MEMORY.h"
#include "GB_GPU.h"
#include "GB_Timer.h"

#define FLAG_ZERO 7
#define FLAG_NEGA 6//sub
#define FLAG_HACA 5
#define FLAG_CARY 4

/*
cpu.
do cpu things.
details in function Step.
*/

class Z80 {
public:
	explicit Z80(Memory &memory, GPU &GPU, Timer &Timer) : _Memory(memory), _GPU(GPU), _Timer(Timer) {
		Init();
	};

	~Z80() {};

	void Step();

	void Init() {
		InitOpCodeList();
		isPause = 0;
		isStop = 0;
		Op = 0;
		delta = 0;
		_REG.A =
		_REG.F =
		_REG.B =
		_REG.C =
		_REG.D =
		_REG.E =
		_REG.H =
		_REG.L = 0;
		_REG.PC = _REG.SP = 0;
		_REG.IME = 0;
	}

	void InitOpCodeList();

private:

	struct REG {
		GB_BY A, F;            //15...8 7...0 F's low 4 bit must always be 0
		GB_BY B, C;            //
		GB_BY D, E;            //
		GB_BY H, L;            //
		GB_DB SP;            //15.........0
		GB_DB PC;

		//Z N H C 0 0 0 0
		GB_BY IME;            //

	};
	REG _REG;
	GB_BY Op;                    //actually is IR
	GB_BY delta;                    //how many clks last Op consumed
	inline void SetFlag(int cname, GB_BY val) {
		if (val == 0) {
			_REG.F &= (static_cast<GB_BY>(1) << cname) ^ 0xFF;
		}
		else {
			_REG.F |= (static_cast<GB_BY>(1) << cname);
		}
	}

	inline GB_BY GetFlag(int cname) {
		if (_REG.F & static_cast<GB_BY>(1) << cname)return (GB_BY) 1;
		return (GB_BY) 0;
	}

	bool isPause, isStop;     //for HALT,STOP Ops

	void LDHL();

	void ADD(GB_BY REG);

	void ADDHL(GB_BY REGH, GB_BY REGL);

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

	void SLA(GB_BY &REG);

	void SRA(GB_BY &REG);

	void SRL(GB_BY &REG);

	void BIT(GB_BY REG, GB_BY No);

	void RST();

	void JP();

	void CALL();

	void Interrupt(GB_BY IMEtype);

	Memory &_Memory;
	GPU &_GPU;
	Timer &_Timer;

};
