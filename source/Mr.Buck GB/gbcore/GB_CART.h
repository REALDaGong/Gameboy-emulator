#pragma once
#include "GB.h"

/*
directly used by MMU.
manage cart info,alloc proper mem space to store data
use the given MBC to manage the mem banks.
*/

class MBC {
public:

	MBC() noexcept {};
	virtual ~MBC() {};
	virtual void Translate(GB_DB ad, GB_BY val) = 0;
	virtual void SwitchROMBank() = 0;
	virtual void SwitchRAMBank() = 0;
};
class Cartriage {

public:

	GB_BY*  CurrentROMBank;//point to current bank's start place
	GB_BY*  CurrentRAMBank;

	friend class Memory;
	friend class MBC_MBC1;
	friend class MBC_MBC2;
	//no mbc3 actually.....
	Cartriage(int RomSize, int RamSize, int haveBattery, int haveRam, int haveMbc, int CartType) :
		_RomSize(RomSize), _RamSize(RamSize), _haveBattery(haveBattery), _haveRam(haveRam), _haveMbc(haveMbc), _CartType(CartType)
	{
		CurrentROMBank = NULL;
		CurrentRAMBank = NULL;
		mbc = NULL;
		RAM = NULL;
		ROM = NULL;
		RamEnable = 0;
	}
	~Cartriage() {
		delete[] ROM;
		if (_haveRam&&_haveBattery)
		{
			//save procedure
			
			int splace = Dir.find_last_of('\\');
			int eplace = Dir.find_last_of('.');
			string subDir = Dir.substr(splace + 1, eplace - splace - 1);


			ofstream fout;
			fout.open("C:\\"+subDir+".sav", ios::trunc | ios::out| ios::binary);
			if (fout.is_open())
			{
				fout.write((char*)RAM, _RamSize);
				fout.close();
			}
			delete[] RAM;
		}
		if (_haveMbc)
			delete mbc;
	}

	//battery memory.
	void MemoryWrite(GB_DB ad,GB_BY val);
	GB_BY MemoryRead(GB_DB ad);

	//return the Bank Pointer.
	GB_BY* SendMessage(GB_DB ad, GB_BY Val) {
		if (ad < 0x8000)mbc->Translate(ad, Val);
		else {

		}
		return NULL;
	}
	

	//load all data
	void LoadROM(const char *dir);
private:
	int _RomSize, _RamSize, _haveBattery, _haveRam, _haveMbc, _CartType;
	GB_BY* ROM;//point to all data
	GB_BY* RAM;//all ram

	string Dir;

	int RamEnable;

	MBC* mbc;
};

#define ROM_BANKING_MODE 0
#define RAM_BANKING_MODE 1

class MBC_MBC1 :public MBC {

public:
	MBC_MBC1(Cartriage& ca) :cart(ca) { 
		RomBankNum = 1;
		RamOrRomUpper = 0;
		Mode = ROM_BANKING_MODE;
	};
	virtual ~MBC_MBC1() {};
	virtual void Translate(GB_DB ad,GB_BY val) {
		switch (ad&0xF000) {
		case 0:
		case 0x1000:
			if ((val & 0xF) == 0xA) {
				cart.RamEnable = 1;
			}
			else {
				cart.RamEnable = 0;
			}
			break;
		case 0x2000:
		case 0x3000:
			RomBankNum = val & 0x1f;
			if (Mode == ROM_BANKING_MODE) {
				SwitchROMBank();
			}
			else {
				if (RomBankNum == 0)
					cart.CurrentROMBank = cart.ROM + 0x4000;
				else
				cart.CurrentROMBank = cart.ROM + (RomBankNum&0x1F) * 0x4000;
			}
			break;
		case 0x4000:
		case 0x5000:
			RamOrRomUpper = val & 3;
			if (Mode == ROM_BANKING_MODE) {
				SwitchROMBank();
			}
			else {
				cart.CurrentRAMBank = cart.RAM+RamOrRomUpper*0x2000;
			}
			break;
		case 0x6000:
			
		case 0x7000:
			if (val == 1) {
				Mode = RAM_BANKING_MODE;
				cart.CurrentROMBank = cart.ROM + (RomBankNum & 0x1F) * 0x4000;
			}
			else {
				Mode = ROM_BANKING_MODE;
				cart.CurrentRAMBank = cart.RAM;
			}
			break;
		default:
			break;
		}
	}
	virtual void SwitchROMBank() {
		if (RomBankNum == 0 || RomBankNum == 0x20 || RomBankNum == 0x40 || RomBankNum == 0x60)
			cart.CurrentROMBank = cart.ROM + (((RomBankNum & 0x1F) + ((RamOrRomUpper & 3) << 5)) + 1) * 0x4000;
		else
			cart.CurrentROMBank = cart.ROM + ((RomBankNum & 0x1F) + ((RamOrRomUpper & 3) << 5)) * 0x4000;
	}
	virtual void SwitchRAMBank() {

	}
private:
	GB_BY Mode;
	
	GB_BY RomBankNum;
	GB_BY RamOrRomUpper;

	Cartriage & cart;
	
};

class MBC_MBC2 :public MBC {
public:
	MBC_MBC2(Cartriage& ca) :cart(ca) {
		RomBankNum = 1;
	};
	virtual ~MBC_MBC2() {};
	virtual void Translate(GB_DB ad, GB_BY val) {
		switch (ad & 0xF000) {
		case 0:
		case 0x1000:
			if ((val & 0xA) == 0xA) {
				cart.RamEnable = 1;
			}
			else {
				cart.RamEnable = 0;
			}
			break;
		case 0x2000:
		case 0x3000:
			RomBankNum = val & 0x1f;
			
			break;
		
		default:
			break;
		}
	}
	virtual void SwitchROMBank() {
	
	}
	virtual void SwitchRAMBank() {

	}
private:
	GB_BY RomBankNum;
	Cartriage & cart;

};


