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
	fin.read((char*)_memory_rom_bank0, 0x4000);
	
	if (haveCart) {
		haveCart = 0;
		delete Cart;
	}
	int RomSize=0, RamSize=0, haveBettery=0, haveRam=0, haveMbc=0, CartType=0;
	if (detectCartType(RomSize, RamSize, haveBettery, haveRam, haveMbc,CartType)) {
		Cart = new Cartriage(RomSize, RamSize, haveBettery, haveRam, haveMbc,CartType);
		Cart->LoadROM(dir);
		haveCart = 1;
		if (haveRam) {
			_memory_exteral_ram = Cart->CurrentRAMBank;
			_memory_rom_bank = Cart->CurrentROMBank;
		}
	}
	else {
		//no cart.
		_memory_rom_bank = new GB_BY[0x4000];
		if (haveRam) {
			_memory_exteral_ram = new GB_BY[0x2000];
		}
		fin.read((char*)_memory_rom_bank, 0x4000);
	}
	fin.close();
	
}
void Memory::Init() {
	//_inbios = 1;
	//memset(_memory_exteral_ram, 0, sizeof(_memory_exteral_ram));
	
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
	MemoryWrite(0xFF4D, 0x00);
	//_memory_mapio[0x44] = 0x90;
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
		return _memory_rom_bank[ad - 0x4000];
		break;
		//VRAM
	case 0x8000:
	case 0x9000:
		return _memory_graphics_ram[ad & 0x1FFF];
		break;
		//Ex RAM
	case 0xA000:
	case 0xB000:
		
		if (haveCart) {
			if (Cart->RamEnable)
				return _memory_exteral_ram[ad & 0x1FFF];
			else
				return 0xFF;
		}
		
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
		
		if (haveCart) {
			Cart->SendMessage(ad, val);
			_memory_exteral_ram = Cart->CurrentRAMBank;
			_memory_rom_bank = Cart->CurrentROMBank;
		}
		
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
		
		if (haveCart) {
			if (Cart->RamEnable)
			_memory_exteral_ram[ad & 0x1FFF] = val;
		}break;
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
			if (ad<0xFF80&&ad>=0xFF00) {
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
			else if(ad>=0xFF80)
			{
				_memory_zero_ram[(ad & 0xFF)-0x80] = val;
			}
			else {
				return; //unused part;
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
			TileSet[TileNo][j][i] = (0xE4 >> (2 * (((Byte1 >> (7 - j)) & 1) + ((Byte2 >> (7 - j)) & 1) * 2))) & 3;
		}
	}
}
int Memory::detectCartType(int &RomSize, int &RamSize, int &haveBettery, int &haveRam, int &haveMbc,int &CartType) {

	if (_memory_rom_bank0[0x148]<8)
		RomSize = (1 << 15) << _memory_rom_bank0[0x148];
	else
		switch (_memory_rom_bank0[0x148])
		{
		case 0x52:
			RomSize = 1152 * 1024;
		case 0x53:
			RomSize = 80 * 16 * 1024;
		case 0x54:
			RomSize = 96 * 16 * 1024;
		default:
			break;
		}


	switch (_memory_rom_bank0[0x149]) {
	case 0:
		RamSize = 0;
		break;
	case 0x01:
		RamSize = 1 << 11;
		break;
	case 0x02:
		RamSize = 1 << 13;
		break;
	case 0x03:
		RamSize = 1 << 15;
		break;
	default:
		RamSize = 0;
	}
	if (RamSize != 0)haveRam = 1;
	CartType = _memory_rom_bank0[0x147];
	switch (CartType) {
	case 0://ROM only
		CartType = 0;
		break;
	case 0x1://mbc1
	case 0x2://&+ram
	case 0x3://&+bat
		CartType = MBC1;
		haveRam = 1;
		haveBettery = 1;
		haveCart = 1;
		haveMbc = 1;
		break;
	case 0x5://mbc2
	case 0x6://&+battery
		CartType = MBC2;
		break;
	case 0x8://ROM
	case 0x9://&+ram
		CartType = 0;
		break;
	case 0xB://MMM01
	case 0xC://&+ram
	case 0xD://&+battery
		CartType = MMM01;
		break;
	case 0xF://MBC3+timer+battery
	case 0x10://&+ram
	case 0x11://none extra
	case 0x12://+ram
	case 0x13://&battery
		CartType = MBC3;
		break;
	case 0x15://mbc4
	case 0x16://+ram
	case 0x17://&+battery
		CartType = MBC4;
		break;
	case 0x19://mbc5
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
		CartType = MBC5;
		break;
	case 0xFC:
	case 0xFD:
	case 0xFE:
	case 0xFF:
	default:
		CartType= 0;
		//not surpported
	}
	return CartType;
}