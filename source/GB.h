#pragma once
#include<iostream>
#include<cctype>
#include<array>
#include<functional>
#include<string>
#include<fstream>
using namespace std;
typedef uint32_t CLOCK_Val;
typedef uint16_t GB_DB;
typedef uint8_t GB_BY;


//Memory REG
//p35
#define P1 0xFF00
//p37
#define SB 0xFF01
#define SC 0xFF02
//p38
#define DIV 0xFF04
#define TIMA 0xFF05
//p39
#define TMA 0xFF06
#define TAC 0xFF07
#define IF 0xFF0F
//p40
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
//p47
#define NR41 0xFF20
#define NR42 0xFF21
//p48
#define NR43 0xFF22
#define NR44 0xFF23
//p49
#define NR50 0xFF24
//p50
#define NR51 0xFF21
//p51
#define NR52 0xFF26
//FF30-3F Wave Pattern
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




