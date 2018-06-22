#include "GB_CART.h"

/*
only declare how to read Ex memmory and how to load rom.
*/

void Cartriage::LoadROM(const char *dir) {
    ROM = new GB_BY[_RomSize];
    CurrentROMBank = ROM + 0x4000;


    if (_haveRam) {
        RAM = new GB_BY[32768];
        CurrentRAMBank = RAM;
        memset(RAM, 0, _RamSize);
        Dir = dir;
        int splace = Dir.find_last_of('\\');
        int eplace = Dir.find_last_of('.');
        string subDir = Dir.substr(splace + 1, eplace - splace - 1);


        ifstream finb;
        finb.open("C:\\" + subDir + ".sav", ios::in | ios::binary);
        if (finb) {
            finb.read((char *) RAM, _RamSize);
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
        }
    ifstream fin(dir, ios_base::in | ios_base::binary);
    fin.read((char *) ROM, _RomSize);
    fin.close();


}

void Cartriage::MemoryWrite(GB_DB ad, GB_BY val) {
    if (_RamSize == 0x2000) {
        if (CurrentRAMBank == RAM) {
            CurrentRAMBank[ad & 0x1FFF] = val;
            return;
        }
    }
    if (_RamSize == 0x800) {
        if (CurrentRAMBank == RAM && (ad & 0x1FFF) < 0x800) {
            CurrentRAMBank[ad & 0x1FFF] = val;
            return;
        }
    }
    CurrentRAMBank[ad & 0x1FFF] = val;
    return;
}

GB_BY Cartriage::MemoryRead(GB_DB ad) {
    if (_RamSize == 0x2000) {
        if (CurrentRAMBank == RAM) {
            return CurrentRAMBank[ad & 0x1FFF];
        } else {
            return 0xFF;
        }
    }
    if (_RamSize == 0x800) {
        if (CurrentRAMBank == RAM && (ad & 0x1FFF) < 0x800) {
            return CurrentRAMBank[ad & 0x1FFF];
        } else {
            return 0xFF;
        }
    }
    return CurrentRAMBank[ad & 0x1FFF];
}