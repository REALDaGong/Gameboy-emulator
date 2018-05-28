#include "GB_GPU.h"
//Does need to check LCDC switch first?
void GPU::GPUStep() {
	//CHECK BIT!
#define Hb 0
#define Vb 1
#define OAM 2
#define VRAM 3
	
	switch ((*mode)&0x3) {
	case(Hb):
		CheckModeInter(Hb);
		if (_GPU_CLOCK >= 204) {
			_GPU_CLOCK = 0;
			(*line)++;
			CheckLCDCCoincidenceInter();
			
			if (*line == 143)
			{
				
				_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF) | 0x1);
				_Memory._memory_mapio[STAT - 0xFF00] &= 0xFC;
				_Memory._memory_mapio[STAT - 0xFF00] += Vb;
				NewFrame();
				
				//new frame
			}
			else {
				
				_Memory._memory_mapio[STAT - 0xFF00] &= 0xFC;
				_Memory._memory_mapio[STAT - 0xFF00] += OAM;
			}
		}
		break;
	case(Vb):
		CheckModeInter(Vb);
		if (_GPU_CLOCK >= 456) {
			_GPU_CLOCK = 0;
			(*line)++;
			CheckLCDCCoincidenceInter();
			
			
			
			if (*line > 153) {
				
				_Memory._memory_mapio[STAT - 0xFF00] &= 0xFC;
				_Memory._memory_mapio[STAT - 0xFF00] += OAM;
				*line = 0;
				CheckLCDCCoincidenceInter();
				
			}
		}
		break;
	case(OAM):
		CheckModeInter(OAM);
		if (_GPU_CLOCK >= 80) {
			_GPU_CLOCK = 0;
			
			_Memory._memory_mapio[STAT - 0xFF00] &= 0xFC;
			_Memory._memory_mapio[STAT - 0xFF00] += VRAM;
		}
		break;
	case(VRAM):
		if (_GPU_CLOCK >= 172) {
			_GPU_CLOCK = 0;
			
			_Memory._memory_mapio[STAT - 0xFF00] &= 0xFC;
			_Memory._memory_mapio[STAT - 0xFF00] += Hb;
			if((((*line)-1)%8)==0)
			Newline();
			//new scanline
		}
	}
}
void GPU::Newline() {
#define WINDOW 1
#define SCREEN 0
	GB_BY Mask =0;
	GB_BY data = _Memory.MemoryRead(LCDC);
	GB_DB TileSt = ((data&16)>>4?(Mask=0,0x8000):(Mask=128,0x8800));
	GB_DB BGMapNoSt = ((data&8)>>3?0x9C00:0x9800);
	GB_DB WNMapNoSt = ((data&64)>>6?0x9C00:0x9800);
	//start address
	//Tile[X][Y]
	Transfer(SCREEN, BGMapNoSt, TileSt, Mask);
	if ((_Memory.MemoryRead(LCDC) >> 5) & 1)
	Transfer(WINDOW, WNMapNoSt, TileSt, Mask);
}
//pay attention, its hard to understand though.......
void GPU::Transfer(GB_BY Type,GB_DB MapNoSt,GB_DB TileSt,GB_BY Mask) {
	(*line)--;
	GB_BY Xoffset;
	GB_BY Yoffset;
	if (Type == WINDOW) {
		Xoffset = _Memory.MemoryRead(WX)-7;
		Yoffset = _Memory.MemoryRead(WY);
	}
	else {
		Xoffset = _Memory.MemoryRead(SCX);
		Yoffset = _Memory.MemoryRead(SCY);

	}
	for (int i = 0; i < 32; i++) {
		GB_DB Place = TileSt + 16 * _Memory.MemoryRead(MapNoSt + (*line)/8 * 32 + i) + Mask;
		
		for (int j = 0; j < 8; j++) {
			GB_BY Byte1 = _Memory.MemoryRead(Place + j*2);
			GB_BY Byte2 = _Memory.MemoryRead(Place+1 + j*2);
			for (int k = 0; k < 8; k++) {
				_Screen[(i*8 + k)][(*line + j)] = (_Memory.MemoryRead(BGP)>>(2*(((Byte1 >> (7 - k)) & 1) + ((Byte2 >> (7 - k)) & 1) * 2)))&3;
			
			}
		}
	}
	(*line)++;
}
//read the man for detailed explanation.
//initial a frame without taking X and Y offset into consideration.
//havn't merge bg,wn and sp together.
void GPU::UpgradeSprite() {
	memset(_Sprite, -1, sizeof(_Sprite));//-1 means sprite layer is empty.
	GB_BY mode8x16 = (_Memory.MemoryRead(LCDC) >> 2) & 1;
	
	memset(sprite, 0, sizeof(sprite));
	GB_BY Rendersize = 0;
	for (int i = 0; i < 40; i++) {
		sprite[i].Y = _Memory.MemoryRead(0xFE00 + i * 4);
		sprite[i].X = _Memory.MemoryRead(0xFE00 + i * 4 + 1);

		if (sprite[i].Y == 0 && sprite[i].X == 0)sprite[i].isRender = 0;
		else {
			sprite[i].Y -= 16;
			sprite[i].X -= 8;
			sprite[i].isRender = 1;
			sprite[i].No = _Memory.MemoryRead(0xFE00 + i * 4 + 2) >> mode8x16;
			GB_BY flag = _Memory.MemoryRead(0xFE00 + i * 4 + 3);
			sprite[i].pirority = (flag >> 7) & 1;
			sprite[i].Yfilp = (flag >> 6) & 1;
			sprite[i].Xfilp = (flag >> 5) & 1;
			sprite[i].PlaNo = (flag >> 4) & 1;
			ptrSprite[Rendersize++] = &sprite[i];
		}
		
	}
	//sort them,the first will be rendered first,meaning lowest priority. 
	
	for (int j = 0; j < Rendersize-1; j++) {
		for (int i = 0; i < Rendersize-1; i++) {

			Sprite* tmp;
			if (ptrSprite[i]->X <= ptrSprite[i + 1]->X) {
				if (ptrSprite[i]->X != ptrSprite[i + 1]->X) {
					tmp = ptrSprite[i];
					ptrSprite[i] = ptrSprite[i + 1];
					ptrSprite[i] = tmp;
				}
				else {
					if (ptrSprite[i]->No < ptrSprite[i + 1]->No) {
						tmp = ptrSprite[i];
						ptrSprite[i] = ptrSprite[i + 1];
						ptrSprite[i] = tmp;
					}
				}
			}
		}
	}
	//i can use less<> if i want.but i dont.
	//transfer data to pics
	//miss the proper process on flip and prority.
	GB_BY PicSize = mode8x16 ? 16 : 8;
	for (int i = 0; i < 40;i++) {
		if (ptrSprite[i]) {
			GB_DB Place = 0x8000+ptrSprite[i]->No*PicSize*2;
			for (int k = 0; k < PicSize;k++) {
				GB_BY byte1 = _Memory.MemoryRead(Place + 2*k);
				GB_BY byte2 = _Memory.MemoryRead(Place + 2*k+1);
				for (int j = 0; j < 8; j++) {
					_Sprite[(j + ptrSprite[i]->X) % 256][(k + ptrSprite[i]->Y) % 256] = (_Memory.MemoryRead((ptrSprite[i]->PlaNo)+0xFF48)
																				>>(2*(((byte1 >> (7 - j)) & 1) + ((byte2 >> (7 - j)) & 1) * 2)))&3;
				}
			}
			continue;
		}
		break;
	}
}
//didnt limit the sprites number per line.
void GPU::NewFrame() {
	UpgradeSprite();
	//[X][Y]
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			if (_Screen[i][j] == 0 && _Sprite[i][j] != 255) {
				_Screen[i][j] = _Sprite[i][j];
			}
			else {
				
			}
		}
	}
	SetNewFrameFlag(1);
	//do it!
	//merge them!!!!;
}
inline void GPU::CheckLCDCCoincidenceInter() {
	if (_Memory._memory_mapio[LYC-0xFF00] == _Memory._memory_mapio[LY - 0xFF00]) {
		_Memory._memory_mapio[STAT - 0xFF00] |= 0x4;
	}
	else {
		_Memory._memory_mapio[STAT - 0xFF00] &= 0xFB;
	}
	if ((_Memory._memory_mapio[STAT - 0xFF00] & 0x4) && (_Memory._memory_mapio[STAT - 0xFF00] & 64)) {
		_Memory.MemoryWrite(IF, _Memory.MemoryRead(0xFFFF) | 0x2);
	}
}
inline void GPU::CheckModeInter(GB_BY Mode) {
	if(_Memory._memory_mapio[STAT - 0xFF00]>>(Mode+3)&1)_Memory.MemoryWrite(IF,_Memory.MemoryRead(0xFFFF)|0x2);
}