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
	Pixel _Screen[256][256];
	Pixel _Window[256][256];
	
	GB_BY* line;
	int NewFrameFlag;
	GB_BY* mode;

	void Newline();
	void NewFrame();
	void UpgradeSprite();
	void Transfer(GB_BY Type, GB_BY MapNoSt, GB_BY TileSt, GB_BY Mask);
public:
	GPU(Memory& memory) :_Memory(memory) {  };
	~GPU() {};
	void AddClock(GPU_CLOCK delta) { 
		_GPU_CLOCK += delta;
		GPUStep();
	}
	void SetNewFrameFlag(int a) {
		NewFrameFlag = a > 0 ? 1 : 0;
	}
	int GetNewFrameFlag() {
		return NewFrameFlag;
	}
};