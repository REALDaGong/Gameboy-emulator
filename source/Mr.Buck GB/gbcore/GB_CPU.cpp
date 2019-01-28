#include "GB_CPU.h"
static array<function<int()>, 0x100 * sizeof(int)> OpCode;
void CPU::Init() {

#ifdef LOG
	freopen("E:\\log.log", "w", stdout);
#endif
	reg.Init();
	CpuState = 1;
	IMEType = 0;
	IME = 0;
	cputick = 0;
	IR = 0;
	_AR = 0;
	reg1 = reg2 = 0;
	param = 0;
	SpeedState = 0;
	NextOp = NONE;
	version = 0;
	noHaltBug = 1;
#define JUMPBIO
#ifdef JUMPBIOS
	_Memory._inbios = 0;
	reg.Set16(AF, 0x01B0);
	reg.Set16(BC, 0x0013);
	reg.Set16(DE, 0x00D8);
	reg.Set16(HL, 0x014D);
	reg.Set16(SP, 0xFFFE);
	reg.SetPC(0x100);
	_Memory.MemoryWrite(0xFF26, 0xF1);
	_Memory.MemoryWrite(0xFF40, 0x91);
	_Memory.MemoryWrite(0xFF47, 0xFC);
	_Memory.MemoryWrite(0xFF48, 0xFF);
	_Memory.MemoryWrite(0xFF49, 0xFF);
	_Memory.MemoryWrite(0xFFFF, 0);
#endif // JUMPBIOS

}
void CPU::tick() {

#define CORRUPTED_STOP 0
#define RUN 1
#define INTER 2
#define STOP 3
#define CPUHALT 4
#define CB 5
#define READ 0
#define WRITE 1
#define ACCESS_MEM 6//limited usage
#define ACCESS_MEM_2 7
#define ACCESS_MEM_MODIFY 8

#define OPRAND 9
#define OPRAND_2 10
#define OPRAND_ALU 11

#define RUN_WITH_EI 12
	cputick++;
	optick++;
	if (cputick != (4 >> SpeedState))return;
	cputick = 0;

	//check Inter first
	if (NextOp==NONE&&(_Memory.MemoryRead(IE)&0x1F&_Memory.MemoryRead(IF))) {
		if (CpuState == CPUHALT || CpuState == STOP)
		{
			CpuState = RUN;
		}
		if (IME) {
			IME = 0;
			IMEType = _Memory.MemoryRead(IE) & _Memory.MemoryRead(IF);
			CpuState = INTER;
			//it seems that a INTR transfer procedure will spend 20clks
			//and it will stop the timer for a while(?)

			//in fact,interrupts can be stopped during the transfer procedure.
			//
		}
	}

	switch (CpuState) {
	case CORRUPTED_STOP: {
		return;
	}break;
	case OPRAND_2: {

		reg.Set8(reg1 + 1, _Memory.MemoryRead(reg.GetPC()));

		reg.IncPC();
		CpuState = OPRAND;
	}break;
	case OPRAND: {
		reg.Set8(reg1, _Memory.MemoryRead(reg.GetPC()));
		reg.IncPC();
		CpuState = RUN;
		NextOp = OpCode[NextOp]();
	}break;
	case OPRAND_ALU: {
		reg.Set8(reg2, _Memory.MemoryRead(reg.GetPC()));
		reg.IncPC();
		CpuState = RUN;
		NextOp = OpCode[NextOp]();
	}break;
	case ACCESS_MEM_MODIFY: {
		reg.Set8(reg2, _Memory.MemoryRead(_AR));
		NextOp = OpCode[NextOp]();
		dir = WRITE;
		CpuState = ACCESS_MEM;
	}break;
	case ACCESS_MEM_2: {

		reg.Set8(reg2, _Memory.MemoryRead(_AR));
		_AR++;
		CpuState = ACCESS_MEM;
	}break;
	case ACCESS_MEM: {
		if (dir == READ) {
			reg.Set8(reg1, _Memory.MemoryRead(_AR));
			NextOp = OpCode[NextOp]();
		}
		else
			_Memory.MemoryWrite(_AR, reg.Get8(DR));
		CpuState = RUN;
	}break;
	case RUN: {
		if (NextOp == NONE) {
#ifdef LOG
			static int startLogging = 0;
			if (reg.GetPC() > 0x100)startLogging = 1;
			if(startLogging)reg.output();
			
			
#endif //LOG
		if (reg.GetPC() == 0xffc5) {
				int breakpoint = 1;
		}
			optick = 0;
			IR = fetch();
		}
		if (IR == 0xCB) {
			CpuState = CB;
			break;
		}
		if(CpuState==RUN||(CpuState==ACCESS_MEM&&dir==WRITE))
			NextOp = OpCode[NextOp]();
		
	}break;
	case RUN_WITH_EI: {
		CpuState = RUN;
		optick = 0;
		IR = fetch();
		
		if (IR == 0xCB) {
			CpuState = CB;
			break;
		}
		
		if (CpuState == RUN || (CpuState == ACCESS_MEM && dir == WRITE))
			NextOp = OpCode[NextOp]();
		IME = 1;
		
	}break;
	case CB: {
		
		IR = fetchCB();
		
		if (CpuState == CB) {
			NextOp = OpCode[NextOp]();
			CpuState = RUN;
		}
	}break;
	case INTER: {
		static int InterStep = 0;
		switch (InterStep) {
		case 0:case 1:break;
		case 2:reg.DecSP(); _Memory.MemoryWrite(reg.GetSP(), (reg.GetPC() & 0xFF00) >> 8);  break;
		case 3:reg.DecSP(); _Memory.MemoryWrite(reg.GetSP(), reg.GetPC() & 0xFF); break;
		case 4:InterHandle(IMEType); CpuState = RUN; InterStep = -1; IMEType = 0; break;
		}
		InterStep++;
	}break;
	case STOP: {
		return;
	}break;
	case CPUHALT: {
		return;
	}break;
	}
}

GB_BY CPU::fetch() {
	GB_BY op = _Memory.MemoryRead(reg.GetPC());
	if (noHaltBug) {
		reg.IncPC();
		
	}
	else {

		noHaltBug = 1;
	}
	if (op == 0x00) {//NOP
		NextOp = NOP;
		return op;
	}
	if (op == 0xCB) {//CB
		NextOp = NOP;
		return op;
	}
	if (op == 0xF3) {//DI
		NextOp = NOP;
		IME = 0;
		return op;
	}
	if (op == 0xFB) {//EI
		NextOp = EI;
		return op;
	}
	if (op == 0x76) {//HALT
		CpuState = CPUHALT;
		if (IME == 0&&version!=2&&(_Memory.MemoryRead(IE)&0x1F&_Memory.MemoryRead(IF))) {//HALT bug,no such behaviour in CGB.
			noHaltBug = 0;

		}
		return NONE;
	}
	if (op == 0x10) {
		CpuState = STOP;
		reg.IncPC();
		return NONE;
	}
	if (op < 0x40) {
		switch (op & 0xF) {
		case 0x0: {
			NextOp = JR;
			if (op == 0x20)
				param = FLAG_NZ;
			else
				param = FLAG_NC;
			CpuState = OPRAND;
			reg1 = TMPH;
		}break;
		case 0x1: {
			NextOp = LD16;
			CpuState = OPRAND_2;
			switch (op) {
			case 0x01:reg1 = BC; break;
			case 0x11:reg1 = DE; break;
			case 0x21:reg1 = HL; break;
			case 0x31:reg1 = SP; break;
			}
		}break;
		case 0x2: {
			NextOp = LD8;
			CpuState = ACCESS_MEM;
			dir = WRITE;
			reg1 = DR;
			reg2 = A;
			switch (op) {
			case 0x02:_AR = reg.Get16(BC); break;
			case 0x12:_AR = reg.Get16(DE); break;
			case 0x22:_AR = reg.Get16(HL); reg.IncHL(); break;
			case 0x32:_AR = reg.Get16(HL); reg.DecHL(); break;//need ++ or -- HL
			}

		}break;
		case 0x3: {
			NextOp = INC16;
			switch (op) {
			case 0x03:reg1 = BC; break;
			case 0x13:reg1 = DE; break;
			case 0x23:reg1 = HL; break;
			case 0x33:reg1 = SP; break;
			}
		}break;
		case 0x4: {
			NextOp = INC8;
			switch (op) {
			case 0x04:reg1 = B; break;
			case 0x14:reg1 = D; break;
			case 0x24:reg1 = H; break;
			case 0x34:reg1 = DR;
				reg2 = DR;
				CpuState = ACCESS_MEM_MODIFY;
				_AR = reg.Get16(HL);
				break;
			}
		}break;
		case 0x5: {
			NextOp = DEC8;
			switch (op) {
			case 0x05:reg1 = B; break;
			case 0x15:reg1 = D; break;
			case 0x25:reg1 = H; break;
			case 0x35:reg1 = DR;
				reg2 = DR;
				CpuState = ACCESS_MEM_MODIFY;
				_AR = reg.Get16(HL);
				break;
			}
		}break;
		case 0x6: {

			CpuState = OPRAND;
			switch (op) {
			case 0x06:reg1 = B; break;
			case 0x16:reg1 = D; break;
			case 0x26:reg1 = H; break;
			case 0x36:reg1 = DR; CpuState = OPRAND; NextOp = LD8_MEM; break;
			}
		}break;
		case 0x7: {
			switch (op) {
			case 0x07:NextOp = RLCA; break;
			case 0x17:NextOp = RLA; break;
			case 0x27:NextOp = DAA; break;
			case 0x37:NextOp = SCF; break;
			}
		}break;
		case 0x8: {
			switch (op) {
			case 0x08: NextOp = LDa16SP; break;
			case 0x18: param = NONE; NextOp = JR;
				CpuState = OPRAND;
				reg1 = TMPH; break;
			case 0x28: param = FLAG_Z; NextOp = JR;
				CpuState = OPRAND;
				reg1 = TMPH; break;
			case 0x38: param = FLAG_C; NextOp = JR;
				CpuState = OPRAND;
				reg1 = TMPH; break;

			}
		}break;
		case 0x9: {
			NextOp = ADDHL;
			reg1 = HL;
			switch (op) {
			case 0x09: reg2 = BC; break;
			case 0x19: reg2 = DE; break;
			case 0x29: reg2 = HL; break;
			case 0x39: reg2 = SP; break;
			}
		}break;
		case 0xA: {
			NextOp = LDA;
			reg2=reg1 = DR;
			dir = READ;
			CpuState = ACCESS_MEM;
			switch (op) {
			case 0x0A:_AR = reg.Get16(BC); break;
			case 0x1A:_AR = reg.Get16(DE); break;
			case 0x2A:_AR = reg.Get16(HL); reg.IncHL(); break;
			case 0x3A:_AR = reg.Get16(HL); reg.DecHL(); break;
			}
		}break;
		case 0xB: {
			NextOp = DEC16;
			switch (op) {
			case 0x0B:reg1 = BC; break;
			case 0x1B:reg1 = DE; break;
			case 0x2B:reg1 = HL; break;
			case 0x3B:reg1 = SP; break;
			}
		}break;
		case 0xC: {
			NextOp = INC8;
			switch (op) {
			case 0x0C:reg1 = C; break;
			case 0x1C:reg1 = E; break;
			case 0x2C:reg1 = L; break;
			case 0x3C:reg1 = A; break;
			}
		}break;
		case 0xD: {
			NextOp = DEC8;
			switch (op) {
			case 0x0D:reg1 = C; break;
			case 0x1D:reg1 = E; break;
			case 0x2D:reg1 = L; break;
			case 0x3D:reg1 = A; break;
			}
		}break;
		case 0xE: {

			CpuState = OPRAND;
			switch (op) {
			case 0x0E:reg1 = C; break;
			case 0x1E:reg1 = E; break;
			case 0x2E:reg1 = L; break;
			case 0x3E:reg1 = A; break;
			}
		}break;
		case 0xF: {
			switch (op) {
			case 0x0F:NextOp = RRCA; break;
			case 0x1F:NextOp = RRA; break;
			case 0x2F:NextOp = CPL; break;
			case 0x3F:NextOp = CCF; break;
			}
		}break;
		}
	}
	else if (op < 0xC0) {
		switch (op & 0xF) {
		case 0x0:reg2 = B;  break;
		case 0x1:reg2 = C;  break;
		case 0x2:reg2 = D;  break;
		case 0x3:reg2 = E; break;
		case 0x4:reg2 = H; break;
		case 0x5:reg2 = L; break;
		case 0x6:reg2 = DR; CpuState = ACCESS_MEM; dir = READ; _AR = reg.Get16(HL); break;//1.
		case 0x7:reg2 = A; break;
		case 0x8:reg2 = B; break;
		case 0x9:reg2 = C; break;
		case 0xA:reg2 = D; break;
		case 0xB:reg2 = E; break;
		case 0xC:reg2 = H; break;
		case 0xD:reg2 = L; break;
		case 0xE:reg2 = DR; CpuState = ACCESS_MEM; dir = READ; _AR = reg.Get16(HL); break;
		case 0xF:reg2 = A; break;
		}
		if (op < 0x80) {
			if ((op & 0xF) != 0x6 && (op & 0xF) != 0xE)NextOp = LD8;
			else NextOp = NONE;
			if ((op & 0xF) < 0x8) {
				switch (op & 0xF0) {
				case 0x40:reg1 = B; break;
				case 0x50:reg1 = D; break;
				case 0x60:reg1 = H; break;
				case 0x70:reg1 = DR; CpuState = ACCESS_MEM; dir = WRITE; _AR = reg.Get16(HL); break;
				}
			}
			else {
				switch (op & 0xF0) {
				case 0x40:reg1 = C; break;
				case 0x50:reg1 = E; break;
				case 0x60:reg1 = L; break;
				case 0x70:reg1 = A; break;
				}
			}
		}
		else {
			reg1 = DR;
			if ((op & 0xF) < 0x8) {
				switch (op & 0xF0) {
				case 0x80:NextOp = ADD; break;
				case 0x90:NextOp = SUB; break;
				case 0xA0:NextOp = AND; break;
				case 0xB0:NextOp = OR;  break;
				}
			}
			else {
				switch (op & 0xF0) {
				case 0x80:NextOp = ADC; break;
				case 0x90:NextOp = SBC; break;
				case 0xA0:NextOp = XOR; break;
				case 0xB0:NextOp = CP;  break;
				}
			}
		}
	}
	else {
		switch (op) {
		case 0xC0: NextOp = RET; param = FLAG_NZ; break;
		case 0xD0: NextOp = RET; param = FLAG_NC; break;
		case 0xE0: NextOp = LDHa8A; CpuState = OPRAND; reg1 = TMPH; break;
		case 0xF0: NextOp = LDHAa8; CpuState = OPRAND; reg1 = TMPH; break;
		case 0xC1: NextOp = POP; reg1 = BC; break;
		case 0xD1: NextOp = POP; reg1 = DE; break;
		case 0xE1: NextOp = POP; reg1 = HL; break;
		case 0xF1: NextOp = POP; reg1 = AF; break;
		case 0xC2: NextOp = JP; param = FLAG_NZ; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xD2: NextOp = JP; param = FLAG_NC; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xE2: NextOp = NONE; reg.Set8(DR, reg.Get8(A)); dir = WRITE; _AR = 0xFF00 | reg.Get8(C); CpuState = ACCESS_MEM; break;
		case 0xF2: NextOp = NONE; reg1 = A; dir = READ; _AR = 0xFF00 | reg.Get8(C); CpuState = ACCESS_MEM; break;
		case 0xC3: NextOp = JP; param = NONE; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xC4: NextOp = CALL; param = FLAG_NZ; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xD4: NextOp = CALL; param = FLAG_NC; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xC5: NextOp = PUSH; reg1 = BC; break;
		case 0xD5: NextOp = PUSH; reg1 = DE; break;
		case 0xE5: NextOp = PUSH; reg1 = HL; break;
		case 0xF5: NextOp = PUSH; reg1 = AF; break;
		case 0xC6: NextOp = ADD; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xD6: NextOp = SUB; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xE6: NextOp = AND; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xF6: NextOp = OR; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xC7: NextOp = RST; param = 0x00; break;
		case 0xD7: NextOp = RST; param = 0x10; break;
		case 0xE7: NextOp = RST; param = 0x20; break;
		case 0xF7: NextOp = RST; param = 0x30; break;
		case 0xC8: NextOp = RET; param = FLAG_Z; break;
		case 0xD8: NextOp = RET; param = FLAG_C; break;
		case 0xE8: NextOp = ADDSP_DELAY; CpuState = OPRAND; reg1 = TMPH; break;
		case 0xF8: NextOp = LDHLSP; CpuState = OPRAND; reg1 = TMPH; break;
		case 0xC9: NextOp = RET_NO_CC; param = NONE; break;
		case 0xD9: NextOp = RETI; break;
		case 0xE9: NextOp = JPHL; break;
		case 0xF9: NextOp = LDSPHL; break;
		case 0xCA: NextOp = JP; param = FLAG_Z; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xDA: NextOp = JP; param = FLAG_C; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xEA: NextOp = LDHa16A; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xFA: NextOp = LDHAa16; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xCC: NextOp = CALL; param = FLAG_Z; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xDC: NextOp = CALL; param = FLAG_C; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xCD: NextOp = CALL; param = NONE; CpuState = OPRAND_2; reg1 = TMP; break;
		case 0xCE: NextOp = ADC; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xDE: NextOp = SBC; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xEE: NextOp = XOR; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xFE: NextOp = CP; reg2 = TMPL; CpuState = OPRAND_ALU; break;
		case 0xCF: NextOp = RST; param = 0x08; break;
		case 0xDF: NextOp = RST; param = 0x18; break;
		case 0xEF: NextOp = RST; param = 0x28; break;
		case 0xFF: NextOp = RST; param = 0x38; break;
		default:
			NextOp = NONE;
			CpuState = CORRUPTED_STOP;//invalid opcodes
		}
	}
	return op;
}
GB_BY CPU::fetchCB() {
	GB_BY op = _Memory.MemoryRead(reg.GetPC());
	reg.IncPC();
	switch (op & 0xF) {
	case 0x0:reg2 = B; break;
	case 0x1:reg2 = C; break;
	case 0x2:reg2 = D; break;
	case 0x3:reg2 = E; break;
	case 0x4:reg2 = H; break;
	case 0x5:reg2 = L; break;
	case 0x6:reg2 = DR; _AR = reg.Get16(HL); CpuState = ACCESS_MEM_MODIFY;  break;//1.
	case 0x7:reg2 = A; break;
	case 0x8:reg2 = B; break;
	case 0x9:reg2 = C; break;
	case 0xA:reg2 = D; break;
	case 0xB:reg2 = E; break;
	case 0xC:reg2 = H; break;
	case 0xD:reg2 = L; break;
	case 0xE:reg2 = DR; _AR = reg.Get16(HL); CpuState = ACCESS_MEM_MODIFY;  break;
	case 0xF:reg2 = A; break;
	}
	if (op < 0x40) {
		if ((op & 0xF) < 0x8) {
			switch (op & 0xF0) {
			case 0x00:NextOp = RLC; break;
			case 0x10:NextOp = RL; break;
			case 0x20:NextOp = SLA; break;
			case 0x30:NextOp = SWAP; break;
			}
		}
		else {
			switch (op & 0xF0) {
			case 0x00:NextOp = RRC; break;
			case 0x10:NextOp = RR; break;
			case 0x20:NextOp = SRA; break;
			case 0x30:NextOp = SRL; break;
			}
		}
	}
	else {//BIT consume 4 clocks less than regular CB ins,because there is no need to writeback;
		if (op < 0x80) {
			if (((op & 0xF) == 0x6) || ((op & 0xF) == 0xE)) {
				CpuState = ACCESS_MEM;
				dir = READ;
				reg1 = DR;
			}
			NextOp = BIT;
			param = (op - 0x40) / 8;
		}
		else if (op < 0xC0) {
			NextOp = RES;
			param = (op - 0x80) / 8;
		}
		else {
			NextOp = SET;
			param = (op - 0xC0) / 8;
		}
	}
	return op;
}
void CPU::BuildOps() {
	OpCode[EI] = [&]()->int {CpuState = RUN_WITH_EI; return NONE; };
	OpCode[NONE] = [&]()->int {return NONE; };
	OpCode[NOP] = [&]()->int {return NONE; };
	OpCode[DELAY] = [&]()->int {return NONE; };
	OpCode[LD8] = [&]()->int {reg.Set8(reg1, reg.Get8(reg2)); return NONE; };
	OpCode[LD16] = [&]()->int {return NONE; };
	OpCode[LDSPHL] = [&]()->int {reg.Set8(SPL, reg.Get8(L)); return LDSPHL_2; };
	OpCode[LDSPHL_2] = [&]()->int {reg.Set8(SPH, reg.Get8(H)); return NONE; };
	OpCode[LDA] = [&]()->int {reg.Set8(A, reg.Get8(reg2)); return NONE; };
	OpCode[LDHa8A] = [&]()->int {CpuState = ACCESS_MEM; dir = WRITE; _AR = 0xFF00 | reg.Get8(TMPH); reg.Set8(DR, reg.Get8(A)); return NONE; };
	OpCode[LDHAa8] = [&]()->int {CpuState = ACCESS_MEM; dir = READ; _AR = 0xFF00 | reg.Get8(TMPH); reg1 = A; return NONE; };
	OpCode[LDHa16A] = [&]()->int {CpuState = ACCESS_MEM; dir = WRITE; _AR = reg.Get16(TMP); reg.Set8(DR, reg.Get8(A)); return NONE; };
	OpCode[LDHAa16] = [&]()->int {CpuState = ACCESS_MEM; dir = READ; _AR = reg.Get16(TMP); reg1 = A; return NONE; };
	OpCode[JR] = [&]()->int {if (reg.TestFlag(param))return JR_JUMP; else return NONE; };
	OpCode[JR_JUMP] = [&]()->int {reg.SetPC(reg.GetPC() + (__int8)reg.Get8(TMPH)); return NONE; };
	OpCode[JP] = [&]()->int {if (reg.TestFlag(param))return JP_JUMP; else return NONE; };
	OpCode[JP_JUMP] = [&]()->int {reg.SetPC(reg.Get16(TMP)); return NONE; };
	OpCode[JPHL] = [&]()->int {reg.SetPC(reg.Get16(HL)); return NONE; };
	OpCode[CALL_2] = [&]()->int {return CALL_JUMP; };
	OpCode[CALL] = [&]()->int {if (reg.TestFlag(param))return CALL_2; else return NONE; };
	OpCode[CALL_JUMP] = [&]()->int {reg.DecSP(); _Memory.MemoryWrite(reg.Get16(SP), (reg.GetPC() & 0xFF00) >> 8);  reg.SetPCHigh(reg.Get8(TMPH)); return CALL_JUMP_2; };
	OpCode[CALL_JUMP_2] = [&]()->int {reg.DecSP(); _Memory.MemoryWrite(reg.Get16(SP), (reg.GetPC() & 0xFF)); reg.SetPCLow(reg.Get8(TMPL)); return NONE; };
	OpCode[RET] = [&]()->int {return RET_2; };
	OpCode[RET_2] = [&]()->int {if (reg.TestFlag(param))return RET_JUMP; else return NONE; };
	OpCode[RET_JUMP] = [&]()->int {reg.SetPCLow(_Memory.MemoryRead(reg.Get16(SP))); reg.IncSP();  return RET_JUMP_2; };
	OpCode[RET_JUMP_2] = [&]()->int { reg.SetPCHigh(_Memory.MemoryRead(reg.Get16(SP))); reg.IncSP(); return RET_JUMP_3; };
	OpCode[RET_JUMP_3] = [&]()->int { return NONE; };
	OpCode[RET_NO_CC] = [&]()->int {return RET_JUMP; };
	OpCode[RETI] = [&]()->int {IME = 1; return RET_JUMP; };
	OpCode[RST] = [&]()->int {return RST_2; };
	OpCode[RST_2] = [&]()->int {reg.Set8(TMPH, 0); reg.Set8(TMPL, param & 0xFF); return CALL_JUMP; };
	OpCode[POP] = [&]()->int { return POP_2; };
	OpCode[POP_2] = [&]()->int { reg.Set8(reg1 + 1, _Memory.MemoryRead(reg.Get16(SP))); reg.IncSP(); return POP_3; };
	OpCode[POP_3] = [&]()->int { reg.Set8(reg1, _Memory.MemoryRead(reg.Get16(SP))); reg.IncSP(); return NONE; };
	OpCode[PUSH] = [&]()->int {return PUSH_2; };
	OpCode[PUSH_2] = [&]()->int {_AR = reg.Get16(SP)-1; return PUSH_3; };
	OpCode[PUSH_3] = [&]()->int { reg.DecSP(); _Memory.MemoryWrite(_AR, reg.Get8(reg1)); return PUSH_4; };
	OpCode[PUSH_4] = [&]()->int { reg.DecSP(); _Memory.MemoryWrite(_AR - 1, reg.Get8(reg1 + 1)); return NONE; };
	OpCode[ADDSP_DELAY] = [&]()->int {return ADDSP; };
	OpCode[ADDSP] = [&]()->int {

		__int8 by = (__int8)reg.Get8(TMPH);
		GB_DB regSP = reg.Get16(SP);
		int re = regSP + by;
		if ((((regSP & 0x0F) + (by & 0x0F)) & 0x10) != 0)
			reg.SetFlag(FLAG_HACA);
		else
			reg.ResetFlag(FLAG_HACA);
		if ((((regSP & 0xFF) + (by & 0xFF)) & 0x100) != 0)
			reg.SetFlag(FLAG_CARY);
		else
			reg.ResetFlag(FLAG_CARY);

		reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.Set8(SPL, re & 0xff);
		param = re;
		return ADDSP_2;
	};
	OpCode[ADDSP_2] = [&]()->int {reg.Set8(SPH, (param & 0xFF00)>>8); return NONE; };
	OpCode[LDHLSP] = [&]()->int {
		__int8 by = (__int8)reg.Get8(TMPH);
		GB_DB regSP = reg.Get16(SP);
		int re = regSP + by;
		if ((((regSP & 0x0F) + (by & 0x0F)) & 0x10) != 0)
			reg.SetFlag(FLAG_HACA);
		else
			reg.ResetFlag(FLAG_HACA);
		if ((((regSP & 0xFF) + (by & 0xFF)) & 0x100) != 0)
			reg.SetFlag(FLAG_CARY);
		else
			reg.ResetFlag(FLAG_CARY);

		reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.Set8(L, re & 0xff);
		param = re;
		return LDHLSP_2;
	};
	OpCode[LDHLSP_2] = [&]()->int {reg.Set8(H, (param & 0xFF00)>>8); return NONE; };
	OpCode[INC16] = [&]()->int {GB_BY tmp = reg.Get8(reg1 + 1); if (tmp == 0xFF)param = 1; else param = 0; reg.Set8(reg1 + 1, tmp + 1); return INC16_2; };
	OpCode[INC16_2] = [&]()->int {if (param)reg.Set8(reg1, reg.Get8(reg1) + 1); return NONE; };
	OpCode[DEC16] = [&]()->int {GB_BY tmp = reg.Get8(reg1 + 1); if (tmp == 0x00)param = 1; else param = 0; reg.Set8(reg1 + 1, tmp - 1); return DEC16_2; };
	OpCode[DEC16_2] = [&]()->int {if (param)reg.Set8(reg1, reg.Get8(reg1) - 1); return NONE; };
	OpCode[INC8] = [&]()->int {GB_BY tmp = reg.Get8(reg1) + 1; reg.Set8(reg1, tmp); reg.ResetFlag(FLAG_NEGA); if (tmp == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO); if ((tmp ^ (tmp - 1)) & 0x10)reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA); return NONE; };
	OpCode[DEC8] = [&]()->int {GB_BY tmp = reg.Get8(reg1) - 1; reg.Set8(reg1, tmp); reg.SetFlag(FLAG_NEGA); if (tmp == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO); if ((tmp ^ (tmp + 1)) & 0x10)reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA); return NONE; };
	OpCode[RLCA] = [&]()->int {
		GB_BY tmp = reg.Get8(A);
		GB_BY by = 0x80 & tmp;
		tmp <<= 1;
		tmp |= by >> 7;
		reg.Set8(A, tmp);
		reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		if (by)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		return NONE;
	};
	OpCode[RLA] = [&]()->int {
		GB_BY tmp = reg.Get8(A);
		GB_BY by = 0x80 & tmp;

		
		tmp <<= 1;
		tmp |= reg.TestFlag(FLAG_CARY) ? 1 : 0;
		reg.Set8(A, tmp);
		if (by)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		return NONE;
	};
	OpCode[DAA] = [&]()->int {
		GB_DB tmp = reg.Get8(A);
		if (reg.TestFlag(FLAG_NEGA)) {
			if (reg.TestFlag(FLAG_HACA)) {
				tmp = (tmp - 0x06) & 0xff;
			}
			if (reg.TestFlag(FLAG_CARY)) {
				tmp = (tmp - 0x60) & 0xff;
			}
		}
		else {
			if (reg.TestFlag(FLAG_HACA) || (tmp & 0xf) > 9) {
				tmp += 0x06;
			}
			if (reg.TestFlag(FLAG_CARY) || tmp> 0x9f) {
				tmp += 0x60;
			}
		}
		reg.ResetFlag(FLAG_HACA);
		if (tmp > 0xff) {
			reg.SetFlag(FLAG_CARY);
		}
		tmp &= 0xff;
		if (tmp == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		reg.Set8(A, tmp);
		return NONE;
	};
	OpCode[SCF] = [&]()->int {
		reg.SetFlag(FLAG_CARY);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		return NONE;
	};
	OpCode[RRCA] = [&]()->int {
		GB_BY tmp = reg.Get8(A);
		GB_BY by = 0x1 & tmp;
		tmp >>= 1;
		tmp |= by << 7;
		reg.Set8(A, tmp);
		reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		if (by)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		return NONE;
	};
	OpCode[RRA] = [&]()->int {
		GB_BY tmp = reg.Get8(A);
		GB_BY by = 0x1 & tmp;

		
		tmp >>= 1;
		tmp |= reg.TestFlag(FLAG_CARY) ? 0x80 : 0;
		reg.Set8(A, tmp);
		if (by)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		return NONE;
	};
	OpCode[CPL] = [&]()->int {reg.SetFlag(FLAG_NEGA); reg.SetFlag(FLAG_HACA); reg.Set8(A, ~reg.Get8(A)); return NONE; };
	OpCode[CCF] = [&]()->int {reg.ResetFlag(FLAG_NEGA); reg.ResetFlag(FLAG_HACA); if (reg.TestFlag(FLAG_CARY)) reg.ResetFlag(FLAG_CARY); else reg.SetFlag(FLAG_CARY); return NONE; };
	OpCode[LD8_MEM] = [&]()->int {CpuState = ACCESS_MEM; dir = WRITE; _AR = reg.Get16(HL);return NONE; };
	OpCode[LDa16SP] = [&]()->int {return LDa16SP_2; };
	OpCode[LDa16SP_2] = [&]()->int {_AR = _Memory.MemoryRead(reg.GetPC()); reg.IncPC(); return LDa16SP_3; };
	OpCode[LDa16SP_3] = [&]()->int {_AR |= (GB_DB)(_Memory.MemoryRead(reg.GetPC()) << 8); reg.IncPC(); return LDa16SP_4; };
	OpCode[LDa16SP_4] = [&]()->int {_Memory.MemoryWrite(_AR, reg.Get8(SPL)); _AR++; return LDa16SP_5; };
	OpCode[LDa16SP_5] = [&]()->int {_Memory.MemoryWrite(_AR, reg.Get8(SPH)); return NONE; };

	OpCode[ADDHL] = [&]()->int {
		GB_DB hl = (reg.Get8(H) << 8) + reg.Get8(L);
		GB_DB db = (reg.Get8(reg2) << 8) + reg.Get8(reg2 + 1);
		uint32_t re = hl + db;
		reg.ResetFlag(FLAG_NEGA);
		if (((hl & 0x0fff) + (db & 0x0fff)) > 0x0fff)reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA);
		if (re> 0xffff) reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		reg.Set8(H, (re & 0xFF00) >> 8);
		param = re & 0xFF;
		return ADDHL_2;
	};
	OpCode[ADDHL_2] = [&]()->int {
		reg.Set8(L, param);
		return NONE;
	};
	OpCode[ADD] = [&]()->int {
		GB_BY regA = reg.Get8(A);
		GB_BY regb = reg.Get8(reg2);
		reg.ResetFlag(FLAG_NEGA);
		GB_DB re = regA + regb;
		if ((re & 0xff) == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		if (((regA & 0x0F) + (regb & 0x0F)) & 0x10)reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA);
		if (re > 0xff)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		reg.Set8(A, re & 0xFF);
		return NONE;
	};
	OpCode[ADC] = [&]()->int {
		reg.ResetFlag(FLAG_NEGA);
		GB_BY regA = reg.Get8(A);
		GB_BY regb = reg.Get8(reg2);
		GB_DB re = regA + regb + (reg.TestFlag(FLAG_CARY) ? 1 : 0);
		if ((re & 0xFF) == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		if (((regA&0x0F) + (regb&0x0F) + (reg.TestFlag(FLAG_CARY) ? 1 : 0)) & 0x10)reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA);
		if (re > 0xFF)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		reg.Set8(A, re & 0xFF);
		return NONE;
	};
	OpCode[SUB] = [&]()->int {
		reg.SetFlag(FLAG_NEGA);
		GB_BY regA = reg.Get8(A);
		GB_BY regb = reg.Get8(reg2);
		GB_DB re = regA - regb;
		if ((re & 0xFF) == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		if ((regA & 0x0F) < (regb & 0x0F))reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA);
		if (regA < regb)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		reg.Set8(A, re & 0xFF);
		return NONE;
	};
	OpCode[SBC] = [&]()->int {
		reg.SetFlag(FLAG_NEGA);
		GB_BY regA = reg.Get8(A);
		GB_BY regb = reg.Get8(reg2);
		GB_DB re = regA - regb - reg.TestFlag(FLAG_CARY);
		if ((re & 0xFF) == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		if (((regA ^ regb ^ (re & 0xff)) & (1 << 4)) != 0)reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA);
		if (re & 0x100)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		reg.Set8(A, re & 0xFF);
		return NONE;
	};
	OpCode[AND] = [&]()->int {
		GB_BY regA = reg.Get8(A);
		GB_BY re = regA & reg.Get8(reg2);
		reg.Set8(A, re);
		if (re == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.SetFlag(FLAG_HACA);
		reg.ResetFlag(FLAG_CARY);
		return NONE;
	};
	OpCode[XOR] = [&]()->int {
		GB_BY regA = reg.Get8(A);
		GB_BY re = regA ^ reg.Get8(reg2);
		reg.Set8(A, re);
		if (re == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		reg.ResetFlag(FLAG_CARY);
		return NONE;
	};
	OpCode[OR] = [&]()->int {
		GB_BY regA = reg.Get8(A);
		GB_BY re = regA | reg.Get8(reg2);
		reg.Set8(A, re);
		if (re == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		reg.ResetFlag(FLAG_CARY);
		return NONE;
	};
	OpCode[CP] = [&]()->int {
		reg.SetFlag(FLAG_NEGA);
		GB_BY regA = reg.Get8(A);
		GB_BY regb = reg.Get8(reg2);
		GB_DB re = regA - regb;
		if ((re & 0xFF) == 0)reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		if ((regA & 0x0F) < (regb & 0x0F))reg.SetFlag(FLAG_HACA); else reg.ResetFlag(FLAG_HACA);
		if (regA < regb)reg.SetFlag(FLAG_CARY); else reg.ResetFlag(FLAG_CARY);
		return NONE;
	};
	OpCode[RLC] = [&]()->int {
		int arg = reg.Get8(reg2);
		int re = (arg << 1) & 0xff;
		if ((arg&(1 << 7)) != 0) {
			re |= 1;
			reg.SetFlag(FLAG_CARY);
		}
		else {
			reg.ResetFlag(FLAG_CARY);

		}
		if (re == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		reg.Set8(reg2, re & 0xFF);
		return NONE;
	};
	OpCode[RRC] = [&]()->int {
		int arg = reg.Get8(reg2);
		int re = arg >> 1;
		if ((arg & 1) == 1) {
			re |= (1 << 7);
			reg.SetFlag(FLAG_CARY);
		}
		else {
			reg.ResetFlag(FLAG_CARY);

		}
		if (re == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		reg.Set8(reg2, re & 0xFF);
		return NONE;
	};
	OpCode[RL] = [&]()->int {
		int arg = reg.Get8(reg2);
		int re = (arg << 1) & 0xff;
		re |= reg.TestFlag(FLAG_CARY) ? 1 : 0;
		if (re == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		if ((arg&(1 << 7)) != 0)
			reg.SetFlag(FLAG_CARY);
		else
			reg.ResetFlag(FLAG_CARY);
		reg.Set8(reg2, re & 0xFF);
		return NONE;
	};
	OpCode[RR] = [&]()->int {
		int arg = reg.Get8(reg2);
		int re = arg >> 1;
		re |= reg.TestFlag(FLAG_CARY) ? 0x80 : 0;
		if (re == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		if ((arg & 1) != 0)
			reg.SetFlag(FLAG_CARY);
		else
			reg.ResetFlag(FLAG_CARY);
		reg.Set8(reg2, re & 0xFF);
		return NONE;
	};
	OpCode[SLA] = [&]()->int {
		GB_BY REG = reg.Get8(reg2);
		GB_BY by = 0x80 & REG;
		REG <<= 1;
		if (REG == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		if (by)
			reg.SetFlag(FLAG_CARY);
		else
			reg.ResetFlag(FLAG_CARY);
		reg.Set8(reg2, REG);
		return NONE;
	};
	OpCode[SRA] = [&]()->int {
		GB_BY REG = reg.Get8(reg2);
		GB_BY by = 0x80 & REG;
		GB_BY c = 0x1 & REG;
		REG >>= 1;
		REG += by;
		if (REG == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		if (c)
			reg.SetFlag(FLAG_CARY);
		else
			reg.ResetFlag(FLAG_CARY);
		reg.Set8(reg2, REG);
		return NONE;
	};
	OpCode[SWAP] = [&]()->int {
		GB_BY REG = reg.Get8(reg2);
		GB_BY tmpl = REG & 0xF;
		GB_BY tmph = REG & 0xF0;
		REG = (tmph >> 4) | (tmpl << 4);
		if (REG == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		reg.ResetFlag(FLAG_CARY);
		reg.Set8(reg2, REG);
		return NONE;
	};
	OpCode[SRL] = [&]()->int {
		GB_BY REG = reg.Get8(reg2);
		GB_BY by = 0x1 & REG;
		REG >>= 1;
		if (REG == 0)
			reg.SetFlag(FLAG_ZERO);
		else
			reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.ResetFlag(FLAG_HACA);
		if (by)
			reg.SetFlag(FLAG_CARY);
		else
			reg.ResetFlag(FLAG_CARY);
		reg.Set8(reg2, REG);
		return NONE;
	};
	OpCode[BIT] = [&]()->int {
		if ((~reg.Get8(reg2) & (1 << param)))reg.SetFlag(FLAG_ZERO); else reg.ResetFlag(FLAG_ZERO);
		reg.ResetFlag(FLAG_NEGA);
		reg.SetFlag(FLAG_HACA);
		return NONE;
	};
	OpCode[RES] = [&]()->int {
		reg.Set8(reg2, reg.Get8(reg2)&~(1 << param));
		return NONE;
	};
	OpCode[SET] = [&]()->int {
		reg.Set8(reg2, reg.Get8(reg2) | (1 << param));
		return NONE;
	};
}
void CPU::InterHandle(GB_BY IMEtype) {


	
	if (IMEtype & 0x01) {
		_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x1));
		reg.SetPC(0x40);
	}//V-BLANK
	else
		if (IMEtype & 0x02) {
			_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x2));
			reg.SetPC(0x48);
		}//LCDC (see STAT)
		else
			if (IMEtype & 0x04) {
				_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x4));
				reg.SetPC(0x50);
			}//timer overflow
			else
				if (IMEtype & 0x08) {
					_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x8));
					reg.SetPC(0x58);
				}//serial io transfer complete
				else
					if (IMEtype & 0x10) {
						_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF)&(~0x10));
						reg.SetPC(0x60);
					}//transition from high to low of pin number p10-p13
					else {
						reg.SetPC(0);//cancell inter.
					}
}