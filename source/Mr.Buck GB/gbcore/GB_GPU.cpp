#include "GB_GPU.h"

void GPU::GPUStep() {
#define Hb 0
#define Vb 1
#define OAM 2
#define VRAM 3
	if (*line > 143) {
		*stat = (*stat) & 0xFC | Vb;
	}
	else {
		GB_BY lastMode = (*stat) & 0x3;
		GB_BY inter = 0;
		if (gpuclock >= 376) {
			*stat = (*stat) & 0xFC | OAM;
			inter = (*stat) & 0x20;
		}
		else if (gpuclock >= 204) {
			*stat = (*stat) & 0xFC | VRAM;
			
		}
		else {
			*stat = (*stat) & 0xFC;
			inter = (*stat) & 0x8;
		}
		if(inter && (lastMode!=((*stat)&0x3)))
			_Memory.MemoryWrite(IF, _Memory.MemoryRead(IF) | 0x2);
	}

	if (gpuclock <= 0) {
		gpuclock = 456;
		(*line)++;
		lcdc_happend = 0;
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
			lcdc_happend = 0;
		}
		
	}
	CheckLCDCInter();
	
}
void GPU::Newline() {
	GB_BY Mask =0;
	GB_BY data = _Memory.MemoryRead(LCDC);
	GB_DB TileSt = ((data&16)>>4?(Mask=0,0):(Mask=128,128));
	GB_DB BGMapNoSt = ((data&8)>>3?0x9C00:0x9800);
	GB_DB WNMapNoSt = ((data&64)>>6?0x9C00:0x9800);
	//start address
	//Tile[X][Y]
	GB_BY _LCDC = _Memory.MemoryRead(LCDC);
	if (_LCDC & 1) {
		UpdateBackGround(BGMapNoSt, TileSt, Mask);
		if ((_LCDC >> 5) & 1) {
			if (*line >= _Memory.MemoryRead(WY))
				UpdateWindow(WNMapNoSt, TileSt, Mask);
		}
	}
	if ((_LCDC >> 1) & 1) {
		UpdateSprite();
	}
}

void GPU::UpdateBackGround(GB_DB MapNoSt,GB_DB TileSt,GB_BY Mask) {
	GB_BY Xoffset = _Memory.MemoryRead(SCX);
	GB_BY Yoffset = _Memory.MemoryRead(SCY);
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
		_Screen[X++][*line] =_Memory.TileSet[((_Memory.MemoryRead(((TileX + 20) & 31) + (GB_DB)TileY * 32 + MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
	}
	
}
void GPU::UpdateWindow(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask) {
	GB_BY Xoffset;
	GB_BY Yoffset;
	GB_BY _WX = _Memory.MemoryRead(WX);
	if (_WX >= 160 && _WX <= 255)return;
	Xoffset = _WX-7;
	Yoffset = _Memory.MemoryRead(WY);
	GB_BY TileY = (((*line)-Yoffset) / 8);
	GB_DB FirstTileNo = TileY*32;
	GB_BY XMax;
	if (Xoffset > 0xF8) {
		XMax = Xoffset;
	}
	else {
		XMax = 160;
	}
	GB_BY MinorYoffset = ((*line) - Yoffset) - TileY * 8;
	GB_BY X = 0;
	
	GB_BY End = 0;
	for (int k = 0; k < 20; k++) {
		for (int i = 0; i < 8; i++) {
			if (Xoffset + X > 0xF8) {
				X++;
			}
			else {
				_Window[Xoffset + X++][*line] = _Memory.TileSet[((_Memory.MemoryRead((k & 31) + (GB_DB)TileY * 32 + MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
				
			}
			if (X + Xoffset >= 160) {
				End = 1;
				break;
			}
		}
		if (End)break;
	}
	if (Xoffset > 0xF8)Xoffset = 0;

	for (int i = Xoffset; i < XMax; i++) {
		_Screen[i][*line] = _Window[i][*line];
	}
}
//read the man for detailed explanation.


void GPU::UpdateSprite() {
	
	memset(_Sprite, -1, sizeof(_Sprite));//-1 means sprite layer part is empty.
	spritelist.clear();

	GB_BY mode8x16 = (_Memory.MemoryRead(LCDC) >> 2) & 1;
	GB_BY PicSize = mode8x16 ? 16 : 8;
	//generate spritelist.
	for (int i = 0; i < 40; i++) {
		sprite.Y = _Memory.MemoryRead(0xFE00 + i * 4);
		sprite.X = _Memory.MemoryRead(0xFE00 + i * 4 + 1);

		if (sprite.Y == 0 && sprite.X == 0||
			((*line)<(sprite.Y-16)||(*line)>((sprite.Y-16)+PicSize-1)))sprite.isRender = 0;
		else {
			sprite.Y -= 16;
			sprite.X -= 8;
			sprite.isRender = 1;
			sprite.TileNo = _Memory.MemoryRead(0xFE00 + i * 4 + 2) &~mode8x16;
			GB_BY flag = _Memory.MemoryRead(0xFE00 + i * 4 + 3);
			sprite.pirority = (flag >> 7) & 1;
			sprite.Yfilp = (flag >> 6) & 1;
			sprite.Xfilp = (flag >> 5) & 1;
			sprite.PlaNo = (flag >> 4) & 1;
			spritelist.push_back(sprite);
			sprite.No = spritelist.size();
			
		}
		if (spritelist.size()==10)break;//max 10 per line
	}
	//sort them,the first will be rendered first,meaning lowest priority. 
	
	sort(spritelist.begin(), spritelist.end(), comp);
	
	GB_BY X;
	GB_BY Second;//used in 8x16 mode
	//render.
	for (auto sp:spritelist) {
		Second = 0;
		
			GB_BY MinorYoffset = *line - sp.Y;
			
			if (sp.Yfilp) {
				MinorYoffset = PicSize-MinorYoffset-1;
			}
			if (MinorYoffset > 7) { Second = 1; MinorYoffset -= 8; }
			
			int j, Step;
		
			if (sp.Xfilp) {
				j = 7;
				Step = -1;
			}
			else {
				j = 0;
				Step = 1;
			}
			for (int n = 0; n < 8;n++) {
				X = n + sp.X;
				
				if (X >= SCREEN_MAX_X) { j += Step; continue; }
				
				_Sprite[X][*line] = _Memory.TileSet[Second+(sp.TileNo)][j][MinorYoffset];
				if (sp.pirority) {
					if (_Screen[X][*line] == 0 && _Sprite[X][*line] != 0) {
						if (sp.PlaNo)
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 4;
						else
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 2;
					}
				}
				else {
					if (_Sprite[X][*line] != 0) {
						if (sp.PlaNo)
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 4;
						else
							_Screen[X][*line] = (1 + _Sprite[X][*line]) << 2;
					}
				}
				j += Step;
			}
		}
}

void GPU::NewFrame() {
	
	//[X][Y]
	GB_BY _BGP = _Memory.MemoryRead(BGP);
	GB_BY OBP[2] = { _Memory.MemoryRead(OBP0) ,_Memory.MemoryRead(OBP1) };

	for (int i = 0; i < SCREEN_MAX_X; i++) {
		for (int j = 0; j < SCREEN_MAX_Y; j++) {
			if (_Screen[i][j] < 4)
				
				_Screen[i][j] = (_BGP >> (2 * _Screen[i][j])) & 3;
			else
				if(_Screen[i][j]<=16)
				_Screen[i][j] = (OBP[0]>>(2*((_Screen[i][j] >> 2) - 1)))&3;
				else
				_Screen[i][j] = (OBP[1]>>(2*((_Screen[i][j] >> 4) - 1))) & 3;
		}
	}
	SetNewFrameFlag(1);
	
}
inline void GPU::CheckLCDCInter() {
	if (lcdc_happend) return;
	if (_Memory._memoryMapio[LYC-0xFF00] == _Memory._memoryMapio[LY - 0xFF00]) {
		_Memory._memoryMapio[STAT - 0xFF00] |= 0x4;
		if (_Memory._memoryMapio[STAT - 0xFF00] & 0x40) {
			_Memory._memoryMapio[0xF] |= 0x2;
			lcdc_happend = 1;
		}
	}
	else {
		_Memory._memoryMapio[STAT - 0xFF00] &= 0xFB;
	}
}
bool GPU::comp(const Sprite& a, const Sprite& b) {
	if (a.X==b.X && a.Y==b.Y && a.No==b.No)return false;
	if (a.Y > b.Y) {return false;}
	else {
		if (a.X <= b.X) {
			if (a.X != b.X) {return false;}
			else {
				if (a.No < b.No) {return false;}

			}
		}
	}
	return true;
}