#pragma once

#include "..\win32\stdafx.h"
/*
Provide some const values.
*/
using namespace std;
typedef uint32_t CLOCK_Val;
typedef uint16_t GB_DB;
typedef uint8_t GB_BY;
#define SYS 4194304

//Memory REG
//p35 joypad
#define P1 0xFF00
//p37 serial I/O
#define SB 0xFF01
#define SC 0xFF02
//p38 Timer
#define DIV 0xFF04
#define TIMA 0xFF05
//p39 Timer
#define TMA 0xFF06
#define TAC 0xFF07
#define IF 0xFF0F
//p40 Sound Hardware
#define NR10 0xFF10
//p41
#define NR11 0xFF11
//p42
#define NR12 0xFF12
#define NR13 0xFF13
#define NR14 0xFF14
//p43
#define NR21 0xFF16
//p44
#define NR22 0xFF17
#define NR23 0xFF18
#define NR24 0xFF19
//p45
#define NR30 0xFF1A
#define NR31 0xFF1B
//p46
#define NR32 0xFF1C
#define NR33 0xFF1D
#define NR34 0xFF1E

//FF1F not used in GB
//p47
#define NR41 0xFF20
#define NR42 0xFF21
//p48
#define NR43 0xFF22
#define NR44 0xFF23
//p49
#define NR50 0xFF24
//p50
#define NR51 0xFF25
//p51
#define NR52 0xFF26
//[FF27,FF2F] not used in GB

//[FF30,FF3F] Wave Pattern

//LCD
#define LCDC 0xFF40
//p52
#define STAT 0xFF41
//p54
#define SCY 0xFF42
//p55
#define SCX 0xFF43
#define LY 0xFF44
#define LYC 0xFF45

#define DMA 0xFF46
//p57
#define BGP 0xFF47
//p58
#define OBP0 0xFF48
#define OBP1 0xFF49
#define WY 0xFF4A
#define WX 0xFF4B
#define IE 0xFFFF


#define SCREEN_MAX_X 160
#define SCREEN_MAX_Y 144
//cart types
#define MBC1 1
#define MBC2 2
#define MBC3 3
#define MMM01 9
#define MBC4 4
#define MBC5 5
#define MBC6 6
#define MBC7 7
//serial statement
#define SERIAL_EXTERN_CLOCK 2
#define SERIAL_INTER_CLOCK 1
#define SERIAL_CLOSED 0
//interrupt type
#define INTERRUPT_VBLANK 0x1
#define INTERRUPT_LCDC 0x2
#define INTERRUPT_TIMER 0x4
#define INTERRUPT_SERIAL 0x8
#define INTERRUPT_JOYPAD 0x10
//for interrupt transfer,base class,an interface.
class Hardware {
public:
	virtual void Send(GB_BY INTERRUPT) = 0;
	//should have Read,Write.....
};