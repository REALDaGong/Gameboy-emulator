#include "GB_MEMORY.h"

void Memory::LoadRom(const char* dir) {
	//Load banks from ROM to rom bank0 and others
	//work with Carts.
	ifstream fin(dir,ios_base::in| ios_base::binary);
	fin.read((char*)_memoryRomBank0, 0x4000);
	
	if (haveCart) {
		haveCart = 0;
		delete Cart;
	}
	int RomSize=0, RamSize=0, haveBettery=0, haveRam=0, haveMbc=0, CartType=0;
	if (detectCartType(RomSize, RamSize, haveBettery, haveRam, haveMbc,CartType)) {
		if (CartType > MBC3) { exit(0); }
		Cart = new Cartriage(RomSize, RamSize, haveBettery, haveRam, haveMbc,CartType);
		Cart->LoadROM(dir);
		haveCart = 1;
		_memoryRomBank = Cart->CurrentROMBank;
		if (haveRam) {
			_memoryExteralRam = Cart->CurrentRAMBank;
		}
	}
	else {
		//if no cart.
		_memoryRomBank = new GB_BY[0x4000];
		if (haveRam) {
			_memoryExteralRam = new GB_BY[0x2000];
		}
		fin.read((char*)_memoryRomBank, 0x4000);
	}
	fin.close();
	
}
void Memory::Init() {
	memset(_memoryWorkingRam, 0, sizeof(_memoryWorkingRam));
	memset(_memoryMapio, 0, sizeof(_memoryMapio));
	memset(_memoryZeroRam, 0, sizeof(_memoryZeroRam));
	DMAEnable = 0;
	DMAReady = 0;
	DMAptr = 0;
	if(_Timer)
		_Timer->Init();
	if (_APU)
		_APU->Init();
	if(_GPU)
		_GPU->Init();
	KeyReset();
	_inbios = 1;
}
/*
a little complex,because the mapio parts need many special W/R rules
*/
GB_BY Memory::MemoryRead(GB_DB ad) {

	switch (ad & 0xF000) {
		//bios
	case 0x0000:
		if (_inbios) {
			if (ad == 0x100)_inbios = 0;
			if (ad < 0x100)return _memory_bios[ad];
			else return _memoryRomBank0[ad];
		}
		else {
			return _memoryRomBank0[ad];
		}
		//ROM0
	case 0x1000:
	case 0x2000:
	case 0x3000:
		return _memoryRomBank0[ad];
		//ROM1
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		return _memoryRomBank[ad - 0x4000];
		break;
		//VRAM
	case 0x8000:
	case 0x9000:
		return _GPU->GPURead(ad);
		break;
		//Ex RAM
	case 0xA000:
	case 0xB000:
		
		if (haveCart) {
			if (Cart->RamEnable) {
				if(Cart->mbc->RamBankNum>3){
					return dynamic_cast<MBC_MBC3*>(Cart->mbc)->GetRTC();
				}else
				if (Cart->_RamSize == 0x2000) {
					if (Cart->CurrentRAMBank == Cart->RAM) {
						return _memoryExteralRam[ad & 0x1FFF];
					}
					else {
						return 0xFF;
					}
				}
				if (Cart->_RamSize == 0x800) {
					if (Cart->CurrentRAMBank == Cart->RAM) {
						if ((ad & 0x1FFF) < 0x800)
							return _memoryExteralRam[ad & 0x1FFF];
						else
							return 0xFF;
					}
					else {
						return 0xFF;
					}
				}
				return _memoryExteralRam[ad & 0x1FFF];
			}
			else
				return 0xFF;
		}
		
		break;
		//Workplace,echo RAM
	case 0xC000:
	case 0xD000:
	case 0xE000:
		return _memoryWorkingRam[ad & 0x1FFF];
		break;

	case 0xF000:
		if ((ad & 0xFFF)<0xE00)
			return _memoryWorkingRam[ad & 0x1FFF];
		else if ((ad & 0xFFF) < 0xEA0) {//OAM
			return _GPU->GPURead(ad);
		}
		else {
			//Zero,IO
			if ((ad & 0xFF) < 0x80) {
				if (ad == 0xFF00)return KeyRead();
				
				if (ad == 0xFF01) {
						return IOPort.ReadData();
				}
				if (ad == IF) {
					return 0xE0 | _memoryMapio[ad & 0xFF];
				}
				if (ad == IE) {
					return _memoryMapio[ad & 0xFF];
				}
				if (ad == TAC ||ad == TIMA ||ad==DIV||ad==TMA) {
					return _Timer->TimerRead(ad);
				}
				if (ad >= NR10 && ad <= 0xFF3F) {
					return _APU->APURead(ad);
				}
				if (ad >= LCDC && ad <= WX) {
					return _GPU->GPURead(ad);
				}
				return 0xFF;//invaild IO ports
			}
			else
			{
				return _memoryZeroRam[(ad & 0xFF) - 0x80];
			}
		}
	}
	return 0xFF;
}

void Memory::MemoryWrite(GB_DB ad, GB_BY val) {
	//if (DMAReady) { DMAEnable = 1; DMAReady = 0; _GPU->GPUWrite(DMA, 1);}
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
			_memoryExteralRam = Cart->CurrentRAMBank;
			_memoryRomBank = Cart->CurrentROMBank;
		}
		
		break;
	//VRAM
	case 0x8000:
	case 0x9000:
		_GPU->GPUWrite(ad, val);
		break;
	//Ex RAM
	case 0xA000:
	case 0xB000:
		
		if (haveCart) {
			if (Cart->RamEnable) {
				if (Cart->mbc->RamBankNum>3) {
					dynamic_cast<MBC_MBC3*>(Cart->mbc)->SetRTC(val);
				}else
				if (Cart->_RamSize == 0x2000) {
					if (Cart->CurrentRAMBank == Cart->RAM) {
						_memoryExteralRam[ad & 0x1FFF] = val;
						
					}
					break;
				}
				if (Cart->_RamSize == 0x800) {
					if (Cart->CurrentRAMBank == Cart->RAM) {
						if ((ad & 0x1FFF) < 0x800) {
							_memoryExteralRam[ad & 0x1FFF] = val;
							
						}
					}
					break;
				}
				_memoryExteralRam[ad & 0x1FFF] = val;
			}
		}break;
	//Workplace,echo RAM
	case 0xC000:
	case 0xD000:
	case 0xE000:
		_memoryWorkingRam[ad&0x1FFF] = val;
		break;
	
	case 0xF000:
		if (ad == 0xFF00) {
			KeyWrite(val);
		}
		if((ad&0xFFF)<0xE00)
		_memoryWorkingRam[ad & 0x1FFF] = val;
		else if ((ad & 0xFFF) < 0xEA0) {//OAM
			_GPU->GPUWrite(ad, val);
		}
		else {
			//Zero,IO
			if (ad<0xFF80&&ad>=0xFF00) {
				if (ad == DIV || ad==TAC || ad==TIMA||ad==TMA){_Timer->TimerWrite(ad,val); break;}

				if (ad == 0xFF4D) {
					_memoryMapio[ad & 0xFF] = val == 1 ? 0x7F : 0x7E;//speed switch, work on cgb.
				}
				if (ad == DMA){
					_memoryMapio[ad & 0xFF] = val;
					
					//DMAReady = 1;
					for (int i = 0; i < 0xA0; i++) {
						MemoryWrite(0xFE00+i, MemoryRead((val<<8) + i));
					}
					_GPU->GPUWrite(DMA, 0);
					break;
				}
				
				if (ad == SB) {
					
					IOPort.WriteData(val);
					
				}
				if (ad == SC) {
					
						if ((val & 0x80)) {
							IOPort.State = 1;
							if ((val & 0x1)==0) {
								IOPort.State = 2;
								//with exteral clock,
								//never tranfer,never end.
							}
						}
						else {
							IOPort.State = 0;
						}
					
				}
				if (ad >= NR10 && ad <= 0xFF3F) {
					_APU->APUWrite(ad, val);
					break;
				}
				if (ad >= LCDC && ad <= WX) {
					_GPU->GPUWrite(ad, val);//DMA will be catched before this.
					break;
				}
				_memoryMapio[ad&0xFF] = val;
			}
			else if(ad>=0xFF80)
			{
				_memoryZeroRam[(ad & 0xFF)-0x80] = val;
			}
			else {
				return; //unused part;
			}
		}
	}
	return;
}
void Memory::ConnectTimer(Timer* timer) {
	_Timer =timer;
}
void Memory::ConnectAPU(APU* apu) {
	_APU = apu;
}
void Memory::ConnectGPU(GPU* gpu) {
	_GPU = gpu;
}
void Memory::Send(GB_BY interrupt) {
	_memoryMapio[0xFF & IF] |= interrupt;
}
//joypad part
inline void Memory::KeyReset() {
	_KeyCol = 0;//bit 5=0 select Button keys (0x10)
			   //bit 4=0 select Direction keys (0x20)
			   //no all zeros

	//bit=0 means pressed  0  1  2  3
	//bit 4->Row 0
	//bit 5->Row 1
	_KeyRow[0] = 0x0F;//   R  L  U  D  	
	_KeyRow[1] = 0x0F;//   A  B  Se St
}
inline GB_BY Memory::KeyRead() {
	if (_KeyCol == 0x10) {//select Button
		return _KeyRow[1]|0xD0;
	}
	else if(_KeyCol == 0x20){//select Dir
		return _KeyRow[0]|0xE0;
	}
	return 0xF;
}
inline void Memory::KeyWrite(GB_BY val) {
	_KeyCol = val & 0x30;
	_memoryMapio[0] &= 0xF;
	_memoryMapio[0] |= val & 0x30;
}





int Memory::detectCartType(int &RomSize, int &RamSize, int &haveBettery, int &haveRam, int &haveMbc,int &CartType) {

	if (_memoryRomBank0[0x148]<8)
		RomSize = (1 << 15) << _memoryRomBank0[0x148];
	else
		switch (_memoryRomBank0[0x148])
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


	switch (_memoryRomBank0[0x149]) {
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
	CartType = _memoryRomBank0[0x147];
	switch (CartType) {
	case 0://ROM only
		CartType = 0;
		break;
	case 0x1://mbc1
		CartType = MBC1;
		haveCart = 1;
		haveMbc = 1;
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
		haveRam = 1;
		haveBettery = 1;
		haveCart = 1;
		haveMbc = 1;
		break;
	case 0x8://ROM
		CartType = 0;
		break;
	case 0x9://&+ram
		haveRam = 1;
		CartType = 0;
		break;
	case 0xB://MMM01
	case 0xC://&+ram
	case 0xD://&+battery
		CartType = MMM01;
		break;
	case 0xF://MBC3+timer+battery
	case 0x10://&+ram
	case 0x11://no extra
	case 0x12://+ram
	case 0x13://&battery
		CartType = MBC3;
		haveRam = 1;
		haveBettery = 1;
		haveCart = 1;
		haveMbc = 1;
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
