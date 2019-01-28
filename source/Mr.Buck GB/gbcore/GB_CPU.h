#pragma once
#include "GB.h"
#include "GB_MEMORY.h"
#define LOGg
#ifdef LOG
#include <iostream>
#include <cstdio>
#include <iomanip>
#endif // LOG



#define FLAG_NC 0
#define FLAG_NZ 3
#define FLAG_C 4
#define FLAG_Z 7
#define NONE 90
#define LD8 0
#define LD16 1
#define NOP 2
#define JR 4
#define INC8 5
#define INC16 6
#define DEC8 7
#define DEC16 8
#define RLCA 9
#define RRCA 10
#define RLA 11
#define RRA 12
#define DAA 13
#define CPL 14
#define SCF 15
#define CCF 16
#define LDa16SP 17
#define HALT 18
#define ADD 19
#define ADC 20
#define SUB 21
#define SBC 22
#define AND 23
#define XOR 24
#define OR 25
#define CP 26
#define RST 27
#define LDHa8A 28
#define LDHAa8 29
#define POP 30
#define JP 31
#define CALL 32
#define PUSH 33
#define DI 34
#define EI 35
#define RET 36
#define RET_NO_CC 37
#define ADDSP 38
#define LDHLSP 39
#define JPHL 40
#define LDHa16A 41
#define LDHAa16 42
#define RLC 43
#define RRC 44
#define RL 45
#define RR 46
#define SLA 47
#define SRA 48
#define SWAP 49
#define SRL 50
#define BIT 51
#define RES 52
#define SET 53
#define ADDHL 54
#define LD8_MEM 55
#define DELAY 56
#define RETI 60
#define LDa16SP_2 61
#define LDa16SP_3 62
#define LDa16SP_4 63
#define LDa16SP_5 64
#define ADDHL_2 65
#define ADDSP_2 66
#define INC16_2 67
#define DEC16_2 68
#define RET_2 69
#define JR_JUMP 70
#define JP_JUMP 71
#define CALL_JUMP 72
#define RET_JUMP 73
#define LDHLSP_2 74
#define RET_JUMP_2 75
#define RET_JUMP_3 76
#define CALL_JUMP_2 77
#define CALL_JUMP_3 78
#define RST_2 79
#define RST_3 80
#define RST_4 81
#define PUSH_2 82
#define PUSH_3 83
#define PUSH_4 84
#define POP_2 85
#define POP_3 86
#define LDA 87
#define LDSPHL 88
#define LDSPHL_2 89
#define ADDSP_DELAY 91
#define CALL_2 92
class Reg {
#define FLAG_ZERO 7
#define FLAG_NEGA 6//sub
#define FLAG_HACA 5
#define FLAG_CARY 4
#define A 0
#define F 1
#define B 2
#define C 3
#define D 4
#define E 5
#define H 6
#define L 7
#define TMPH 8
#define TMPL 9
#define SPH 10
#define SPL 11
#define PCH 12
#define PCL 13
#define AF 0
#define BC 2
#define DE 4
#define HL 6
#define TMP 8
#define SP 10
#define PC 12
#define DR 14
#define AR 16
#define ARH 15
#define ARL 16

private:
	GB_BY _REG8[15];
	GB_DB _REG16[2];
public:
	void DeadLoop() {
		while (1);
	}
	void Init() {
		for (int i = 0; i < 15; i++)_REG8[i] = 0;
		for (int i = 0; i < 2; i++)_REG16[i] = 0;
	};
	//no PC,SP here.
	GB_BY Get8(const GB_BY ad) {
		if (ad == F)return _REG8[F] & 0xF0;
		return _REG8[ad];
	}
	void Set8(const GB_BY ad, const GB_BY val) {
		_REG8[ad] = val;
		if (ad > 15)DeadLoop();
	}
	void Set16(const GB_BY ad, const GB_DB val) {
		_REG8[ad] = (val & 0xFF00) >> 8;
		_REG8[ad+1] = val & 0xFF;
		if (ad > 14)DeadLoop();
	}
	GB_DB Get16(const GB_BY ad) {
		return (_REG8[ad]<< 8) | _REG8[ad+1];
	}
	void Load8(const GB_BY dest, const GB_BY origin) {
		_REG8[dest] = _REG8[origin];
	}
	void Load16(const GB_BY dest, const GB_BY origin) {
		_REG8[dest] = _REG8[origin];
		_REG8[dest+1] = _REG8[origin+1];
	}

	//PC,SP part.
	void LoadPC(const GB_DB origin) {
		_REG16[0] = (_REG8[origin] << 8) | _REG8[origin + 1];
	}
	void LoadSP(const GB_DB origin) {
		_REG16[1] = (_REG8[origin] << 8) | _REG8[origin + 1];
	}
	void IncPC() {
		_REG16[0]++;
	}
	void DecSP() {
		if (_REG8[SPL] == 0x00)_REG8[SPH]--;
		_REG8[SPL]--;
	}
	void IncSP() {
		if (_REG8[SPL] == 0xFF)_REG8[SPH]++;
		_REG8[SPL]++;
	}
	void IncHL() {
		if (_REG8[L] == 0xFF)_REG8[H]++;
		_REG8[L]++;
	}
	void DecHL() {
		if (_REG8[L] == 0x00)_REG8[H]--;
		_REG8[L]--;
	}
	GB_DB GetPC() {
		return _REG16[0];
	}
	GB_DB GetSP() {
		return ((GB_DB)_REG8[SPH]<<8)|_REG8[SPL];
	}
	void SetPC(const GB_DB val) {
		_REG16[0] = val;
	}
	void SetSP(const GB_DB val) {
		_REG8[SPH] = (val&0xFF00)>>8;
		_REG8[SPL] = val & 0xFF;
	}
	void SetPCHigh(const GB_BY val) {
		_REG16[0] = _REG16[0] & 0xFF | ((GB_DB)val << 8);
	}
	void SetSPHigh(const GB_BY val) {
		_REG8[SPH] = val;
	}
	void SetPCLow(const GB_BY val) {
		_REG16[0] = _REG16[0] & 0xFF00 | val;
	}
	void SetSPLow(const GB_BY val) {
		_REG8[SPL] = val;
	}
	void SetFlag(const int Flag) {
		_REG8[F] |= 1 << Flag;
	}
	void ResetFlag(const int Flag) {
		_REG8[F] &= ~(1 << Flag);
	}
	GB_BY TestFlag(const int Flag) {//state=1->return true if true
		if (Flag == NONE)return true;
		if(Flag<4)
			return (_REG8[F] & (1 << (Flag+4))) != 0 ? 0 : 1;//test for 'NOT FLAG'
		else
			return (_REG8[F] & (1 << Flag)) != 0 ? 1 : 0;
	}
#ifdef LOG
	void output() {
		static int time = 0;
		using namespace std;
		cout.fill('0');
		
		cout << "AF:" << setw(4) << hex << (GB_DB)Get16(AF) << endl;
		cout << "BC:" << setw(4) << hex << (GB_DB)Get16(BC) << endl;
		cout << "DE:" << setw(4) << hex << (GB_DB)Get16(DE) << endl;
		cout << "HL:" << setw(4) << hex << (GB_DB)Get16(HL) << endl;
		cout << "SP:" << setw(4) << hex << (GB_DB)GetSP() << endl;
		cout << "PC:" << setw(4) << hex << (GB_DB)GetPC() << endl;
		cout << "-----------------------------------------------------------------------------" << endl;
		
		time++;
	}
#endif // LOG

	
};
class CPU:public Hardware
{
public:
	explicit CPU(Memory& memory) :_Memory(memory){
		BuildOps();
		optick = 0;
	};
	~CPU() {};
	void tick();
	void Init();
	void Send(GB_BY nothing) {};
private:
	int version;//0=DMG,1=GBP,2=CGB,3=SGB
	int optick;
	int cputick;
	Reg reg;
	int NextOp;//-1 means need to fetch.
	int CpuState;
	int IME;
	GB_BY IMEType;
	GB_BY IR;
	GB_DB _AR;
	GB_BY _DR;
	GB_DB _TMP;
	int reg1, reg2;
	int param;
	int SpeedState;
	GB_BY fetch();//change reg1,reg2.
	GB_BY fetchCB();
	int noHaltBug;
	void InterHandle(GB_BY);

	int dir;//memory read or write
	int MemReg;//which reg is involved
	//micro-ops
	
	void BuildOps();
	Memory & _Memory;
};
