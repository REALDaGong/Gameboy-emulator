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
	Pixel _Screen[32][32];
	int line;
	GB_BY mode;

	void Newline();
	void NewFrame();

public:
	GPU(Memory& memory):_Memory(memory) {};
	~GPU() {};
	void AddClock(GPU_CLOCK delta) { _GPU_CLOCK += delta; GPUStep(); }
	
};