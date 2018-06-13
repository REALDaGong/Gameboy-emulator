#include "GB_CART.h"


void Cartriage::LoadROM(const char* dir) {
	ROM = new GB_BY[_RomSize];
	CurrentROMBank = ROM + 0x4000;
	


	if (_haveRam) {
		RAM = new GB_BY[_RamSize];
		CurrentRAMBank = RAM;
		memset(RAM, 0, _RamSize);
		Dir=dir;
		int splace = Dir.find_last_of('\\');
		int eplace = Dir.find_last_of('.');
		string subDir = Dir.substr(splace + 1, eplace - splace - 1);


		ifstream finb;
		finb.open("E:\\saves\\" + subDir + ".sav", ios::in || ios::binary);
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

		}
	ifstream fin(dir, ios_base::in | ios_base::binary);
	fin.read((char*)ROM, _RomSize);
	fin.close();
	
	
}
int Cartriage::saveB(GB_DB ad, GB_BY val) {
	CurrentRAMBank[ad & 0x1FFF] = val;
	return 1;
}
int Cartriage::loadB(GB_DB ad) {
	return 1;
}