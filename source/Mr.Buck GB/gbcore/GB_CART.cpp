#include "GB_CART.h"
/*
so complex....
*/
void Cartriage::LoadROM(const char* dir) {
	ROM = new GB_BY[_RomSize];
	CurrentROMBank = ROM + 0x4000;
	


	if (_haveRam&&_haveBattery) {
		RAM = new GB_BY[32768];
		CurrentRAMBank = RAM;
		memset(RAM, 0, _RamSize);
		Dir=dir;
		int eplace = Dir.find_last_of('.');
		string subDir = Dir.substr(0, eplace);


		ifstream finb;
		finb.open(subDir + ".sav", ios::in | ios::binary);
		if (finb)
		{
			finb.read((char*)RAM, _RamSize);
			finb.close();
		}
	}
	if (_haveMbc)
		switch (_CartType) {
		case MBC1:
			mbc = new MBC_MBC1(*this);
			break;
		case MBC2:
			mbc = new MBC_MBC2(*this);
			break;
		case MBC3:
			mbc = new MBC_MBC3(*this);
			break;
		default:
			exit(0);
			break;
		}
	ifstream fin(dir, ios_base::in | ios_base::binary);
	fin.read((char*)ROM, _RomSize);
	fin.close();
	
	
}
Cartriage::~Cartriage() {
	delete[] ROM;
	if (_haveRam&&_haveBattery)
	{
		//save procedure

		int eplace = Dir.find_last_of('.');
		string subDir = Dir.substr(0, eplace);


		ofstream fout;
		fout.open(subDir + ".sav", ios::trunc | ios::out | ios::binary);
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

void MBC_MBC1::Translate(const GB_DB ad, const GB_BY val) {
	switch (ad & 0xF000) {
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
				cart.CurrentROMBank = cart.ROM + (RomBankNum & 0x1F) * 0x4000;
		}
		break;
	case 0x4000:
	case 0x5000:
		RamOrRomUpper = val & 3;
		if (Mode == ROM_BANKING_MODE) {
			SwitchROMBank();
		}
		else {
			cart.CurrentRAMBank = cart.RAM + RamOrRomUpper * 0x2000;
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
void MBC_MBC2::Translate(const GB_DB ad, const GB_BY val) {
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
MBC_MBC3::MBC_MBC3(Cartriage& ca) :cart(ca) {
	RomBankNum = 1;
	RamBankNum = 0;
	oldtimev = 0;
	oldtime = NULL;
	if (cart._haveBattery && cart._haveRam) {
		
		int eplace = cart.Dir.find_last_of('.');
		string subDir = cart.Dir.substr(0, eplace);


		ifstream finb;
		finb.open(subDir + ".tma", ios::in | ios::binary);
		if (finb)
		{
			
			finb.read((char*)(&RTC_S), 1);
			finb.read((char*)(&RTC_M), 1);
			finb.read((char*)(&RTC_H), 1);
			finb.read((char*)(&RTC_DL), 1);
			finb.read((char*)(&RTC_DH), 1);
			
			int have;
			finb >> have;
			if (have) {
				oldtimev = time(NULL);
				oldtime = localtime(&oldtimev);
				finb >> oldtime->tm_yday;
				finb >> oldtime->tm_min;
				finb >> oldtime->tm_sec;
				finb >> oldtime->tm_hour;
				finb >> oldtime->tm_year;

				
			}
			else {
				oldtime = NULL;
			}
			finb.close();
		}
		if ((RTC_DH & 0x40) == 0 && oldtime!=NULL) {
			UpdateTime();
		}
	}
};
MBC_MBC3::~MBC_MBC3() {
	if (cart._haveRam&&cart._haveBattery)
	{
		//save procedure

		int eplace = cart.Dir.find_last_of('.');
		string subDir = cart.Dir.substr(0, eplace);


		ofstream fout;
		fout.open(subDir + ".tma", ios::trunc | ios::out | ios::binary);
		if (fout.is_open())
		{
			
			fout.write((char*)(&RTC_S), 1);
			fout.write((char*)(&RTC_M), 1);
			fout.write((char*)(&RTC_H), 1);
			fout.write((char*)(&RTC_DL), 1);
			fout.write((char*)(&RTC_DH), 1);
			
			if (oldtime != NULL) {
				fout << static_cast<int>(1);
				fout << oldtime->tm_yday;
				fout << oldtime->tm_min;
				fout << oldtime->tm_sec;
				fout << oldtime->tm_hour;
				fout << oldtime->tm_year;
			}
			else {
				fout << static_cast<int>(0);
			}
			fout.close();
		}
		
	}
}
void MBC_MBC3::Active() {
	oldtimev = time(NULL);
	oldtime = localtime(&oldtimev);
}
void MBC_MBC3::UpdateTime() {
	time_t newtimev = time(NULL);
	tm* newtime = localtime(&newtimev);
	if (newtime != NULL && oldtime != NULL) {
		
		SubstractTime(Dtime, newtime);
		
		int carry = 0;
		int tmp = 0;
		tmp = RTC_S + Dtime.sec;
		if (tmp > 59) {
			tmp -= 60;
			carry = 1;
		}
		RTC_S = tmp;

		tmp = RTC_M + Dtime.min+carry;
		if (tmp > 59) {
			tmp -= 60;
			carry = 1;
		}
		else {
			carry = 0;
		}
		RTC_M = tmp;

		tmp = RTC_H + Dtime.hour + carry;
		if (tmp > 23) {
			tmp -= 24;
			carry = 1;
		}
		else {
			carry = 0;
		}
		RTC_H = tmp;

		int day = ((RTC_DH & 1) << 8) + RTC_DL +Dtime.day+carry;
		
		
		if (day > 511) {
			RTC_DH |= 0x80;
			day %= 512;
			
		}
		RTC_DH &= 0xC0;
		RTC_DH |= (day & 0x100) >> 8;
		RTC_DL = day & 0xFF;

		
	}
}
void MBC_MBC3::SubstractTime(timeDelta &tD,const tm* newtime) {
	int borrow = 0;
	int tmp=0;
	tmp = newtime->tm_sec - oldtime->tm_sec;
	if (tmp < 0) {
		borrow = 1;
		tD.sec = 60 - oldtime->tm_sec + newtime->tm_sec;
	}
	else {
		tD.sec = tmp;
	}
	tmp = newtime->tm_min - oldtime->tm_min-borrow;
	if (tmp < 0) {
		
		tD.min = 60 - oldtime->tm_min + newtime->tm_min-borrow;
		borrow = 1;
	}
	else {
		borrow = 0;
		tD.min = tmp;
	}
	tmp = newtime->tm_hour- oldtime->tm_hour - borrow;
	if (tmp < 0) {

		tD.hour = 24 - oldtime->tm_hour + newtime->tm_hour - borrow;
		borrow = 1;
	}
	else {
		borrow = 0;
		tD.min = tmp;
	}
	tD.day = newtime->tm_yday - oldtime->tm_yday + 365 * (newtime->tm_year - oldtime->tm_year)-borrow;//well,leap year doesnt matter.
	
	if (tD.day < 0) {
		tD.day = 0;
		tD.hour = 0;
		tD.min = 0;
		tD.sec = 0;
		return;
	}
}
void MBC_MBC3::Translate(GB_DB ad, GB_BY val) {
	switch (ad & 0xF000) {
	case 0:
	case 0x1000:
		if ((val & 0xF) == 0xA) {
			cart.RamEnable = 1;//ram and timer actually.
		}
		else {
			cart.RamEnable = 0;
		}
		break;
	case 0x2000:
	case 0x3000:
		RomBankNum = val & 0x7f;
		if(val!=0)
			cart.CurrentROMBank = cart.ROM + RomBankNum * 0x4000;
		else
			cart.CurrentROMBank = cart.ROM + 0x4000;
		break;
	case 0x4000:
	case 0x5000:
		RamBankNum = val;
		if (RamBankNum <= 3) {
			cart.CurrentRAMBank = cart.RAM + RamBankNum * 0x2000;
		}
		else {
			switch (RamBankNum) {
			case 0x8:
				RTC = &RTC_S;
				break;
			case 0x9:
				RTC = &RTC_M;
				break;
			case 0xa:
				RTC = &RTC_H;
				break;
			case 0xb:
				RTC = &RTC_DL;
				break;
			case 0xc:
				RTC = &RTC_DH;
				break;
			default:
				break;
			}
		}
		break;
	case 0x6000:

	case 0x7000: {
		static GB_BY Prepared = 0;
		if (val == 0) {
			Prepared = 1;
		}
		if (Prepared && val==1) {
			Prepared = 0;
			if((RTC_DH&0x40)==0){
				UpdateTime();
			}
			
		}
	}break;
	default:
		break;
	}
}
