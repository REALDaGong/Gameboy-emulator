#pragma once
#include "GB.h"
#include "GB_MEMORY.h"
class GPU {
	typedef uint32_t GPU_CLOCK;
	typedef GB_BY Pixel;
private:
	GPU_CLOCK _GPU_CLOCK;
	void GPUStep();
	Memory& _Memory;
	
	
	
	GB_BY* line;
	int NewFrameFlag;
	GB_BY* lcdc;
	GB_BY* mode;

	void Newline();
	void NewFrame();
	void UpgradeSprite();
	void Transfer(GB_BY Type, GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask);

	void CheckLCDCCoincidenceInter();
	void CheckModeInter(GB_BY Mode);

	struct Sprite {
		GB_BY X, Y, No;
		GB_BY pirority;
		GB_BY Xfilp, Yfilp;
		GB_BY PlaNo;
		GB_BY isRender;
	}sprite[40];
	Sprite *ptrSprite[40];
public:
	Pixel _Screen[256][256];
	Pixel _Sprite[256][256];
	GPU(Memory& memory) :_Memory(memory) {
		line = &_Memory._memory_mapio[0x44];
		mode = &_Memory._memory_mapio[0x41];
		lcdc = &_Memory._memory_mapio[0x40];
	};
	~GPU() {};
	void AddClock(GPU_CLOCK delta) {
		if ((*lcdc) & 0x80) {
			_GPU_CLOCK += delta;
			GPUStep();
		}
	}
	void SetNewFrameFlag(int a) {
		NewFrameFlag = a > 0 ? 1 : 0;
	}
	int GetNewFrameFlag() {
		return NewFrameFlag;
	}
};