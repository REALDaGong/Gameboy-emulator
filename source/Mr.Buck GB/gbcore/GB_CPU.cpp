
/*
contains:
all opcodes
cpu main loop
interrupt handler
*/

#include"GB_CPU.h"

static array<function<int()>, 0x100 * sizeof(int)> OpCode;
static array<function<int()>, 0x100 * sizeof(int)> CBOpCode;


void Z80::Step() {
	if (_REG.PC == 0x7846) {
		int a = 2;
		//debug breakpoint here.
	}
	if (!isPause) {
		Op = _Memory.MemoryRead(_REG.PC++);
		delta = OpCode[Op]();
		_Timer.TimerInc(delta);
		_GPU.AddClock(delta);
		_Memory.SendClock(delta);
	}
	else {
		_Timer.TimerInc(4);
		_GPU.AddClock(4);
		_Memory.SendClock(4);
	}

	if (_Memory.MemoryRead(IE)&_Memory.MemoryRead(IF)) {
		isPause = 0;
		if (_REG.IME) {
			GB_BY IMEType = _Memory.MemoryRead(IE) & _Memory.MemoryRead(IF);
			Interrupt(IMEType);
			_GPU.AddClock(20);
			_Memory.SendClock(20);
			//it seems that a INTR transfer procedure will spend 20clks
			//and it will stop the timer for a while
			//i dont quite understand.
		}
	}
		
	

}
void Z80::Interrupt(GB_BY IMEtype) {
	
	_REG.IME = 0;
	if (IMEtype & 0x01) {
		_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x1));
		RST();
		_REG.PC = 0x40;
	}//V-BLANK
	else
		if (IMEtype & 0x02) {
			_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x2));
			RST();
			_REG.PC = 0x48;
		}//LCDC (see STAT)
		else
			if (IMEtype & 0x04) {
				_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x4));
				RST();
				_REG.PC = 0x50;
			}//timer overflow
			else
				if (IMEtype & 0x08) {
					_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x8));
					RST();
					_REG.PC = 0x58;
				}//serial io transfer complete
				else
					if (IMEtype & 0x10) {
						_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x10));
						RST();
						_REG.PC = 0x60;
					}//transition from high to low of pin number p10-p13
	
}
void Z80::InitOpCodeList() {
	for (int i = 0; i < 0x100; i++) {
		OpCode[i] = [&]()->int {return 0; };
		CBOpCode[i] = [&]()->int {return 0; };
	}
	//LD nn,n put n into reg nn

	OpCode[0x06] = [&]()->int {_REG.B = _Memory.MemoryRead(_REG.PC++); return 8; };
	OpCode[0x0E] = [&]()->int {_REG.C = _Memory.MemoryRead(_REG.PC++); return 8; };
	OpCode[0x16] = [&]()->int {_REG.D = _Memory.MemoryRead(_REG.PC++); return 8; };
	OpCode[0x1E] = [&]()->int {_REG.E = _Memory.MemoryRead(_REG.PC++); return 8; };
	OpCode[0x26] = [&]()->int {_REG.H = _Memory.MemoryRead(_REG.PC++); return 8; };
	OpCode[0x2E] = [&]()->int {_REG.L = _Memory.MemoryRead(_REG.PC++); return 8; };

	//LD r1,r2
	//p66
	OpCode[0x7F] = [&]()->int {_REG.A = _REG.A; return 4; };
	OpCode[0x78] = [&]()->int {_REG.A = _REG.B; return 4; };
	OpCode[0x79] = [&]()->int {_REG.A = _REG.C; return 4; };
	OpCode[0x7A] = [&]()->int {_REG.A = _REG.D; return 4; };
	OpCode[0x7B] = [&]()->int {_REG.A = _REG.E; return 4; };
	OpCode[0x7C] = [&]()->int {_REG.A = _REG.H; return 4; };
	OpCode[0x7D] = [&]()->int {_REG.A = _REG.L; return 4; };
	OpCode[0x7E] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.H << 8 | _REG.L); return 8; };
	OpCode[0x40] = [&]()->int {_REG.B = _REG.B; return 4; };
	OpCode[0x41] = [&]()->int {_REG.B = _REG.C; return 4; };
	OpCode[0x42] = [&]()->int {_REG.B = _REG.D; return 4; };
	OpCode[0x43] = [&]()->int {_REG.B = _REG.E; return 4; };
	OpCode[0x44] = [&]()->int {_REG.B = _REG.H; return 4; };
	OpCode[0x45] = [&]()->int {_REG.B = _REG.L; return 4; };
	OpCode[0x46] = [&]()->int {_REG.B = _Memory.MemoryRead(_REG.H << 8 | _REG.L); return 8; };
	OpCode[0x48] = [&]()->int {_REG.C = _REG.B; return 4; };
	OpCode[0x49] = [&]()->int {_REG.C = _REG.C; return 4; };
	OpCode[0x4A] = [&]()->int {_REG.C = _REG.D; return 4; };
	OpCode[0x4B] = [&]()->int {_REG.C = _REG.E; return 4; };
	OpCode[0x4C] = [&]()->int {_REG.C = _REG.H; return 4; };
	OpCode[0x4D] = [&]()->int {_REG.C = _REG.L; return 4; };
	OpCode[0x4E] = [&]()->int {_REG.C = _Memory.MemoryRead(_REG.H << 8 | _REG.L); return 8; };
	OpCode[0x50] = [&]()->int {_REG.D = _REG.B; return 4; };
	OpCode[0x51] = [&]()->int {_REG.D = _REG.C; return 4; };
	OpCode[0x52] = [&]()->int {_REG.D = _REG.D; return 4; };
	OpCode[0x53] = [&]()->int {_REG.D = _REG.E; return 4; };
	OpCode[0x54] = [&]()->int {_REG.D = _REG.H; return 4; };
	OpCode[0x55] = [&]()->int {_REG.D = _REG.L; return 4; };
	OpCode[0x56] = [&]()->int {_REG.D = _Memory.MemoryRead(_REG.H << 8 | _REG.L); return 8; };
	OpCode[0x58] = [&]()->int {_REG.E = _REG.B; return 4; };
	OpCode[0x59] = [&]()->int {_REG.E = _REG.C; return 4; };
	OpCode[0x5A] = [&]()->int {_REG.E = _REG.D; return 4; };
	OpCode[0x5B] = [&]()->int {_REG.E = _REG.E; return 4; };
	OpCode[0x5C] = [&]()->int {_REG.E = _REG.H; return 4; };
	OpCode[0x5D] = [&]()->int {_REG.E = _REG.L; return 4; };
	OpCode[0x5E] = [&]()->int {_REG.E = _Memory.MemoryRead(_REG.H << 8 | _REG.L); return 8; };
	OpCode[0x60] = [&]()->int {_REG.H = _REG.B; return 4; };
	OpCode[0x61] = [&]()->int {_REG.H = _REG.C; return 4; };
	OpCode[0x62] = [&]()->int {_REG.H = _REG.D; return 4; };
	OpCode[0x63] = [&]()->int {_REG.H = _REG.E; return 4; };
	OpCode[0x64] = [&]()->int {_REG.H = _REG.H; return 4; };
	OpCode[0x65] = [&]()->int {_REG.H = _REG.L; return 4; };
	OpCode[0x66] = [&]()->int {_REG.H = _Memory.MemoryRead(_REG.H << 8 | _REG.L); return 8; };
	OpCode[0x68] = [&]()->int {_REG.L = _REG.B; return 4; };
	OpCode[0x69] = [&]()->int {_REG.L = _REG.C; return 4; };
	OpCode[0x6A] = [&]()->int {_REG.L = _REG.D; return 4; };
	OpCode[0x6B] = [&]()->int {_REG.L = _REG.E; return 4; };
	OpCode[0x6C] = [&]()->int {_REG.L = _REG.H; return 4; };
	OpCode[0x6D] = [&]()->int {_REG.L = _REG.L; return 4; };
	OpCode[0x6E] = [&]()->int {_REG.L = _Memory.MemoryRead(_REG.H << 8 | _REG.L); return 8; };
	OpCode[0x70] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.B); return 8; };
	OpCode[0x71] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.C); return 8; };
	OpCode[0x72] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.D); return 8; };
	OpCode[0x73] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.E); return 8; };
	OpCode[0x74] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.H); return 8; };
	OpCode[0x75] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.L); return 8; };
	OpCode[0x36] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _Memory.MemoryRead(_REG.PC++)); return 12; };

	//LD A,n
	//p67


	OpCode[0x0A] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.B << 8 | _REG.C); return 8; };
	OpCode[0x1A] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.D << 8 | _REG.E); return 8; };
	//0x7E
	OpCode[0xFA] = [&]()->int {_REG.A = _Memory.MemoryRead(_Memory.MemoryRead(_REG.PC++)|_Memory.MemoryRead(_REG.PC++)<<8); return 16; };
	OpCode[0x3E] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.PC++); return 8; };

	//LD n,A
	//p69
	OpCode[0x47] = [&]()->int {_REG.B = _REG.A; return 4; };
	OpCode[0x4F] = [&]()->int {_REG.C = _REG.A; return 4; };
	OpCode[0x57] = [&]()->int {_REG.D = _REG.A; return 4; };
	OpCode[0x5F] = [&]()->int {_REG.E = _REG.A; return 4; };
	OpCode[0x67] = [&]()->int {_REG.H = _REG.A; return 4; };
	OpCode[0x6F] = [&]()->int {_REG.L = _REG.A; return 4; };
	OpCode[0x02] = [&]()->int {_Memory.MemoryWrite(_REG.B << 8 | _REG.C, _REG.A); return 8; };
	OpCode[0x12] = [&]()->int {_Memory.MemoryWrite(_REG.D << 8 | _REG.E, _REG.A); return 8; };
	OpCode[0x77] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.A); return 8; };
	OpCode[0xEA] = [&]()->int {_Memory.MemoryWrite(_Memory.MemoryRead(_REG.PC++) | _Memory.MemoryRead(_REG.PC++) << 8, _REG.A); return 16; };

	//LD A,(C)
	//p70
	OpCode[0xF2] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.C + 0xFF00); return 8; };

	//LD (C),A
	OpCode[0xE2] = [&]()->int {_Memory.MemoryWrite(_REG.C + 0xFF00, _REG.A); return 8; };

	//LDD A,(HL)(HL),A
	//P71
	OpCode[0x3A] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.H << 8 | _REG.L); if (_REG.L == 0) { _REG.H--; } _REG.L--; return 8; };
	OpCode[0x32] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.A);  if (_REG.L == 0) { _REG.H--; }_REG.L--; return 8; };

	//LDI A,(HL)(HL),A
	//p72
	OpCode[0x2A] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.H << 8 | _REG.L); if (_REG.L == 0xFF) { _REG.H++; } _REG.L++; return 8; };
	OpCode[0x22] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, _REG.A);  if (_REG.L == 0xFF) { _REG.H++; }_REG.L++; return 8; };

	//LDH (n),A
	//p75
	OpCode[0xE0] = [&]()->int {_Memory.MemoryWrite(0xFF00 + _Memory.MemoryRead(_REG.PC++), _REG.A); return 12; };
	OpCode[0xF0] = [&]()->int {_REG.A = _Memory.MemoryRead(0xFF00 + _Memory.MemoryRead(_REG.PC++)); return 12; };

	//LD n,nn
	//p76
	OpCode[0x01] = [&]()->int {_REG.C = _Memory.MemoryRead(_REG.PC++); _REG.B = _Memory.MemoryRead(_REG.PC++); return 12; };
	OpCode[0x11] = [&]()->int {_REG.E = _Memory.MemoryRead(_REG.PC++); _REG.D = _Memory.MemoryRead(_REG.PC++); return 12; };
	OpCode[0x21] = [&]()->int {_REG.L = _Memory.MemoryRead(_REG.PC++); _REG.H = _Memory.MemoryRead(_REG.PC++); return 12; };
	OpCode[0x31] = [&]()->int {_REG.SP = _Memory.MemoryRead(_REG.PC++) | _Memory.MemoryRead(_REG.PC++) << 8; return 12; };

	//LD SP,HL
	OpCode[0xF9] = [&]()->int {_REG.SP = _REG.H << 8 | _REG.L; return 8; };
	//LDHL SP,n
	//p77
	//flagf
	OpCode[0xF8] = [&]()->int {LDHL(); return 12; };//address?
	//LD (nn),SP
	OpCode[0x08] = [&]()->int {GB_DB ads = _Memory.MemoryRead(_REG.PC) | _Memory.MemoryRead(_REG.PC + 1) << 8; _Memory.MemoryWrite(ads, _REG.SP & 0xFF); _Memory.MemoryWrite(ads+1, (_REG.SP & 0xFF00)>>8); _REG.PC += 2; return 20; };
	//PUSH nn
	OpCode[0xF5] = [&]()->int {_REG.SP -= 2; _Memory.MemoryWrite(_REG.SP, _REG.F); _Memory.MemoryWrite(_REG.SP+1, _REG.A); return 16; };
	OpCode[0xC5] = [&]()->int {_REG.SP -= 2; _Memory.MemoryWrite(_REG.SP, _REG.C); _Memory.MemoryWrite(_REG.SP+1, _REG.B); return 16; };
	OpCode[0xD5] = [&]()->int {_REG.SP -= 2; _Memory.MemoryWrite(_REG.SP, _REG.E); _Memory.MemoryWrite(_REG.SP+1, _REG.D); return 16; };
	OpCode[0xE5] = [&]()->int {_REG.SP -= 2; _Memory.MemoryWrite(_REG.SP, _REG.L); _Memory.MemoryWrite(_REG.SP+1, _REG.H); return 16; };
	//POP nn
	OpCode[0xF1] = [&]()->int {_REG.A = _Memory.MemoryRead(_REG.SP+1); _REG.F = _Memory.MemoryRead(_REG.SP) & 0xF0; _REG.SP += 2; return 12; };
	OpCode[0xC1] = [&]()->int {_REG.B = _Memory.MemoryRead(_REG.SP+1); _REG.C = _Memory.MemoryRead(_REG.SP); _REG.SP += 2; return 12; };
	OpCode[0xD1] = [&]()->int {_REG.D = _Memory.MemoryRead(_REG.SP+1); _REG.E = _Memory.MemoryRead(_REG.SP); _REG.SP += 2; return 12; };
	OpCode[0xE1] = [&]()->int {_REG.H = _Memory.MemoryRead(_REG.SP+1); _REG.L = _Memory.MemoryRead(_REG.SP); _REG.SP += 2; return 12; };//16bit bus?
	//ADD A,n
	//p80
	//flagf
	OpCode[0x87] = [&]()->int {ADD(_REG.A); return 4; };
	OpCode[0x80] = [&]()->int {ADD(_REG.B); return 4; };
	OpCode[0x81] = [&]()->int {ADD(_REG.C); return 4; };
	OpCode[0x82] = [&]()->int {ADD(_REG.D); return 4; };
	OpCode[0x83] = [&]()->int {ADD(_REG.E); return 4; };
	OpCode[0x84] = [&]()->int {ADD(_REG.H); return 4; };
	OpCode[0x85] = [&]()->int {ADD(_REG.L); return 4; };
	OpCode[0x86] = [&]()->int {ADD(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xC6] = [&]()->int {ADD(_Memory.MemoryRead(_REG.PC++)); return 8; };

	//ADC A,n
	//p81
	//flagf
	OpCode[0x8F] = [&]()->int {ADC(_REG.A); return 4; };
	OpCode[0x88] = [&]()->int {ADC(_REG.B); return 4; };
	OpCode[0x89] = [&]()->int {ADC(_REG.C); return 4; };
	OpCode[0x8A] = [&]()->int {ADC(_REG.D); return 4; };
	OpCode[0x8B] = [&]()->int {ADC(_REG.E); return 4; };
	OpCode[0x8C] = [&]()->int {ADC(_REG.H); return 4; };
	OpCode[0x8D] = [&]()->int {ADC(_REG.L); return 4; };
	OpCode[0x8E] = [&]()->int {ADC(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xCE] = [&]()->int {ADC(_Memory.MemoryRead(_REG.PC++)); return 8; };


	//SUB A,n
	//p82
	//flagf
	OpCode[0x97] = [&]()->int {SUB(_REG.A); return 4; };
	OpCode[0x90] = [&]()->int {SUB(_REG.B); return 4; };
	OpCode[0x91] = [&]()->int {SUB(_REG.C); return 4; };
	OpCode[0x92] = [&]()->int {SUB(_REG.D); return 4; };
	OpCode[0x93] = [&]()->int {SUB(_REG.E); return 4; };
	OpCode[0x94] = [&]()->int {SUB(_REG.H); return 4; };
	OpCode[0x95] = [&]()->int {SUB(_REG.L); return 4; };
	OpCode[0x96] = [&]()->int {SUB(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xD6] = [&]()->int {SUB(_Memory.MemoryRead(_REG.PC++)); return 8; };

	//SBC A,n
	//p81
	//flagf
	OpCode[0x9F] = [&]()->int {SBC(_REG.A); return 4; };
	OpCode[0x98] = [&]()->int {SBC(_REG.B); return 4; };
	OpCode[0x99] = [&]()->int {SBC(_REG.C); return 4; };
	OpCode[0x9A] = [&]()->int {SBC(_REG.D); return 4; };
	OpCode[0x9B] = [&]()->int {SBC(_REG.E); return 4; };
	OpCode[0x9C] = [&]()->int {SBC(_REG.H); return 4; };
	OpCode[0x9D] = [&]()->int {SBC(_REG.L); return 4; };
	OpCode[0x9E] = [&]()->int {SBC(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xDE] = [&]()->int {SBC(_Memory.MemoryRead(_REG.PC++)); return 8; };

	//AND n
	//p84
	//flagf
	OpCode[0xA7] = [&]()->int {AND(_REG.A); return 4; };
	OpCode[0xA0] = [&]()->int {AND(_REG.B); return 4; };
	OpCode[0xA1] = [&]()->int {AND(_REG.C); return 4; };
	OpCode[0xA2] = [&]()->int {AND(_REG.D); return 4; };
	OpCode[0xA3] = [&]()->int {AND(_REG.E); return 4; };
	OpCode[0xA4] = [&]()->int {AND(_REG.H); return 4; };
	OpCode[0xA5] = [&]()->int {AND(_REG.L); return 4; };
	OpCode[0xA6] = [&]()->int {AND(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xE6] = [&]()->int {AND(_Memory.MemoryRead(_REG.PC++)); return 8; };
	//OR n
	//p85
	//flagf
	OpCode[0xB7] = [&]()->int {OR(_REG.A); return 4; };
	OpCode[0xB0] = [&]()->int {OR(_REG.B); return 4; };
	OpCode[0xB1] = [&]()->int {OR(_REG.C); return 4; };
	OpCode[0xB2] = [&]()->int {OR(_REG.D); return 4; };
	OpCode[0xB3] = [&]()->int {OR(_REG.E); return 4; };
	OpCode[0xB4] = [&]()->int {OR(_REG.H); return 4; };
	OpCode[0xB5] = [&]()->int {OR(_REG.L); return 4; };
	OpCode[0xB6] = [&]()->int {OR(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xF6] = [&]()->int {OR(_Memory.MemoryRead(_REG.PC++)); return 8; };

	//XOR n
	//p86
	//flagf
	OpCode[0xAF] = [&]()->int {XOR(_REG.A); return 4; };
	OpCode[0xA8] = [&]()->int {XOR(_REG.B); return 4; };
	OpCode[0xA9] = [&]()->int {XOR(_REG.C); return 4; };
	OpCode[0xAA] = [&]()->int {XOR(_REG.D); return 4; };
	OpCode[0xAB] = [&]()->int {XOR(_REG.E); return 4; };
	OpCode[0xAC] = [&]()->int {XOR(_REG.H); return 4; };
	OpCode[0xAD] = [&]()->int {XOR(_REG.L); return 4; };
	OpCode[0xAE] = [&]()->int {XOR(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xEE] = [&]()->int {XOR(_Memory.MemoryRead(_REG.PC++)); return 8; };//?

	//CP n
	//p87
	//flagf
	OpCode[0xBF] = [&]()->int {CP(_REG.A); return 4; };
	OpCode[0xB8] = [&]()->int {CP(_REG.B); return 4; };
	OpCode[0xB9] = [&]()->int {CP(_REG.C); return 4; };
	OpCode[0xBA] = [&]()->int {CP(_REG.D); return 4; };
	OpCode[0xBB] = [&]()->int {CP(_REG.E); return 4; };
	OpCode[0xBC] = [&]()->int {CP(_REG.H); return 4; };
	OpCode[0xBD] = [&]()->int {CP(_REG.L); return 4; };
	OpCode[0xBE] = [&]()->int {CP(_Memory.MemoryRead(_REG.H << 8 | _REG.L)); return 8; };
	OpCode[0xFE] = [&]()->int {CP(_Memory.MemoryRead(_REG.PC++)); return 8; };

	//INC n
	//p88
	//flagf
	OpCode[0x3C] = [&]()->int {INC(_REG.A); return 4; };
	OpCode[0x04] = [&]()->int {INC(_REG.B); return 4; };
	OpCode[0x0C] = [&]()->int {INC(_REG.C); return 4; };
	OpCode[0x14] = [&]()->int {INC(_REG.D); return 4; };
	OpCode[0x1C] = [&]()->int {INC(_REG.E); return 4; };
	OpCode[0x24] = [&]()->int {INC(_REG.H); return 4; };
	OpCode[0x2C] = [&]()->int {INC(_REG.L); return 4; };
	OpCode[0x34] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); INC(T); _Memory.MemoryWrite((_REG.H << 8 | _REG.L), T); return 12; };

	//DEC n
	//p89
	//flagf
	OpCode[0x3D] = [&]()->int {DEC(_REG.A); return 4; };
	OpCode[0x05] = [&]()->int {DEC(_REG.B); return 4; };
	OpCode[0x0D] = [&]()->int {DEC(_REG.C); return 4; };
	OpCode[0x15] = [&]()->int {DEC(_REG.D); return 4; };
	OpCode[0x1D] = [&]()->int {DEC(_REG.E); return 4; };
	OpCode[0x25] = [&]()->int {DEC(_REG.H); return 4; };
	OpCode[0x2D] = [&]()->int {DEC(_REG.L); return 4; };
	OpCode[0x35] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); DEC(T); _Memory.MemoryWrite((_REG.H << 8 | _REG.L), T); return 12; };

	//ADD HL,n
	//p90
	//flag
	OpCode[0x09] = [&]()->int {ADDHL(_REG.B,_REG.C); return 8; };
	OpCode[0x19] = [&]()->int {ADDHL(_REG.D, _REG.E); return 8; };
	OpCode[0x29] = [&]()->int {ADDHL(_REG.H, _REG.L); return 8; };
	OpCode[0x39] = [&]()->int {ADDHL((_REG.SP&0xFF00)>>8, _REG.SP&0xFF); return 8; };

	//ADD SP.n
	//p91
	//flag
	//unfinished
	//n should be signed......
	OpCode[0xE8] = [&]()->int {
		__int8 by = _Memory.MemoryRead(_REG.PC++);
		int re = _REG.SP + by;
		SetFlag(FLAG_HACA, (((_REG.SP & 0x0F) + (by & 0x0F)) & 0x10) != 0);
		SetFlag(FLAG_CARY, (((_REG.SP & 0xFF) + (by & 0xFF)) & 0x100) != 0);
		
		SetFlag(FLAG_ZERO, 0);
		SetFlag(FLAG_NEGA, 0);
		_REG.SP = re&0xffff;
		return 16;
	};

	  //INC nn
	  //p92
	OpCode[0x03] = [&]()->int {_REG.C++; if (_REG.C == 0)_REG.B++; return 8; };
	OpCode[0x13] = [&]()->int {_REG.E++; if (_REG.E == 0)_REG.D++; return 8; };
	OpCode[0x23] = [&]()->int {_REG.L++; if (_REG.L == 0)_REG.H++; return 8; };
	OpCode[0x33] = [&]()->int {_REG.SP++; return 8; };

	//DEC nn
	//p93
	OpCode[0x0B] = [&]()->int {_REG.C--; if (_REG.C == 0xFF)_REG.B--; return 8; };
	OpCode[0x1B] = [&]()->int {_REG.E--; if (_REG.E == 0xFF)_REG.D--; return 8; };
	OpCode[0x2B] = [&]()->int {_REG.L--; if (_REG.L == 0xFF)_REG.H--; return 8; };
	OpCode[0x3B] = [&]()->int {_REG.SP--; return 8; };

	//CB
	OpCode[0xCB] = [&]()->int { return CBOpCode[_Memory.MemoryRead(_REG.PC++)]();};
	//SWAP
	//p94
	//flagf
	CBOpCode[0x37] = [&]()->int {SWAP(_REG.A); return 8; };
	CBOpCode[0x30] = [&]()->int {SWAP(_REG.B); return 8; };
	CBOpCode[0x31] = [&]()->int {SWAP(_REG.C); return 8; };
	CBOpCode[0x32] = [&]()->int {SWAP(_REG.D); return 8; };
	CBOpCode[0x33] = [&]()->int {SWAP(_REG.E); return 8; };
	CBOpCode[0x34] = [&]()->int {SWAP(_REG.H); return 8; };
	CBOpCode[0x35] = [&]()->int {SWAP(_REG.L); return 8; };
	CBOpCode[0x36] = [&]()->int {GB_BY by = _Memory.MemoryRead(_REG.H << 8 | _REG.L); SWAP(by); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, by); return 16; };

	//DAA
	//p95
	//flagf
	
	OpCode[0x27] = [&]()->int {
		GB_DB tmp = _REG.A;
		if(GetFlag(FLAG_NEGA)){
			if (GetFlag(FLAG_HACA)) {
				tmp = (tmp-0x06)&0xff;
			}
			if (GetFlag(FLAG_CARY)) {
				tmp = (tmp-0x60)&0xff;
			}
		}else{
			if (GetFlag(FLAG_HACA) || (tmp & 0xf) > 9) {
				tmp += 0x06;
			}
			if (GetFlag(FLAG_CARY) || tmp> 0x9f) {
				tmp += 0x60;
			}
		}
		SetFlag(FLAG_HACA,0);
		if (tmp > 0xff) {
			SetFlag(FLAG_CARY, 1);
		}
		tmp &= 0xff;
		SetFlag(FLAG_ZERO, tmp==0);
		_REG.A = tmp & 0xff;
		return 4;
	};
	 
	//CPL
	//flagf
	OpCode[0x2F] = [&]()->int {SetFlag(FLAG_NEGA, 1); SetFlag(FLAG_HACA, 1); _REG.A = ~_REG.A; return 4; };

	//CCF
	//flagf
	OpCode[0x3F] = [&]()->int {SetFlag(FLAG_NEGA, 0); SetFlag(FLAG_HACA, 0); _REG.F^=0x10; return 4; };

	//SCF
	//flagf
	OpCode[0x37] = [&]()->int {SetFlag(FLAG_NEGA, 0); SetFlag(FLAG_HACA, 0); SetFlag(FLAG_CARY, 1); return 4; };

	//Nop
	OpCode[0x00] = [&]()->int {return 4; };

	//HALT
	
	OpCode[0x76] = [&]()->int {isPause = 1; return 4; };//for further ?

	//STOP
	
	OpCode[0x10] = [&]()->int {isStop = 1; _REG.PC++; if ((_Memory.MemoryRead(0xFF4D) & 1))_Memory.MemoryWrite(0xFF4D, 0xFE); return 4; };//set cpu and lcd pause

	 //DI,EI
	 //p98
	
	OpCode[0xF3] = [&]()->int {_REG.IME = 0; return 4; };

	OpCode[0xFB] = [&]()->int {_REG.IME = 1; return 4; };
	//RLCA
	//flagf
	
	OpCode[0x07] = [&]()->int {
		GB_BY by = 0x80 & _REG.A;
		_REG.A <<= 1;
		_REG.A |= by >> 7;
		SetFlag(FLAG_ZERO, 0);
		SetFlag(FLAG_NEGA, 0);
		SetFlag(FLAG_HACA, 0);
		SetFlag(FLAG_CARY, by);
		return 4;
	};
	//RLA
	//flagf
	OpCode[0x17] = [&]()->int {
		GB_BY by = 0x80 & _REG.A;
		_REG.A <<= 1;
		_REG.A |= GetFlag(FLAG_CARY);
		SetFlag(FLAG_ZERO, 0);
		SetFlag(FLAG_NEGA, 0);
		SetFlag(FLAG_HACA, 0);
		SetFlag(FLAG_CARY, by);
		return 4;
	};
	//RRCA
	//flagf
	OpCode[0x0F] = [&]()->int {
		GB_BY by = 0x1 & _REG.A;
		_REG.A >>= 1;
		_REG.A |= by << 7;
		SetFlag(FLAG_ZERO, 0);
		SetFlag(FLAG_NEGA, 0);
		SetFlag(FLAG_HACA, 0);
		SetFlag(FLAG_CARY, by);
		return 4;
	};
	//RRA
	//flagf
	OpCode[0x1F] = [&]()->int {
		GB_BY by = 0x1 & _REG.A;
		_REG.A >>= 1;
		_REG.A |= (GetFlag(FLAG_CARY) << 7);
		SetFlag(FLAG_ZERO, 0);
		SetFlag(FLAG_NEGA, 0);
		SetFlag(FLAG_HACA, 0);
		SetFlag(FLAG_CARY, by & 0x1);
		return 4;
	};

	//RLC
	//p99
	//flagf
	CBOpCode[0x07] = [&]()->int {RLC(_REG.A); return 8; };
	CBOpCode[0x00] = [&]()->int {RLC(_REG.B); return 8; };
	CBOpCode[0x01] = [&]()->int {RLC(_REG.C); return 8; };
	CBOpCode[0x02] = [&]()->int {RLC(_REG.D); return 8; };
	CBOpCode[0x03] = [&]()->int {RLC(_REG.E); return 8; };
	CBOpCode[0x04] = [&]()->int {RLC(_REG.H); return 8; };
	CBOpCode[0x05] = [&]()->int {RLC(_REG.L); return 8; };
	CBOpCode[0x06] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); RLC(T); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, T); return 16; };

	//RL
	//p99
	//flagf
	CBOpCode[0x17] = [&]()->int {RL(_REG.A); return 8; };
	CBOpCode[0x10] = [&]()->int {RL(_REG.B); return 8; };
	CBOpCode[0x11] = [&]()->int {RL(_REG.C); return 8; };
	CBOpCode[0x12] = [&]()->int {RL(_REG.D); return 8; };
	CBOpCode[0x13] = [&]()->int {RL(_REG.E); return 8; };
	CBOpCode[0x14] = [&]()->int {RL(_REG.H); return 8; };
	CBOpCode[0x15] = [&]()->int {RL(_REG.L); return 8; };
	CBOpCode[0x16] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); RL(T); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, T); return 16; };

	//RRC
	//p103
	//flagf
	CBOpCode[0x0F] = [&]()->int {RRC(_REG.A); return 8; };
	CBOpCode[0x08] = [&]()->int {RRC(_REG.B); return 8; };
	CBOpCode[0x09] = [&]()->int {RRC(_REG.C); return 8; };
	CBOpCode[0x0A] = [&]()->int {RRC(_REG.D); return 8; };
	CBOpCode[0x0B] = [&]()->int {RRC(_REG.E); return 8; };
	CBOpCode[0x0C] = [&]()->int {RRC(_REG.H); return 8; };
	CBOpCode[0x0D] = [&]()->int {RRC(_REG.L); return 8; };
	CBOpCode[0x0E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); RRC(T); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, T); return 16; };

	//RR
	//p103
	//flagf
	CBOpCode[0x1F] = [&]()->int {RR(_REG.A); return 8; };
	CBOpCode[0x18] = [&]()->int {RR(_REG.B); return 8; };
	CBOpCode[0x19] = [&]()->int {RR(_REG.C); return 8; };
	CBOpCode[0x1A] = [&]()->int {RR(_REG.D); return 8; };
	CBOpCode[0x1B] = [&]()->int {RR(_REG.E); return 8; };
	CBOpCode[0x1C] = [&]()->int {RR(_REG.H); return 8; };
	CBOpCode[0x1D] = [&]()->int {RR(_REG.L); return 8; };
	CBOpCode[0x1E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); RR(T); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, T); return 16; };

	//SLA
	//p105
	//flagf
	CBOpCode[0x27] = [&]()->int {SLA(_REG.A); return 8; };
	CBOpCode[0x20] = [&]()->int {SLA(_REG.B); return 8; };
	CBOpCode[0x21] = [&]()->int {SLA(_REG.C); return 8; };
	CBOpCode[0x22] = [&]()->int {SLA(_REG.D); return 8; };
	CBOpCode[0x23] = [&]()->int {SLA(_REG.E); return 8; };
	CBOpCode[0x24] = [&]()->int {SLA(_REG.H); return 8; };
	CBOpCode[0x25] = [&]()->int {SLA(_REG.L); return 8; };
	CBOpCode[0x26] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); SLA(T); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, T); return 16; };

	//SRA
	//p106
	//flagf
	CBOpCode[0x2F] = [&]()->int {SRA(_REG.A); return 8; };
	CBOpCode[0x28] = [&]()->int {SRA(_REG.B); return 8; };
	CBOpCode[0x29] = [&]()->int {SRA(_REG.C); return 8; };
	CBOpCode[0x2A] = [&]()->int {SRA(_REG.D); return 8; };
	CBOpCode[0x2B] = [&]()->int {SRA(_REG.E); return 8; };
	CBOpCode[0x2C] = [&]()->int {SRA(_REG.H); return 8; };
	CBOpCode[0x2D] = [&]()->int {SRA(_REG.L); return 8; };
	CBOpCode[0x2E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); SRA(T); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, T); return 16; };

	//SRL
	//p107
	//flagf
	CBOpCode[0x3F] = [&]()->int {SRL(_REG.A); return 8; };
	CBOpCode[0x38] = [&]()->int {SRL(_REG.B); return 8; };
	CBOpCode[0x39] = [&]()->int {SRL(_REG.C); return 8; };
	CBOpCode[0x3A] = [&]()->int {SRL(_REG.D); return 8; };
	CBOpCode[0x3B] = [&]()->int {SRL(_REG.E); return 8; };
	CBOpCode[0x3C] = [&]()->int {SRL(_REG.H); return 8; };
	CBOpCode[0x3D] = [&]()->int {SRL(_REG.L); return 8; };
	CBOpCode[0x3E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); SRL(T); _Memory.MemoryWrite(_REG.H << 8 | _REG.L, T); return 16; };

	//BIT
	//P108
	//flagf
	CBOpCode[0x47] = [&]()->int {BIT(_REG.A, 0); return 8; };
	CBOpCode[0x40] = [&]()->int {BIT(_REG.B, 0); return 8; };
	CBOpCode[0x41] = [&]()->int {BIT(_REG.C, 0); return 8; };
	CBOpCode[0x42] = [&]()->int {BIT(_REG.D, 0); return 8; };
	CBOpCode[0x43] = [&]()->int {BIT(_REG.E, 0); return 8; };
	CBOpCode[0x44] = [&]()->int {BIT(_REG.H, 0); return 8; };
	CBOpCode[0x45] = [&]()->int {BIT(_REG.L, 0); return 8; };
	CBOpCode[0x46] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 0); return 12; };

	CBOpCode[0x4F] = [&]()->int {BIT(_REG.A, 1); return 8; };
	CBOpCode[0x48] = [&]()->int {BIT(_REG.B, 1); return 8; };
	CBOpCode[0x49] = [&]()->int {BIT(_REG.C, 1); return 8; };
	CBOpCode[0x4A] = [&]()->int {BIT(_REG.D, 1); return 8; };
	CBOpCode[0x4B] = [&]()->int {BIT(_REG.E, 1); return 8; };
	CBOpCode[0x4C] = [&]()->int {BIT(_REG.H, 1); return 8; };
	CBOpCode[0x4D] = [&]()->int {BIT(_REG.L, 1); return 8; };
	CBOpCode[0x4E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 1); return 12; };

	CBOpCode[0x57] = [&]()->int {BIT(_REG.A, 2); return 8; };
	CBOpCode[0x50] = [&]()->int {BIT(_REG.B, 2); return 8; };
	CBOpCode[0x51] = [&]()->int {BIT(_REG.C, 2); return 8; };
	CBOpCode[0x52] = [&]()->int {BIT(_REG.D, 2); return 8; };
	CBOpCode[0x53] = [&]()->int {BIT(_REG.E, 2); return 8; };
	CBOpCode[0x54] = [&]()->int {BIT(_REG.H, 2); return 8; };
	CBOpCode[0x55] = [&]()->int {BIT(_REG.L, 2); return 8; };
	CBOpCode[0x56] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 2); return 12; };

	CBOpCode[0x5F] = [&]()->int {BIT(_REG.A, 3); return 8; };
	CBOpCode[0x58] = [&]()->int {BIT(_REG.B, 3); return 8; };
	CBOpCode[0x59] = [&]()->int {BIT(_REG.C, 3); return 8; };
	CBOpCode[0x5A] = [&]()->int {BIT(_REG.D, 3); return 8; };
	CBOpCode[0x5B] = [&]()->int {BIT(_REG.E, 3); return 8; };
	CBOpCode[0x5C] = [&]()->int {BIT(_REG.H, 3); return 8; };
	CBOpCode[0x5D] = [&]()->int {BIT(_REG.L, 3); return 8; };
	CBOpCode[0x5E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 3); return 12; };

	CBOpCode[0x67] = [&]()->int {BIT(_REG.A, 4); return 8; };
	CBOpCode[0x60] = [&]()->int {BIT(_REG.B, 4); return 8; };
	CBOpCode[0x61] = [&]()->int {BIT(_REG.C, 4); return 8; };
	CBOpCode[0x62] = [&]()->int {BIT(_REG.D, 4); return 8; };
	CBOpCode[0x63] = [&]()->int {BIT(_REG.E, 4); return 8; };
	CBOpCode[0x64] = [&]()->int {BIT(_REG.H, 4); return 8; };
	CBOpCode[0x65] = [&]()->int {BIT(_REG.L, 4); return 8; };
	CBOpCode[0x66] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 4); return 12; };

	CBOpCode[0x6F] = [&]()->int {BIT(_REG.A, 5); return 8; };
	CBOpCode[0x68] = [&]()->int {BIT(_REG.B, 5); return 8; };
	CBOpCode[0x69] = [&]()->int {BIT(_REG.C, 5); return 8; };
	CBOpCode[0x6A] = [&]()->int {BIT(_REG.D, 5); return 8; };
	CBOpCode[0x6B] = [&]()->int {BIT(_REG.E, 5); return 8; };
	CBOpCode[0x6C] = [&]()->int {BIT(_REG.H, 5); return 8; };
	CBOpCode[0x6D] = [&]()->int {BIT(_REG.L, 5); return 8; };
	CBOpCode[0x6E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 5); return 12; };

	CBOpCode[0x77] = [&]()->int {BIT(_REG.A, 6); return 8; };
	CBOpCode[0x70] = [&]()->int {BIT(_REG.B, 6); return 8; };
	CBOpCode[0x71] = [&]()->int {BIT(_REG.C, 6); return 8; };
	CBOpCode[0x72] = [&]()->int {BIT(_REG.D, 6); return 8; };
	CBOpCode[0x73] = [&]()->int {BIT(_REG.E, 6); return 8; };
	CBOpCode[0x74] = [&]()->int {BIT(_REG.H, 6); return 8; };
	CBOpCode[0x75] = [&]()->int {BIT(_REG.L, 6); return 8; };
	CBOpCode[0x76] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 6); return 12; };

	CBOpCode[0x7F] = [&]()->int {BIT(_REG.A, 7); return 8; };
	CBOpCode[0x78] = [&]()->int {BIT(_REG.B, 7); return 8; };
	CBOpCode[0x79] = [&]()->int {BIT(_REG.C, 7); return 8; };
	CBOpCode[0x7A] = [&]()->int {BIT(_REG.D, 7); return 8; };
	CBOpCode[0x7B] = [&]()->int {BIT(_REG.E, 7); return 8; };
	CBOpCode[0x7C] = [&]()->int {BIT(_REG.H, 7); return 8; };
	CBOpCode[0x7D] = [&]()->int {BIT(_REG.L, 7); return 8; };
	CBOpCode[0x7E] = [&]()->int {GB_BY T = _Memory.MemoryRead(_REG.H << 8 | _REG.L); BIT(T, 7); return 12; };

	//SET
	//P109

	CBOpCode[0xC7] = [&]()->int {_REG.A |=  ( 1 << 0); return 8; };
	CBOpCode[0xC0] = [&]()->int {_REG.B |=  ( 1 << 0); return 8; };
	CBOpCode[0xC1] = [&]()->int {_REG.C |=  ( 1 << 0); return 8; };
	CBOpCode[0xC2] = [&]()->int {_REG.D |=  ( 1 << 0); return 8; };
	CBOpCode[0xC3] = [&]()->int {_REG.E |=  ( 1 << 0); return 8; };
	CBOpCode[0xC4] = [&]()->int {_REG.H |=  ( 1 << 0); return 8; };
	CBOpCode[0xC5] = [&]()->int {_REG.L |=  ( 1 << 0); return 8; };
	CBOpCode[0xC6] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 0))); return 16; };

	CBOpCode[0xCF] = [&]()->int {_REG.A |=  ( 1 << 1); return 8; };
	CBOpCode[0xC8] = [&]()->int {_REG.B |=  ( 1 << 1); return 8; };
	CBOpCode[0xC9] = [&]()->int {_REG.C |=  ( 1 << 1); return 8; };
	CBOpCode[0xCA] = [&]()->int {_REG.D |=  ( 1 << 1); return 8; };
	CBOpCode[0xCB] = [&]()->int {_REG.E |=  ( 1 << 1); return 8; };
	CBOpCode[0xCC] = [&]()->int {_REG.H |=  ( 1 << 1); return 8; };
	CBOpCode[0xCD] = [&]()->int {_REG.L |=  ( 1 << 1); return 8; };
	CBOpCode[0xCE] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 1))); return 16; };

	CBOpCode[0xD7] = [&]()->int {_REG.A |=  ( 1 << 2); return 8; };
	CBOpCode[0xD0] = [&]()->int {_REG.B |=  ( 1 << 2); return 8; };
	CBOpCode[0xD1] = [&]()->int {_REG.C |=  ( 1 << 2); return 8; };
	CBOpCode[0xD2] = [&]()->int {_REG.D |=  ( 1 << 2); return 8; };
	CBOpCode[0xD3] = [&]()->int {_REG.E |=  ( 1 << 2); return 8; };
	CBOpCode[0xD4] = [&]()->int {_REG.H |=  ( 1 << 2); return 8; };
	CBOpCode[0xD5] = [&]()->int {_REG.L |=  ( 1 << 2); return 8; };
	CBOpCode[0xD6] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 2))); return 16; };

	CBOpCode[0xDF] = [&]()->int {_REG.A |=  ( 1 << 3); return 8; };
	CBOpCode[0xD8] = [&]()->int {_REG.B |=  ( 1 << 3); return 8; };
	CBOpCode[0xD9] = [&]()->int {_REG.C |=  ( 1 << 3); return 8; };
	CBOpCode[0xDA] = [&]()->int {_REG.D |=  ( 1 << 3); return 8; };
	CBOpCode[0xDB] = [&]()->int {_REG.E |=  ( 1 << 3); return 8; };
	CBOpCode[0xDC] = [&]()->int {_REG.H |=  ( 1 << 3); return 8; };
	CBOpCode[0xDD] = [&]()->int {_REG.L |=  ( 1 << 3); return 8; };
	CBOpCode[0xDE] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 3))); return 16; };

	CBOpCode[0xE7] = [&]()->int {_REG.A |=  ( 1 << 4); return 8; };
	CBOpCode[0xE0] = [&]()->int {_REG.B |=  ( 1 << 4); return 8; };
	CBOpCode[0xE1] = [&]()->int {_REG.C |=  ( 1 << 4); return 8; };
	CBOpCode[0xE2] = [&]()->int {_REG.D |=  ( 1 << 4); return 8; };
	CBOpCode[0xE3] = [&]()->int {_REG.E |=  ( 1 << 4); return 8; };
	CBOpCode[0xE4] = [&]()->int {_REG.H |=  ( 1 << 4); return 8; };
	CBOpCode[0xE5] = [&]()->int {_REG.L |=  ( 1 << 4); return 8; };
	CBOpCode[0xE6] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 4))); return 16; };

	CBOpCode[0xEF] = [&]()->int {_REG.A |=  ( 1 << 5); return 8; };
	CBOpCode[0xE8] = [&]()->int {_REG.B |=  ( 1 << 5); return 8; };
	CBOpCode[0xE9] = [&]()->int {_REG.C |=  ( 1 << 5); return 8; };
	CBOpCode[0xEA] = [&]()->int {_REG.D |=  ( 1 << 5); return 8; };
	CBOpCode[0xEB] = [&]()->int {_REG.E |=  ( 1 << 5); return 8; };
	CBOpCode[0xEC] = [&]()->int {_REG.H |=  ( 1 << 5); return 8; };
	CBOpCode[0xED] = [&]()->int {_REG.L |=  ( 1 << 5); return 8; };
	CBOpCode[0xEE] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 5))); return 16; };

	CBOpCode[0xF7] = [&]()->int {_REG.A |=  ( 1 << 6); return 8; };
	CBOpCode[0xF0] = [&]()->int {_REG.B |=  ( 1 << 6); return 8; };
	CBOpCode[0xF1] = [&]()->int {_REG.C |=  ( 1 << 6); return 8; };
	CBOpCode[0xF2] = [&]()->int {_REG.D |=  ( 1 << 6); return 8; };
	CBOpCode[0xF3] = [&]()->int {_REG.E |=  ( 1 << 6); return 8; };
	CBOpCode[0xF4] = [&]()->int {_REG.H |=  ( 1 << 6); return 8; };
	CBOpCode[0xF5] = [&]()->int {_REG.L |=  ( 1 << 6); return 8; };
	CBOpCode[0xF6] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 6))); return 16; };

	CBOpCode[0xFF] = [&]()->int {_REG.A |=  ( 1 << 7); return 8; };
	CBOpCode[0xF8] = [&]()->int {_REG.B |=  ( 1 << 7); return 8; };
	CBOpCode[0xF9] = [&]()->int {_REG.C |=  ( 1 << 7); return 8; };
	CBOpCode[0xFA] = [&]()->int {_REG.D |=  ( 1 << 7); return 8; };
	CBOpCode[0xFB] = [&]()->int {_REG.E |=  ( 1 << 7); return 8; };
	CBOpCode[0xFC] = [&]()->int {_REG.H |=  ( 1 << 7); return 8; };
	CBOpCode[0xFD] = [&]()->int {_REG.L |=  ( 1 << 7); return 8; };
	CBOpCode[0xFE] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) |  ( 1 << 7))); return 16; };

	//RES
	//P110

	CBOpCode[0x87] = [&]()->int {_REG.A &= ~( 1 << 0); return 8; };
	CBOpCode[0x80] = [&]()->int {_REG.B &= ~( 1 << 0); return 8; };
	CBOpCode[0x81] = [&]()->int {_REG.C &= ~( 1 << 0); return 8; };
	CBOpCode[0x82] = [&]()->int {_REG.D &= ~( 1 << 0); return 8; };
	CBOpCode[0x83] = [&]()->int {_REG.E &= ~( 1 << 0); return 8; };
	CBOpCode[0x84] = [&]()->int {_REG.H &= ~( 1 << 0); return 8; };
	CBOpCode[0x85] = [&]()->int {_REG.L &= ~( 1 << 0); return 8; };
	CBOpCode[0x86] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 <<0))); return 16; };

	CBOpCode[0x8F] = [&]()->int {_REG.A &= ~( 1 << 1); return 8; };
	CBOpCode[0x88] = [&]()->int {_REG.B &= ~( 1 << 1); return 8; };
	CBOpCode[0x89] = [&]()->int {_REG.C &= ~( 1 << 1); return 8; };
	CBOpCode[0x8A] = [&]()->int {_REG.D &= ~( 1 << 1); return 8; };
	CBOpCode[0x8B] = [&]()->int {_REG.E &= ~( 1 << 1); return 8; };
	CBOpCode[0x8C] = [&]()->int {_REG.H &= ~( 1 << 1); return 8; };
	CBOpCode[0x8D] = [&]()->int {_REG.L &= ~( 1 << 1); return 8; };
	CBOpCode[0x8E] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 << 1))); return 16; };

	CBOpCode[0x97] = [&]()->int {_REG.A &= ~( 1 << 2); return 8; };
	CBOpCode[0x90] = [&]()->int {_REG.B &= ~( 1 << 2); return 8; };
	CBOpCode[0x91] = [&]()->int {_REG.C &= ~( 1 << 2); return 8; };
	CBOpCode[0x92] = [&]()->int {_REG.D &= ~( 1 << 2); return 8; };
	CBOpCode[0x93] = [&]()->int {_REG.E &= ~( 1 << 2); return 8; };
	CBOpCode[0x94] = [&]()->int {_REG.H &= ~( 1 << 2); return 8; };
	CBOpCode[0x95] = [&]()->int {_REG.L &= ~( 1 << 2); return 8; };
	CBOpCode[0x96] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 << 2))); return 16; };

	CBOpCode[0x9F] = [&]()->int {_REG.A &= ~( 1 << 3); return 8; };
	CBOpCode[0x98] = [&]()->int {_REG.B &= ~( 1 << 3); return 8; };
	CBOpCode[0x99] = [&]()->int {_REG.C &= ~( 1 << 3); return 8; };
	CBOpCode[0x9A] = [&]()->int {_REG.D &= ~( 1 << 3); return 8; };
	CBOpCode[0x9B] = [&]()->int {_REG.E &= ~( 1 << 3); return 8; };
	CBOpCode[0x9C] = [&]()->int {_REG.H &= ~( 1 << 3); return 8; };
	CBOpCode[0x9D] = [&]()->int {_REG.L &= ~( 1 << 3); return 8; };
	CBOpCode[0x9E] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 << 3))); return 16; };

	CBOpCode[0xA7] = [&]()->int {_REG.A &= ~( 1 << 4); return 8; };
	CBOpCode[0xA0] = [&]()->int {_REG.B &= ~( 1 << 4); return 8; };
	CBOpCode[0xA1] = [&]()->int {_REG.C &= ~( 1 << 4); return 8; };
	CBOpCode[0xA2] = [&]()->int {_REG.D &= ~( 1 << 4); return 8; };
	CBOpCode[0xA3] = [&]()->int {_REG.E &= ~( 1 << 4); return 8; };
	CBOpCode[0xA4] = [&]()->int {_REG.H &= ~( 1 << 4); return 8; };
	CBOpCode[0xA5] = [&]()->int {_REG.L &= ~( 1 << 4); return 8; };
	CBOpCode[0xA6] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 << 4))); return 16; };

	CBOpCode[0xAF] = [&]()->int {_REG.A &= ~( 1 << 5); return 8; };
	CBOpCode[0xA8] = [&]()->int {_REG.B &= ~( 1 << 5); return 8; };
	CBOpCode[0xA9] = [&]()->int {_REG.C &= ~( 1 << 5); return 8; };
	CBOpCode[0xAA] = [&]()->int {_REG.D &= ~( 1 << 5); return 8; };
	CBOpCode[0xAB] = [&]()->int {_REG.E &= ~( 1 << 5); return 8; };
	CBOpCode[0xAC] = [&]()->int {_REG.H &= ~( 1 << 5); return 8; };
	CBOpCode[0xAD] = [&]()->int {_REG.L &= ~( 1 << 5); return 8; };
	CBOpCode[0xAE] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 << 5))); return 16; };

	CBOpCode[0xB7] = [&]()->int {_REG.A &= ~( 1 << 6); return 8; };
	CBOpCode[0xB0] = [&]()->int {_REG.B &= ~( 1 << 6); return 8; };
	CBOpCode[0xB1] = [&]()->int {_REG.C &= ~( 1 << 6); return 8; };
	CBOpCode[0xB2] = [&]()->int {_REG.D &= ~( 1 << 6); return 8; };
	CBOpCode[0xB3] = [&]()->int {_REG.E &= ~( 1 << 6); return 8; };
	CBOpCode[0xB4] = [&]()->int {_REG.H &= ~( 1 << 6); return 8; };
	CBOpCode[0xB5] = [&]()->int {_REG.L &= ~( 1 << 6); return 8; };
	CBOpCode[0xB6] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 << 6))); return 16; };

	CBOpCode[0xBF] = [&]()->int {_REG.A &= ~( 1 << 7); return 8; };
	CBOpCode[0xB8] = [&]()->int {_REG.B &= ~( 1 << 7); return 8; };
	CBOpCode[0xB9] = [&]()->int {_REG.C &= ~( 1 << 7); return 8; };
	CBOpCode[0xBA] = [&]()->int {_REG.D &= ~( 1 << 7); return 8; };
	CBOpCode[0xBB] = [&]()->int {_REG.E &= ~( 1 << 7); return 8; };
	CBOpCode[0xBC] = [&]()->int {_REG.H &= ~( 1 << 7); return 8; };
	CBOpCode[0xBD] = [&]()->int {_REG.L &= ~( 1 << 7); return 8; };
	CBOpCode[0xBE] = [&]()->int {_Memory.MemoryWrite(_REG.H << 8 | _REG.L, (_Memory.MemoryRead(_REG.H << 8 | _REG.L) & ~( 1 << 7))); return 16; };

	//JP
	//p111
	OpCode[0xC3] = [&]()->int {JP(); return 16; };
	//JP nz
	OpCode[0xC2] = [&]()->int {if (!GetFlag(FLAG_ZERO)) { JP(); return 16; }_REG.PC += 2; return 12; };
	//JP z
	OpCode[0xCA] = [&]()->int {if (GetFlag(FLAG_ZERO)) { JP(); return 16; }_REG.PC += 2; return 12; };
	//JP nc
	OpCode[0xD2] = [&]()->int {if (!GetFlag(FLAG_CARY)) { JP(); return 16; }_REG.PC += 2; return 12; };
	//JP c
	OpCode[0xDA] = [&]()->int {if (GetFlag(FLAG_CARY)) { JP(); return 16; }_REG.PC += 2; return 12; };
	//JP hl
	OpCode[0xE9] = [&]()->int {_REG.PC = _REG.H << 8 | _REG.L; return 4; };
	//JR n (signed!)
	OpCode[0x18] = [&]()->int {_REG.PC += (__int8)_Memory.MemoryRead(_REG.PC); _REG.PC++; return 12; };
	//JR nz(signed!)
	OpCode[0x20] = [&]()->int {__int8 i = _Memory.MemoryRead(_REG.PC); _REG.PC++; if (!GetFlag(FLAG_ZERO)) { _REG.PC += (__int8)i; return 12; }return 8; };
	//JR z(signed!)
	OpCode[0x28] = [&]()->int {__int8 i = _Memory.MemoryRead(_REG.PC); _REG.PC++; if (GetFlag(FLAG_ZERO)) { _REG.PC += (__int8)i; return 12; }return 8; };
	//JR nc(signed!)
	OpCode[0x30] = [&]()->int {__int8 i = _Memory.MemoryRead(_REG.PC); _REG.PC++; if (!GetFlag(FLAG_CARY)) { _REG.PC += (__int8)i; return 12; }return 8; };
	//JR c(signed!)
	OpCode[0x38] = [&]()->int {__int8 i = _Memory.MemoryRead(_REG.PC); _REG.PC++; if (GetFlag(FLAG_CARY)) { _REG.PC += (__int8)i; return 12; }return 8; };

	//CALL 
	OpCode[0xCD] = [&]()->int {CALL();  return 24; };
	//CALL nz 
	OpCode[0xC4] = [&]()->int {if (!GetFlag(FLAG_ZERO)) { CALL(); return 24; }_REG.PC += 2; return 12; };
	//CALL z
	OpCode[0xCC] = [&]()->int {if (GetFlag(FLAG_ZERO)) { CALL();  return 24; }_REG.PC += 2; return 12; };
	//CALL nc
	OpCode[0xD4] = [&]()->int {if (!GetFlag(FLAG_CARY)) { CALL(); return 24; }_REG.PC += 2; return 12; };
	//CALL c
	OpCode[0xDC] = [&]()->int {if (GetFlag(FLAG_CARY)) { CALL(); return 24; }_REG.PC += 2; return 12; };

	//RST n
	//p116
	OpCode[0xC7] = [&]()->int {RST(); _REG.PC = 0; return 16; };
	OpCode[0xCF] = [&]()->int {RST(); _REG.PC = 0x8; return 16; };
	OpCode[0xD7] = [&]()->int {RST(); _REG.PC = 0x10; return 16; };
	OpCode[0xDF] = [&]()->int {RST(); _REG.PC = 0x18; return 16; };
	OpCode[0xE7] = [&]()->int {RST(); _REG.PC = 0x20; return 16; };
	OpCode[0xEF] = [&]()->int {RST(); _REG.PC = 0x28; return 16; };
	OpCode[0xF7] = [&]()->int {RST(); _REG.PC = 0x30; return 16; };
	OpCode[0xFF] = [&]()->int {RST(); _REG.PC = 0x38; return 16; };
	//RET
	//p117
	OpCode[0xC9] = [&]()->int {_REG.PC = _Memory.MemoryRead(_REG.SP) | (_Memory.MemoryRead(_REG.SP+1) << 8); _REG.SP += 2; return 16; };
	//RET cc
	OpCode[0xC0] = [&]()->int {if (!GetFlag(FLAG_ZERO)) { return OpCode[0xC9]()+4; }return 8; };
	OpCode[0xC8] = [&]()->int {if (GetFlag(FLAG_ZERO)) { return OpCode[0xC9]()+4; }return 8; };
	OpCode[0xD0] = [&]()->int {if (!GetFlag(FLAG_CARY)) { return OpCode[0xC9]()+4; }return 8; };
	OpCode[0xD8] = [&]()->int {if (GetFlag(FLAG_CARY)) { return OpCode[0xC9]()+4; }return 8; };
	//RETI
	OpCode[0xD9] = [&]()->int {OpCode[0xC9](); _REG.IME = 1; return 16; };
	
	
	OpCode[0xD3] = [&]()->int {SetFlag(FLAG_ZERO, 1); return 0; };
	OpCode[0xED] = [&]()->int {_Memory._inbios = 0; _REG.PC--; return 0; };
	//no such Opcode,but make my work easy,yay.
}

void Z80::LDHL() {
	SetFlag(FLAG_ZERO, 0);
	SetFlag(FLAG_NEGA, 0);
	
	__int8 byte = _Memory.MemoryRead(_REG.PC++);
	int re = _REG.SP+byte;
	SetFlag(FLAG_HACA, (((_REG.SP & 0x0F) + (byte & 0x0F)) & 0x10) != 0);
	SetFlag(FLAG_CARY, (((_REG.SP & 0xFF) + (byte & 0xFF)) & 0x100) != 0);
	
	_REG.L = re & 0xFF;
	_REG.H = (re >> 8) & 0xFF;
}

void Z80::ADD(GB_BY REG) {
	SetFlag(FLAG_NEGA, 0);
	GB_DB re = _REG.A + REG;
	SetFlag(FLAG_ZERO, (re & 0xff) == 0);
	SetFlag(FLAG_HACA, ((_REG.A & 0x0F) + (REG & 0x0F)) & 0x10);
	SetFlag(FLAG_CARY, re>0xff);
	_REG.A = re & 0xFF;

}
void Z80::ADDHL(GB_BY REGH,GB_BY REGL) {
	GB_DB hl = (_REG.H << 8) + _REG.L;
	GB_DB db = (REGH << 8) + REGL;
	uint32_t re = hl + db;
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, ((hl & 0x0fff) + (db & 0x0fff)) > 0x0fff);
	SetFlag(FLAG_CARY, re> 0xffff);
	_REG.H = (re & 0xFF00)>>8;
	_REG.L = re & 0xFF;
}
void Z80::ADC(GB_BY REG) {
	SetFlag(FLAG_NEGA, 0);
	GB_DB re = _REG.A + REG + GetFlag(FLAG_CARY);
	SetFlag(FLAG_ZERO, (re & 0xff) == 0);
	SetFlag(FLAG_HACA, ((_REG.A & 0x0F) + (REG & 0x0F) + GetFlag(FLAG_CARY)) & 0x10);
	SetFlag(FLAG_CARY, re>0xff);
	_REG.A = re & 0xFF;
}
void Z80::SUB(GB_BY REG) {
	SetFlag(FLAG_NEGA, 1);
	GB_DB re = _REG.A - REG;
	SetFlag(FLAG_ZERO, (re & 0xff) == 0);
	SetFlag(FLAG_HACA, (_REG.A & 0x0F) < (REG & 0x0F));
	SetFlag(FLAG_CARY, _REG.A<REG);
	_REG.A = re & 0xFF;
}
void Z80::SBC(GB_BY REG) {
	SetFlag(FLAG_NEGA, 1);
	GB_DB re = _REG.A - REG - GetFlag(FLAG_CARY);
	SetFlag(FLAG_ZERO, (re&0xff) == 0);
	SetFlag(FLAG_HACA, ((_REG.A ^ REG ^ (re & 0xff)) & (1 << 4)) != 0);
	SetFlag(FLAG_CARY, (re & 0x100)>>8);
	_REG.A = re & 0xFF;
}
void Z80::AND(GB_BY REG) {
	_REG.A &= REG;
	SetFlag(FLAG_ZERO, _REG.A == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 1);
	SetFlag(FLAG_CARY, 0);

}
void Z80::OR(GB_BY REG) {
	_REG.A |= REG;
	SetFlag(FLAG_ZERO, _REG.A == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, 0);

}
void Z80::XOR(GB_BY REG) {
	_REG.A ^= REG;
	SetFlag(FLAG_ZERO, _REG.A == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, 0);

}
void Z80::CP(GB_BY REG) {
	SetFlag(FLAG_NEGA, 1);
	GB_DB re = _REG.A - REG;
	SetFlag(FLAG_ZERO, (re & 0xff) == 0);
	SetFlag(FLAG_HACA, (_REG.A & 0x0F) < (REG & 0x0F));
	SetFlag(FLAG_CARY, REG>_REG.A);

}
void Z80::INC(GB_BY &REG) {
	SetFlag(FLAG_NEGA, 0);
	REG++;
	SetFlag(FLAG_ZERO, REG == 0);
	SetFlag(FLAG_HACA, (REG ^ (REG - 1)) & 0x10);
}
void Z80::DEC(GB_BY &REG) {
	SetFlag(FLAG_NEGA, 1);
	REG--;
	SetFlag(FLAG_ZERO, REG == 0);
	SetFlag(FLAG_HACA, (REG ^ (REG + 1)) & 0x10);
}
void Z80::EXADD(GB_BY HREG, GB_BY LREG) {
	GB_BY by = _Memory.MemoryRead(HREG << 8 | LREG);
	GB_DB HL = _REG.H << 8 | _REG.L;
	uint32_t re = HL + by;


	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, ((HL & 0xFF) + by) & 0x100);
	SetFlag(FLAG_CARY, (re & 0x10000)>>16);
	_REG.H = (re >> 8) & 0xFF;
	_REG.L = re & 0xFF;
}


void Z80::SWAP(GB_BY &REG) {
	GB_BY tmpl = REG & 0xF;
	GB_BY tmph = REG & 0xF0;
	REG =(tmph >> 4) | (tmpl<<4);
	SetFlag(FLAG_ZERO, REG == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, 0);

}

void Z80::RLC(GB_BY &REG) {
	int arg = REG;
	int re = (arg << 1) & 0xff;
	if ((arg&(1 << 7)) != 0) {
		re |= 1;
		SetFlag(FLAG_CARY, 1);
	}
	else {
		SetFlag(FLAG_CARY, 0);

	}
	SetFlag(FLAG_ZERO, re == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	REG = re;
}
void Z80::RL(GB_BY &REG) {
	int arg = REG;
	int re = (arg << 1) & 0xff;
	re |= GetFlag(FLAG_CARY)?1:0;
	SetFlag(FLAG_ZERO, re == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, (arg&(1<<7))!=0);
	REG = re;
}
void Z80::RRC(GB_BY &REG) {
	int arg = REG;
	int re = arg >> 1;
	if ((arg&1)==1) {
		re |= (1<<7);
		SetFlag(FLAG_CARY, 1);
	}
	else {
		SetFlag(FLAG_CARY, 0);

	}
	SetFlag(FLAG_ZERO, re == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	REG = re;

}
void Z80::RR(GB_BY &REG) {
	int arg = REG;
	int re = arg >> 1;
	re |= GetFlag(FLAG_CARY) ? 0x80 : 0;
	SetFlag(FLAG_ZERO, re == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, (arg&1)!= 0);
	REG = re;
}
void Z80::SLA(GB_BY &REG) {
	GB_BY by = 0x80 & REG;
	REG <<= 1;
	SetFlag(FLAG_ZERO, REG == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, by & 0x80);

}
void Z80::SRA(GB_BY &REG) {
	GB_BY by = 0x80 & REG;
	GB_BY c = 0x1 & REG;
	REG >>= 1;
	REG += by;
	SetFlag(FLAG_ZERO, REG == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, c & 0x1);
}
void Z80::SRL(GB_BY &REG) {
	GB_BY by = 0x1 & REG;
	REG >>= 1;
	SetFlag(FLAG_ZERO, REG == 0);
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 0);
	SetFlag(FLAG_CARY, by & 0x1);
}

void Z80::BIT(GB_BY REG, GB_BY No) {
	SetFlag(FLAG_ZERO, (~REG & (1 << No)));
	SetFlag(FLAG_NEGA, 0);
	SetFlag(FLAG_HACA, 1);
}
void Z80::RST() {
	_REG.SP -= 2;
	_Memory.MemoryWrite(_REG.SP+1 , (_REG.PC >> 8) & 0xFF);
	_Memory.MemoryWrite(_REG.SP, _REG.PC & 0xFF);
	
}
void Z80::JP() {
	_REG.PC = _Memory.MemoryRead(_REG.PC) | _Memory.MemoryRead(_REG.PC + 1) << 8;
	
}
void Z80::CALL() {
	_REG.SP -= 2;
	_Memory.MemoryWrite(_REG.SP+1, ((_REG.PC + 2) >> 8) & 0xFF);
	_Memory.MemoryWrite(_REG.SP, (_REG.PC + 2) & 0xFF);
	
	_REG.PC = _Memory.MemoryRead(_REG.PC) | _Memory.MemoryRead(_REG.PC + 1) << 8;
}
