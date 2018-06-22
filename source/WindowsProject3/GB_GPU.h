#pragma once

#include "GB.h"
#include "GB_MEMORY.h"

class GPU {
	typedef uint32_t GPU_CLOCK;
	typedef GB_BY Pixel;

public:
	Pixel _Screen[SCREEN_MAX_X][SCREEN_MAX_Y];
	Pixel _Sprite[SCREEN_MAX_X][SCREEN_MAX_Y];
	Pixel _Window[SCREEN_MAX_X][SCREEN_MAX_Y];

	GPU(Memory &memory) : _Memory(memory) {
		line = &_Memory._memory_mapio[0x44];
		stat = &_Memory._memory_mapio[0x41];
		lcdc = &_Memory._memory_mapio[0x40];
		lyc = &_Memory._memory_mapio[0x45];
		memset(_Screen, 0, sizeof(_Screen));
		memset(_Sprite, 0, sizeof(_Sprite));
		memset(_Window, 0, sizeof(_Window));
		gpuclock = 456;
		NewFrameFlag = 0;
		lcdc_happend = 0;
		memset(ptrSprite, 0, sizeof(ptrSprite));
		memset(sprite, 0, sizeof(sprite));
	};

	~GPU() {};

	void AddClock(GPU_CLOCK delta) {
		(*stat) |= 0x80;
		if ((*lcdc) & 0x80) {
			gpuclock -= delta;
			justclosed = 1;
			GPUStep();
		} else {
			(*stat) = (*stat) & 0xf8;
			gpuclock = 456;
			*line = 0;
			if (justclosed) {
				justclosed = 0;
				memset(_Screen, 0, sizeof(_Screen));
				memset(_Sprite, 0, sizeof(_Sprite));
				memset(_Window, 0, sizeof(_Window));
				_Memory._memory_mapio[0x0f] |= 0x1;
			}
			if (*line == *lyc)(*stat) |= 0x4;
			else (*stat) &= ~0x4;
		}
	}

	inline void SetNewFrameFlag(int a) {
		NewFrameFlag = a > 0 ? 1 : 0;
	}

	inline int GetNewFrameFlag() {
		return NewFrameFlag;
	}

private:

	int gpuclock;

	void GPUStep();

	Memory &_Memory;


	GB_BY lcdc_happend;

	GB_BY *line;
	int NewFrameFlag;
	GB_BY *lcdc;
	GB_BY *stat;
	GB_BY *lyc;

	int justclosed;

	void Newline();

	void NewFrame();

	void UpdateSprite();

	void UpdateBackGround(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask);

	void UpdateWindow(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask);

	void CheckLCDCInter();

	void CheckModeInter(GB_BY Mode);

	struct Sprite {
		GB_BY X, Y, TileNo;
		GB_BY pirority;
		GB_BY Xfilp, Yfilp;
		GB_BY PlaNo;
		GB_BY isRender;
		GB_BY No;
	} sprite[41];
	Sprite *ptrSprite[41];


};
