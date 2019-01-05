#pragma once
#include "GB.h"
#include "GB_CART.h"
#include "GB_SERIALIO.h"
#include "GB_Timer.h"
#include "GB_APU.h"
/*
Memory Manage Unit actually.
Manage all W/R.
also have to control Carts and Serial IO parts.
load rom
also manage joypad.
*/
class Memory:public Hardware 
{
public:
	Memory(){
		Init(); 
		Cart = NULL;
		haveCart = 0; 
		_memoryRomBank = NULL;
		memset(TileSet, 0, sizeof(TileSet));
	};
	~Memory() { 
		if (haveCart)delete Cart;
		else delete[]_memoryRomBank;
	};
	GB_BY MemoryRead(GB_DB ad);
	void MemoryWrite(GB_DB ad, GB_BY v);
	
	void ConnectTimer(Timer* Timer);
	//void ConnectGPU(GPU* GPU);
	void ConnectAPU(APU* APU);

	void Init();
	void LoadRom(const char* dir);
	
	inline void SendClock(GB_BY delta) {
		if (IOPort.State == 1) {
			IOPort.addClock(delta);
			if (IOPort.State == 0) {
				_memoryMapio[0x0f] |= 0x08;
				_memoryMapio[0x02] = 0;
				_memoryMapio[0x01] = 0;
			}
		}
		//_APU->SendClock(delta);
	}//only to timing Serial IO.
	void Send(GB_BY Interrupt);//for other device sending interrupt
	
	GB_BY _memoryMapio[0x80];
	GB_BY _inbios;
	GB_BY _KeyRow[2];
	GB_BY TileSet[384][8][8];
	//be accessed very often.

	Cartriage* Cart;
	int haveCart;

	SerialPort IOPort;

	const GB_BY _memory_bios[0x101] = {
		0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
		0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
		0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
		0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
		0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
		0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
		0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
		0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
		0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
		0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
		0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
		0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
		0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
		0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
		0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
		0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50,0xED
	};

private:
	
	GB_BY _memoryRomBank0[0x4000];
	GB_BY *_memoryRomBank;
	GB_BY _memoryGraphicsRam[0x2000];
	GB_BY *_memoryExteralRam;
	GB_BY _memoryWorkingRam[0x2000];
	GB_BY _memoryOam[0x100];
	GB_BY _memoryZeroRam[0x80];
	
	void KeyReset();
	GB_BY KeyRead();
	void KeyWrite(GB_BY val);
	GB_BY _KeyCol;

	void UpdateTile(GB_DB ad);//pre-tranlate the tile data into images,making GPU work faster.

	Timer *_Timer;
	//GPU *_GPU;
	APU *_APU;

	int detectCartType(int &RomSize, int &RamSize, int &haveBettery, int &haveRam, int &haveMbc,int &CartType);
};