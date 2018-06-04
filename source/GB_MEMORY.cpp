#include "GB_MEMORY.h"
#define _EARLY_DEBU
#ifdef _EARLY_DEBUG
#ifdef _EARLY_DEBUG
extern int PAUSE;
#include<conio.h>
#endif // _EARLY_DEBUG
#endif
void Memory::LoadRom(const char* dir) {
	//Load banks from ROM to rom bank0 and others
	ifstream fin(dir,ios_base::in| ios_base::binary);
	fin.read((char*)&_memory_rom_bank0, 0x4000);
	fin.read((char*)&_memory_rom_other_bank, 0x4000);
	fin.close();
	
}
void Memory::Init() {
	//_inbios = 1;
	memset(_memory_exteral_ram, 0, sizeof(_memory_exteral_ram));
	memset(_memory_graphics_ram, 0, sizeof(_memory_graphics_ram));
	memset(_memory_working_ram, 0, sizeof(_memory_working_ram));
	memset(_memory_mapio, 0, sizeof(_memory_mapio));
	memset(_memory_oam, 0, sizeof(_memory_oam));
	memset(_memory_zero_ram, 0, sizeof(_memory_zero_ram));
	
	KeyReset();
	_inbios = 0;
	//string dir;
	//LoadRom(dir);
	MemoryWrite(TIMA, (GB_BY)0);
	MemoryWrite(TAC, (GB_BY)0);
	MemoryWrite(TMA, (GB_BY)0);
	MemoryWrite(LCDC, (GB_BY)0x91);
	MemoryWrite(SCY, (GB_BY)0);
	MemoryWrite(SCX, (GB_BY)0);
	MemoryWrite(LYC, (GB_BY)0);
	MemoryWrite(BGP, (GB_BY)0xFC);
	MemoryWrite(OBP0, (GB_BY)0xFF);
	MemoryWrite(OBP1, (GB_BY)0xFF);
	MemoryWrite(WY, (GB_BY)0);
	MemoryWrite(WX, (GB_BY)0);
	MemoryWrite(IE, (GB_BY)0);

	MemoryWrite(STAT, (GB_BY)0x81);
	MemoryWrite(IF, (GB_BY)0xE1);
	MemoryWrite(0xFF4D, 0x7E);
	_memory_mapio[0x44] = 0x90;
}
GB_BY Memory::MemoryRead(GB_DB ad) {

	switch (ad & 0xF000) {
		//bios
	case 0x0000:
		if (_inbios) {
			if (ad < 0x101)return _memory_bios[ad];

		}
		else {
			return _memory_rom_bank0[ad];
		}
		//ROM0
	case 0x1000:
	case 0x2000:
	case 0x3000:
		return _memory_rom_bank0[ad];
		//ROM1
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		return _memory_rom_other_bank[ad - 0x4000];
		break;
		//VRAM
	case 0x8000:
	case 0x9000:
		return _memory_graphics_ram[ad & 0x1FFF];
		break;
		//Ex RAM
	case 0xA000:
	case 0xB000:
		return _memory_exteral_ram[ad & 0x1FFF];
		break;
		//Workplace,echo RAM
	case 0xC000:
	case 0xD000:
	case 0xE000://epic!long live the imran nazar!
		return _memory_working_ram[ad & 0x1FFF];
		break;

	case 0xF000:
		if ((ad & 0xFFF)<0xE00)
			return _memory_working_ram[ad & 0x1FFF];
		else if ((ad & 0xFFF) < 0xEA0) {//OAM
			return _memory_oam[ad & 0xFF];
		}
		else {
			//Zero,IO
			if ((ad & 0xFF) < 0x80) {
				if (ad == 0xFF00)return KeyRead();

				return _memory_mapio[ad & 0xFF];
			}
			else
			{
				return _memory_zero_ram[(ad & 0xFF) - 0x80];
			}
		}
	}
}

void Memory::MemoryWrite(GB_DB ad, GB_BY val) {
#ifdef _EARLY_DEBUG
	//if (PAUSE==1)
	//{
		//cout << hex << ad << "=" << (GB_DB)val << endl;
		//PAUSE = 1;
	//}
#endif // _EARLY_DEBUG 
	switch (ad&0xF000) {
	//bios
	case 0x0000:
		if (_inbios&&ad<0x100)return;
	//ROM0
	case 0x1000:
	case 0x2000:
	case 0x3000:
	//ROM1
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		break;
	//VRAM
	case 0x8000:
	case 0x9000:
		_memory_graphics_ram[ad & 0x1FFF] = val;
		if (ad <= 0x97FF) {
			UpdateTile(ad);
		}
		break;
	//Ex RAM
	case 0xA000:
	case 0xB000:
		_memory_exteral_ram[ad & 0x1FFF] = val;
		break;
	//Workplace,echo RAM
	case 0xC000:
	case 0xD000:
	case 0xE000:
		_memory_working_ram[ad&0x1FFF] = val;
		break;
	
	case 0xF000:
		if (ad == 0xFF00) {
			MemoryWrite(IF, MemoryRead(IF) | 0x10);//input
			KeyWrite(val);
		}
		if((ad&0xFFF)<0xE00)
		_memory_working_ram[ad & 0x1FFF] = val;
		else if ((ad & 0xFFF) < 0xEA0) {//OAM
			_memory_oam[ad & 0xFF] = val;
		}
		else {
			//Zero,IO
			if ((ad & 0xFF) < 0x80) {
				if (ad == DIV){_memory_mapio[ad & 0xFF] = 0; break;}
				if (ad==LY){ _memory_mapio[ad & 0xFF] = 0; break; }
				if (ad == 0xFF4D) {
					_memory_mapio[ad & 0xFF] = val == 1 ? 0x7F : 0x7E;
				}
				if (ad == LCDC) {
					_memory_mapio[ad & 0xFF] = val;
					if (!(val & 0x80)) {
						_memory_mapio[0x44] = 0;
						_memory_mapio[0x41] = 0x80;
						break;
					}
				}
				if (ad == DMA){
					for (int i = 0; i < 0xA0; i++) {
						MemoryWrite(0xFE00+i, MemoryRead((val<<8) + i));
					}
					break;
				}
				
				_memory_mapio[ad&0xFF] = val;
			}
			else
			{
				_memory_zero_ram[(ad & 0xFF)-0x80] = val;
			}
		}
	}
		
}

void Memory::KeyReset() {
	_KeyCol = 0;
	_KeyRow[0] = 0x0F;
	_KeyRow[1] = 0x0F;
}
GB_BY Memory::KeyRead() {
	if (_KeyCol == 0x10) {
		return _KeyRow[0]|0xD0;
	}
	else if(_KeyCol==0x20){
		return _KeyRow[1]|0xE0;
	}return 0;
}
void Memory::KeyWrite(GB_BY val) {
	_KeyCol = val & 0x30;
}
void Memory::UpdateTile(GB_DB ad) {
	GB_DB TileNo = (ad - 0x8000) / 16;

	for (int i = 0; i < 8; i++) {
		GB_BY Byte1 = _memory_graphics_ram[TileNo * 16 + i * 2];
		GB_BY Byte2 = _memory_graphics_ram[TileNo * 16 + i * 2 + 1];
		for (int j = 0; j < 8; j++) {
			TileSet[TileNo][j][i] = (MemoryRead(BGP) >> (2 * (((Byte1 >> (7 - j)) & 1) + ((Byte2 >> (7 - j)) & 1) * 2))) & 3;
		}
	}
}