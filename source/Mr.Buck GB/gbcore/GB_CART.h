#pragma once
#include "GB.h"
#include <ctime>
/*
directly used by MMU.
manage cart info,alloc proper mem space to store data
use the given MBC to manage the mem banks.
*/

class MBC {
public:

	MBC() noexcept {};
	virtual ~MBC() {};
	virtual void Translate(const GB_DB ad,const GB_BY val) = 0;
	virtual void SwitchROMBank() = 0;
	virtual void SwitchRAMBank() = 0;
	GB_BY RomBankNum;
	GB_BY RamBankNum;
private:

};
class Cartriage {

public:

	GB_BY*  CurrentROMBank;//point to current bank's start place
	GB_BY*  CurrentRAMBank;

	friend class Memory;
	friend class MBC_MBC1;
	friend class MBC_MBC2;
	friend class MBC_MBC3;
	
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
	~Cartriage();

	
	void SendMessage(GB_DB ad, GB_BY Val) {
		if (ad < 0x8000)mbc->Translate(ad, Val);
		else {}
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
		RamBankNum = 0;
		RamOrRomUpper = 0;
		Mode = ROM_BANKING_MODE;
	};
	virtual ~MBC_MBC1() {};
	virtual void Translate(const GB_DB ad, const GB_BY val);
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
	GB_BY RamOrRomUpper;

	Cartriage & cart;
	
};

class MBC_MBC2 :public MBC {
public:
	MBC_MBC2(Cartriage& ca) :cart(ca) {
		RomBankNum = 1;
		RamBankNum = 0;
	};
	virtual ~MBC_MBC2() {};
	virtual void Translate(const GB_DB ad, const GB_BY val);
	virtual void SwitchROMBank() {
	
	}
	virtual void SwitchRAMBank() {

	}
private:
	
	Cartriage & cart;

};

class MBC_MBC3 :public MBC {
public:
	MBC_MBC3(Cartriage& ca);
	virtual ~MBC_MBC3();
	virtual void Translate(GB_DB ad, GB_BY val);
	virtual void SwitchROMBank() {}
	virtual void SwitchRAMBank() {}
	GB_BY GetRamBankNum() { return RamOrRomUpper; }
	GB_BY GetRTC() { return *RTC; }
	void SetRTC(GB_BY val) {
		
		if (RTC == &RTC_DH) {
			if ((val & 0x40)&&((*RTC)&0x40)==0) {//stop timer
				active = 0;
				UpdateTime();
			}
			else {//active timer
				active = 1;
				Active();
			}
		}
		*RTC |= val&0xFE;//? is the day first bit can be changed?
	}
	void UpdateTime();
	void Active();
	
private:
	GB_BY Mode;
	GB_BY RamOrRomUpper;

	friend class Cartriage;
	Cartriage & cart;
	
	GB_BY RTC_S;
	GB_BY RTC_M;
	GB_BY RTC_H;

	GB_BY RTC_DL;
	GB_BY RTC_DH;

	GB_BY *RTC;
	GB_BY active;
	/*
	 08h  RTC S   Seconds   0-59 (0-3Bh)
	 09h  RTC M   Minutes   0-59 (0-3Bh)
	 0Ah  RTC H   Hours     0-23 (0-17h)
	 0Bh  RTC DL  Lower 8 bits of Day Counter (0-FFh)
	 0Ch  RTC DH  Upper 1 bit of Day Counter, Carry Bit, Halt Flag
        Bit 0  Most significant bit of Day Counter (Bit 8)
        Bit 6  Halt (0=Active, 1=Stop Timer)
        Bit 7  Day Counter Carry Bit (1=Counter Overflow)
	*/
	GB_BY RTCRamBank;
	tm* oldtime;
	time_t oldtimev;
	struct timeDelta {
		int day;
		int hour;
		int min;
		int sec;
	}Dtime;
	void SubstractTime(timeDelta &tD,const tm* newtime);
};
