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
			
			if (*line == 144)//?143 or 144?
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
	GB_DB TileSt = ((data&16)>>4?(Mask=0,0):(Mask=128,128));
	GB_DB BGMapNoSt = ((data&8)>>3?0x9C00:0x9800);
	GB_DB WNMapNoSt = ((data&64)>>6?0x9C00:0x9800);
	//start address
	//Tile[X][Y]
	TransferScreen(BGMapNoSt, TileSt, Mask);
	if ((_Memory.MemoryRead(LCDC) >> 5) & 1) {
		if (*line >= _Memory.MemoryRead(WY))
			TransferWindow(WNMapNoSt, TileSt, Mask);
	}
}
//pay attention, its hard to understand though.......
void GPU::TransferScreen(GB_DB MapNoSt,GB_DB TileSt,GB_BY Mask) {
	
	
	GB_BY Xoffset;
	GB_BY Yoffset;
	
	Xoffset = _Memory.MemoryRead(SCX);
	Yoffset = _Memory.MemoryRead(SCY);

	
	GB_BY TileX= Xoffset / 8;
	GB_BY TileY = ((((*line) + Yoffset) & 255) / 8);
	GB_DB FirstTileNo = ((((*line)+Yoffset)&255)/8)*32+Xoffset/8;//get the first(left up corner tile)'s No.
	GB_BY MinorXoffset = Xoffset - TileX*8;
	GB_BY MinorYoffset = Yoffset+(*line) - TileY*8;
	GB_BY X = 0;
	for (int i = MinorXoffset; i < 8; i++) {
		_Screen[X++][*line] = _Memory.TileSet[((_Memory.MemoryRead(FirstTileNo+MapNoSt)+Mask)&0xff)+TileSt][i][MinorYoffset];
	}
	for (int k = 1; k < 20; k++) {
		for (int i = 0; i < 8; i++) {
			_Screen[X++][*line] = _Memory.TileSet[((_Memory.MemoryRead(((TileX+k)&31)+(GB_DB)TileY*32+MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
		}
	}
	for (int i = 0; i < MinorXoffset; i++) {
		_Screen[X++][*line] = _Memory.TileSet[((_Memory.MemoryRead(((TileX + 20) & 31) + (GB_DB)TileY * 32 + MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
	}
	
}
void GPU::TransferWindow(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask) {
	GB_BY Xoffset;
	GB_BY Yoffset;

	Xoffset = _Memory.MemoryRead(WX)-7;
	Yoffset = _Memory.MemoryRead(WY);


	GB_BY TileX = Xoffset / 8;
	GB_BY TileY = (((*line)-Yoffset) / 8);
	GB_DB FirstTileNo = TileY*32;
	GB_BY MinorXoffset = Xoffset - TileX * 8;
	GB_BY MinorYoffset = ((*line) - Yoffset) - TileY * 8;
	GB_BY X = 0;
	
	for (int k = 0; k < 20; k++) {
		for (int i = 0; i < 8; i++) {
			_Window[Xoffset + X++][*line] = _Memory.TileSet[((_Memory.MemoryRead(((TileX + k) & 31) + (GB_DB)TileY * 32 + MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
			if (Xoffset != 0 && X + Xoffset == 0) {
				return;
			}
		}
	}
	
}
//read the man for detailed explanation.
//initial a frame without taking X and Y offset into consideration.
//havn't merge bg,wn and sp together.
void GPU::UpgradeSprite() {
	memset(_Sprite, -1, sizeof(_Sprite));//-1 means sprite layer is empty.
	GB_BY mode8x16 = (_Memory.MemoryRead(LCDC) >> 2) & 1;
	
	memset(sprite, 0, sizeof(sprite));
	memset(ptrSprite, 0, sizeof(ptrSprite));
	int Rendersize = 0;
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
	//miss the proper process on prority.
	GB_BY PicSize = mode8x16 ? 16 : 8;
	for (int i = 0; i < 40;i++) {
		if (ptrSprite[i]) {
			GB_DB Place = 0x8000+ptrSprite[i]->No*PicSize*2;
			if (ptrSprite[i]->Xfilp || ptrSprite[i]->Yfilp) {
				if (ptrSprite[i]->Xfilp && ptrSprite[i]->Yfilp) {
					for (int k = 0; k < PicSize; k++) {
						GB_BY byte1 = _Memory.MemoryRead(Place + 2 * (PicSize - 1 - k));
						GB_BY byte2 = _Memory.MemoryRead(Place + 2 * (PicSize - 1 - k) + 1);
						for (int j = 0; j < 8; j++) {
							GB_BY X = (j + ptrSprite[i]->X) % 256;
							GB_BY Y = (k + ptrSprite[i]->Y) % 256;
							_Sprite[X][Y] = (_Memory.MemoryRead((ptrSprite[i]->PlaNo) + 0xFF48)
								>> (2 * (((byte1 >> j) & 1) + ((byte2 >> j) & 1) * 2))) & 3;
							if (ptrSprite[i]->pirority) {
								if (_Screen[X][Y] == 0 && _Sprite[X][Y] != 0)
									_Screen[X][Y] = _Sprite[X][Y];
							}
							else {
								if (_Sprite[X][Y] != 0)
									_Screen[X][Y] = _Sprite[X][Y];
							}
						}

					}
				}else
				if (ptrSprite[i]->Xfilp) {
					for (int k = 0; k < PicSize; k++) {
						GB_BY byte1 = _Memory.MemoryRead(Place + 2 * k);
						GB_BY byte2 = _Memory.MemoryRead(Place + 2 * k + 1);
						for (int j = 0; j < 8; j++) {
							GB_BY X = (j + ptrSprite[i]->X) % 256;
							GB_BY Y = (k + ptrSprite[i]->Y) % 256;
							_Sprite[X][Y] = (_Memory.MemoryRead((ptrSprite[i]->PlaNo) + 0xFF48)
								>> (2 * (((byte1 >> j) & 1) + ((byte2 >> j) & 1) * 2))) & 3;
							if (ptrSprite[i]->pirority) {
								if(_Screen[X][Y]==0 && _Sprite[X][Y] != 0)
								_Screen[X][Y] = _Sprite[X][Y];
							}
							else {
								if (_Sprite[X][Y] != 0)
								_Screen[X][Y] = _Sprite[X][Y];
							}
						}

					}
				}
				else {
					for (int k = 0; k < PicSize; k++) {
						GB_BY byte1 = _Memory.MemoryRead(Place + 2 * (PicSize-1-k));
						GB_BY byte2 = _Memory.MemoryRead(Place + 2 * (PicSize-1-k)+1);
						for (int j = 0; j < 8; j++) {
							GB_BY X = (j + ptrSprite[i]->X) % 256;
							GB_BY Y = (k + ptrSprite[i]->Y) % 256;
							_Sprite[X][Y] = (_Memory.MemoryRead((ptrSprite[i]->PlaNo) + 0xFF48)
								>> (2 * (((byte1 >> (7-j)) & 1) + ((byte2 >>(7 - j)) & 1) * 2))) & 3;
							if (ptrSprite[i]->pirority) {
								if (_Screen[X][Y] == 0&& _Sprite[X][Y]!=0)
									_Screen[X][Y] = _Sprite[X][Y];
							}
							else {
								if(_Sprite[X][Y] != 0)
								_Screen[X][Y] = _Sprite[X][Y];
							}
						}

					}
				}
			}
			else {
				for (int k = 0; k < PicSize; k++) {
					GB_BY byte1 = _Memory.MemoryRead(Place + 2 * k);
					GB_BY byte2 = _Memory.MemoryRead(Place + 2 * k + 1);
					for (int j = 0; j < 8; j++) {
						GB_BY X = (j + ptrSprite[i]->X) % 256;
						GB_BY Y = (k + ptrSprite[i]->Y) % 256;
						_Sprite[X][Y] = (_Memory.MemoryRead((ptrSprite[i]->PlaNo) + 0xFF48)
							>> (2 * (((byte1 >> (7-j)) & 1) + ((byte2 >>(7- j)) & 1) * 2))) & 3;
						if (ptrSprite[i]->pirority) {
							if (_Screen[X][Y] == 0 && _Sprite[X][Y] != 0)
								_Screen[X][Y] = _Sprite[X][Y];
						}
						else {
							if (_Sprite[X][Y] != 0)
							_Screen[X][Y] = _Sprite[X][Y];
						}
					}

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
	if ((_Memory.MemoryRead(LCDC) >> 5) & 1) {
		GB_BY X = _Memory.MemoryRead(WX) - 7;
		GB_BY Y = _Memory.MemoryRead(WY);
		for (int i = X; i < 160; i++) {
			for (int j = Y; j < 144; j++) {
				_Screen[i][j] = _Window[i][j];
			}
		}
	}
	SetNewFrameFlag(1);
	//do it!
	//dont let your dream be dream!
	//yesterday you say tomorrow.
	//
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
