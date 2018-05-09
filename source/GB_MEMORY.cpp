#include "GB_MEMORY.h"
void Memory::LoadRom() {
	//Load banks from ROM to rom bank0 and others
}
void Memory::Init() {
	_inbios = 1;
}
GB_BY Memory::MemoryRead(GB_DB ad) {
	
	switch (ad & 0xF000) {
		//bios
	case 0x0000:
		if (_inbios) {
			if (ad < 0x100)return _memory_bios[ad];
			else {
				_inbios = 0;
			}
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
		return _memory_rom_other_bank[ad-0x4000];
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
				
				return _memory_mapio[ad];
			}
			else
			{
				return _memory_zero_ram[ad & 0xFF];
			}
		}
	}
}
void Memory::MemoryWrite(GB_DB ad, GB_BY val) {
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
		if((ad&0xFFF)<0xE00)
		_memory_working_ram[ad & 0x1FFF] = val;
		else if ((ad & 0xFFF) < 0xEA0) {//OAM
			_memory_oam[ad & 0xFF] = val;
		}
		else {
			//Zero,IO
			if ((ad & 0xFF) < 0x80) {
				if (ad == DIV){_memory_mapio[ad] = 0; break;}
				if (ad == DMA){
					for (int i = 0; i < 0xA0; i++) {
						MemoryWrite(0xFE00 + i, MemoryRead(ad + i));
					}
					break;
				}
				_memory_mapio[ad] = val;
			}
			else
			{
				_memory_zero_ram[ad & 0xFF] = val;
			}
		}
	}
		
}

