#pragma once
#include "GB.h"
#include "GB_CART.h"
#include "GB_MEMORY.h"
#include "GB_CPU.h"

//for gui.
//key event
#define KY_A 0
#define KY_B 1
#define KY_UP 2
#define KY_DOWN 3
#define KY_LEFT 4
#define KY_RIGHT 5
#define KY_START 6
#define KY_SELECT 7

#define KY_RELESE 0
#define KY_PRESS 1

typedef uint8_t KEYNAME;
typedef uint8_t KEYMOVE;

namespace GBCore {
	//should obey this order.
	
	
	Memory memory;
	Timer timer(&memory);
	APU apu;
	GPU gpu(memory);
	
	
	Z80 cpu(memory, gpu, timer);
}

const GB_BY(*const GameBoyScreen)[SCREEN_MAX_Y] = GBCore::gpu._Screen;

int GameBoyInit();
void RunANewFrame();
int KeyEvent(KEYNAME keyname,KEYMOVE keymove);
int LoadRom(std::string &dir);

int GameBoyInit() {
	GBCore::memory.ConnectTimer(&GBCore::timer);
	GBCore::memory.ConnectAPU(&GBCore::apu);
	GBCore::memory.Init();
	GBCore::cpu.Init();
	return 1;
}

void RunANewFrame() {
	while (!GBCore::gpu.GetNewFrameFlag()) {
		GBCore::cpu.Step();
	}
	GBCore::gpu.SetNewFrameFlag(0);
}
//return 0 if failed.
int KeyEvent(KEYNAME keyname, KEYMOVE keymove) {
	using namespace GBCore;
	if (keymove == KY_PRESS) {
		switch (keyname) {
		case KY_A:
			memory._KeyRow[0] &= 0xE;
			return 1;
		case KY_B:
			memory._KeyRow[0] &= 0xD;
			return 1;
		case KY_UP:
			memory._KeyRow[1] &= 0xB;
			return 1;
		case KY_DOWN:
			memory._KeyRow[1] &= 0x7;
			return 1;
		case KY_LEFT:
			memory._KeyRow[1] &= 0xD;
			return 1;
		case KY_RIGHT:
			memory._KeyRow[1] &= 0xE;
			return 1;
		case KY_SELECT:
			memory._KeyRow[0] &= 0xB;
			return 1;
		case KY_START:
			memory._KeyRow[0] &= 0x7;
			return 1;
		default:
			return 0;
		}
	}
	else if (keymove == KY_RELESE) {
		switch (keyname) {
		case KY_A:
			memory._KeyRow[0] |= 0x1;
			return 1;
		case KY_B:
			memory._KeyRow[0] |= 0x2;
			return 1;
		case KY_UP:
			memory._KeyRow[1] |= 0x4;
			return 1;
		case KY_DOWN:
			memory._KeyRow[1] |= 0x8;
			return 1;
		case KY_LEFT:
			memory._KeyRow[1] |= 0x2;
			return 1;
		case KY_RIGHT:
			memory._KeyRow[1] |= 0x1;
			return 1;
		case KY_SELECT:
			memory._KeyRow[0] |= 0x4;
			return 1;
		case KY_START:
			memory._KeyRow[0] |= 0x8;
			return 1;
		default:
			return 0;
		}
	}
	return 0;//failed
}
//return 0 if failed.
int LoadRom(std::string &dir) {
	const char *tmp = dir.c_str();
	GBCore::memory.LoadRom(tmp);
	return 1;
}
int LoadRom(char* dir) {
	GBCore::memory.LoadRom(dir);
	return 1;
}