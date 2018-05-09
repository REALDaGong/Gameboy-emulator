#include "GB_GPU.h"
void GPU::GPUStep() {
	//CHECK BIT!
#define Hb 1
#define Vb 2
#define OAM 4
#define VRAM 8
	switch (mode) {
	case(Hb):
		if (_GPU_CLOCK >= 204) {
			_GPU_CLOCK = 0;
			line++;

			if (line == 143)
			{
				_Memory.MemoryWrite(0xFF41, (GB_BY)Vb);
				//new frame;
			}
			else {
				_Memory.MemoryWrite(0xFF41, (GB_BY)OAM);
			}
		}
		break;
	case(Vb):
		if (_GPU_CLOCK >= 456) {
			_GPU_CLOCK = 0;
			line++;

			if (line > 153) {
				_Memory.MemoryWrite(0xFF41, (GB_BY)OAM);
				line = 0;
			}
		}
		break;
	case(OAM):
		if (_GPU_CLOCK >= 80) {
			_GPU_CLOCK = 0;
			_Memory.MemoryWrite(0xFF41, (GB_BY)VRAM);

		}
		break;
	case(VRAM):
		if (_GPU_CLOCK >= 172) {
			_GPU_CLOCK = 0;
			_Memory.MemoryWrite(0xFF41, (GB_BY)Hb);

			//new scanline
		}
	}
}
void GPU::Newline() {

}
void GPU::NewFrame() {

}