#include "GB_GPU.h"

void GPU::GPUStep() {

#define Hb 0
#define Vb 1
#define OAM 2
#define VRAM 3
	if (*line > 143) {
		*stat = (*stat) & 0xFC | Vb;
	} else {
		GB_BY lastMode = (*stat) & 0x3;
		GB_BY inter = 0;
		if (gpuclock >= 376) {
			*stat = (*stat) & 0xFC | OAM;
			inter = (*stat) & 0x20;
		} else if (gpuclock >= 204) {
			*stat = (*stat) & 0xFC | VRAM;

		} else {
			*stat = (*stat) & 0xFC;
			inter = (*stat) & 0x8;
		}
		if (inter && (lastMode != ((*stat) & 0x3)))
			_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF) | 0x2);
	}

	if (gpuclock <= 0) {
		gpuclock = 456;
		(*line)++;
		if (*line == 145) {
			_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF) | 0x1);
			*stat = (*stat) & 0xFC | Vb;
			return;
		}
		if (*line < 145) {
			(*line)--;
			Newline();
			(*line)++;
		}
		if (*line == 153) {
			NewFrame();
			(*line) = 0;
		}

	}
	CheckLCDCInter();

}

void GPU::Newline() {
#define WINDOW 1
#define SCREEN 0
	GB_BY Mask = 0;
	GB_BY data = _Memory.MemoryRead(LCDC);
	GB_DB TileSt = ((data & 16) >> 4 ? (Mask = 0, 0) : (Mask = 128, 128));
	GB_DB BGMapNoSt = ((data & 8) >> 3 ? 0x9C00 : 0x9800);
	GB_DB WNMapNoSt = ((data & 64) >> 6 ? 0x9C00 : 0x9800);
	//start address
	//Tile[X][Y]
	UpdateBackGround(BGMapNoSt, TileSt, Mask);
	if ((_Memory.MemoryRead(LCDC) >> 5) & 1) {
		if (*line >= _Memory.MemoryRead(WY))
			UpdateWindow(WNMapNoSt, TileSt, Mask);
	}
	if ((_Memory.MemoryRead(LCDC) >> 1) & 1) {
		UpdateSprite();
	}
}

void GPU::UpdateBackGround(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask) {


	GB_BY Xoffset;
	GB_BY Yoffset;

	Xoffset = _Memory.MemoryRead(SCX);
	Yoffset = _Memory.MemoryRead(SCY);


	GB_BY TileX = Xoffset / 8;
	GB_BY TileY = ((((*line) + Yoffset) & 255) / 8);
	GB_DB FirstTileNo = ((((*line) + Yoffset) & 255) / 8) * 32 + Xoffset / 8;//get the first(left up corner tile)'s No.
	GB_BY MinorXoffset = Xoffset - TileX * 8;
	GB_BY MinorYoffset = Yoffset + (*line) - TileY * 8;
	GB_BY X = 0;
	for (int i = MinorXoffset; i < 8; i++) {
		_Screen[X++][*line] = _Memory.TileSet[((_Memory.MemoryRead(FirstTileNo + MapNoSt) + Mask) & 0xff) +
											  TileSt][i][MinorYoffset];
	}
	for (int k = 1; k < 20; k++) {
		for (int i = 0; i < 8; i++) {
			_Screen[X++][*line] = _Memory.TileSet[
					((_Memory.MemoryRead(((TileX + k) & 31) + (GB_DB) TileY * 32 + MapNoSt) + Mask) & 0xff) +
					TileSt][i][MinorYoffset];
		}
	}
	for (int i = 0; i < MinorXoffset; i++) {
		_Screen[X++][*line] = _Memory.TileSet[
				((_Memory.MemoryRead(((TileX + 20) & 31) + (GB_DB) TileY * 32 + MapNoSt) + Mask) & 0xff) +
				TileSt][i][MinorYoffset];
	}

}

void GPU::UpdateWindow(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask) {
	GB_BY Xoffset;
	GB_BY Yoffset;

	Xoffset = _Memory.MemoryRead(WX) - 7;
	Yoffset = _Memory.MemoryRead(WY);


	GB_BY TileY = (((*line) - Yoffset) / 8);
	GB_DB FirstTileNo = TileY * 32;

	GB_BY MinorYoffset = ((*line) - Yoffset) - TileY * 8;
	GB_BY X = 0;

	GB_BY End = 0;
	for (int k = 0; k < 20; k++) {
		for (int i = 0; i < 8; i++) {
			_Window[Xoffset + X++][*line] = _Memory.TileSet[
					((_Memory.MemoryRead((k & 31) + (GB_DB) TileY * 32 + MapNoSt) + Mask) & 0xff) +
					TileSt][i][MinorYoffset];
			if (X + Xoffset >= 160) {
				End = 1;
				break;
			}
		}
		if (End)break;
	}
	for (int i = Xoffset; i < SCREEN_MAX_X; i++) {
		_Screen[i][*line] = _Window[i][*line];
	}
}
//read the man for detailed explanation.


void GPU::UpdateSprite() {

	memset(_Sprite, -1, sizeof(_Sprite));//-1 means sprite layer part is empty.
	GB_BY mode8x16 = (_Memory.MemoryRead(LCDC) >> 2) & 1;
	GB_BY PicSize = mode8x16 ? 16 : 8;
	memset(sprite, 0, sizeof(sprite));
	memset(ptrSprite, 0, sizeof(ptrSprite));
	int Rendersize = 0;
	for (int i = 0; i < 40; i++) {
		sprite[i].Y = _Memory.MemoryRead(0xFE00 + i * 4);
		sprite[i].X = _Memory.MemoryRead(0xFE00 + i * 4 + 1);

		if (sprite[i].Y == 0 && sprite[i].X == 0 ||
			((*line) < (sprite[i].Y - 16) || (*line) > ((sprite[i].Y - 16) + PicSize - 1)))
			sprite[i].isRender = 0;
		else {
			sprite[i].Y -= 16;
			sprite[i].X -= 8;
			sprite[i].isRender = 1;
			sprite[i].TileNo = _Memory.MemoryRead(0xFE00 + i * 4 + 2) >> mode8x16;
			GB_BY flag = _Memory.MemoryRead(0xFE00 + i * 4 + 3);
			sprite[i].pirority = (flag >> 7) & 1;
			sprite[i].Yfilp = (flag >> 6) & 1;
			sprite[i].Xfilp = (flag >> 5) & 1;
			sprite[i].PlaNo = (flag >> 4) & 1;
			sprite[i].No = Rendersize;
			ptrSprite[Rendersize++] = &sprite[i];
		}
		if (Rendersize == 10)break;//max 10 per line
	}
	//sort them,the first will be rendered first,meaning lowest priority. 
	Sprite *tmp;
	for (int j = 0; j < Rendersize - 1; j++) {
		for (int i = 0; i < Rendersize - 1; i++) {


			if (ptrSprite[i]->Y > ptrSprite[i + 1]->Y) {
				tmp = ptrSprite[i];
				ptrSprite[i] = ptrSprite[i + 1];
				ptrSprite[i + 1] = tmp;
			} else {
				if (ptrSprite[i]->X <= ptrSprite[i + 1]->X) {
					if (ptrSprite[i]->X != ptrSprite[i + 1]->X) {
						tmp = ptrSprite[i];
						ptrSprite[i] = ptrSprite[i + 1];
						ptrSprite[i + 1] = tmp;
					} else {

						if (ptrSprite[i]->No < ptrSprite[i + 1]->No) {
							tmp = ptrSprite[i];
							ptrSprite[i] = ptrSprite[i + 1];
							ptrSprite[i + 1] = tmp;
						}

					}
				}
			}
		}
	}

	GB_BY X;
	for (int i = 0; i < 10; i++) {
		if (ptrSprite[i]) {
			GB_BY MinorYoffset = *line - ptrSprite[i]->Y;
			if (ptrSprite[i]->Yfilp) {
				MinorYoffset = PicSize - MinorYoffset - 1;
			}
			int j, Step;

			if (ptrSprite[i]->Xfilp) {
				j = 7;
				Step = -1;
			} else {
				j = 0;
				Step = 1;
			}
			for (int n = 0; n < 8; n++) {
				X = n + ptrSprite[i]->X;

				if (X >= SCREEN_MAX_X) {
					j += Step;
					continue;
				}

				_Sprite[X][*line] = _Memory.TileSet[ptrSprite[i]->TileNo][j][MinorYoffset];
				if (ptrSprite[i]->pirority) {
					if (_Screen[X][*line] == 0 && _Sprite[X][*line] != 0) {
						if (ptrSprite[i]->PlaNo)
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 4;
						else
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 2;
					}
				} else {
					if (_Sprite[X][*line] != 0) {
						if (ptrSprite[i]->PlaNo)
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 4;
						else
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 2;
					}
				}
				j += Step;
			}
		}
	}


}

void GPU::NewFrame() {

	//[X][Y]
	GB_BY _BGP = _Memory.MemoryRead(BGP);
	GB_BY OBP[2] = {_Memory.MemoryRead(OBP0), _Memory.MemoryRead(OBP1)};

	for (int i = 0; i < SCREEN_MAX_X; i++) {
		for (int j = 0; j < SCREEN_MAX_Y; j++) {
			if (_Screen[i][j] < 4)

				_Screen[i][j] = (_BGP >> (2 * _Screen[i][j])) & 3;
			else if (_Screen[i][j] <= 16)
				_Screen[i][j] = (OBP[0] >> (2 * ((_Screen[i][j] >> 2) - 1))) & 3;
			else
				_Screen[i][j] = (OBP[1] >> (2 * ((_Screen[i][j] >> 4) - 1))) & 3;
		}
	}
	SetNewFrameFlag(1);

}

inline void GPU::CheckLCDCInter() {
	//if (lcdc_happend) return;
	if (_Memory._memory_mapio[LYC - 0xFF00] == _Memory._memory_mapio[LY - 0xFF00]) {
		_Memory._memory_mapio[STAT - 0xFF00] |= 0x4;
		if (_Memory._memory_mapio[STAT - 0xFF00] & 0x40)
			_Memory._memory_mapio[0xF] |= 0x2;
	} else {
		_Memory._memory_mapio[STAT - 0xFF00] &= 0xFB;
	}

}

inline void GPU::CheckModeInter(GB_BY Mode) {
	if (lcdc_happend) return;
	if ((_Memory._memory_mapio[STAT - 0xFF00] >> (Mode + 3)) & 1) {
		_Memory._memory_mapio[0xF] |= 0x2;
		lcdc_happend = 1;
	}
}
