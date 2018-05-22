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
	
	Pixel _Window[256][256];
	
	GB_BY* line;
	int NewFrameFlag;
	GB_BY* mode;

	void Newline();
	void NewFrame();
	void UpgradeSprite();
	void Transfer(GB_BY Type, GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask);
public:
	Pixel _Screen[256][256];
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