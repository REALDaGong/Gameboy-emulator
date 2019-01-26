#include "GB_GPU.h"
void GPU::LCDoff() {
	Power = 0;
	
	_STAT &= 0xFC;
	gpuclock = 456;
	//should other registers need to be write in zero?
	memset(_Screen, 0, sizeof(_Screen));
	memset(_Sprite, 0, sizeof(_Sprite));
	memset(_Window, 0, sizeof(_Window));
	_InterHandler->Send(INTERRUPT_VBLANK);//?
}
void GPU::LCDon() {
	Power = 1;
	_LY = 0;
	lcdc_happend = 0;
	CheckLCDCInter();//?
}
void GPU::GPUStep(int delta) {
#define Hb 0
#define Vb 1
#define OAM 2
#define VRAM 3//if accurate emulation:line drawing should only happen in mode 3(and 2?),so the actually clocks consumed varies.
	if(_LY<144)
	{
		GB_BY lastMode = _STAT & 0x3;
		GB_BY inter = 0;
		if (gpuclock >= 376) {//376
			
			_STAT = _STAT & 0xFC | OAM;//last exactly 80 clks.
			inter = _STAT & 0x20;
			//UpdateSprite(delta);
		}
		else if (gpuclock >= 206) {//204 if you choose generate a new line after all 456 cycles.
			//UpdateSprite(delta);
			_STAT = _STAT & 0xFC | VRAM;//this state will last about 170-240 clks depends on where the sprite is,i choose 376-240(most)=136.
										//wrong!some sprites will blink when they are overlapping!
										//Nitty Gritty mentioned it should be 173.5 to 187.5 ?
										//i dont want to emulate this far.....
			
		}
		else {//last for 200clks most.

			
			_STAT = _STAT & 0xFC;
			inter = _STAT & 0x8;
			if (lastMode != (_STAT & 0x3) && _LY<144)Newline();//if you choose generate a new line after all 456 cycles,some 
															  //games with complex graphic would not be displayed successfully
		}
		if (inter && (lastMode != (_STAT & 0x3)))
			_InterHandler->Send(INTERRUPT_LCDC);
	}

	if (gpuclock <= 0) {
		gpuclock += 456;
		_LY++;
		lcdc_happend = 0;
		if (_LY == 144) {
			_InterHandler->Send(INTERRUPT_VBLANK);
			_STAT = _STAT & 0xFC | Vb;
			if (_STAT & 0x20) {
				_InterHandler->Send(INTERRUPT_LCDC);
			}
		}else
		if (_LY == 154) {
			SetNewFrameFlag();
			_LY = 0;
			lcdc_happend = 0;
		}
		CheckLCDCInter();
	}
	
	
}
void GPU::Newline() {
	GB_BY Mask =0;
	GB_DB TileSt = ((_LCDC&16)>>4?(Mask=0,0):(Mask=128,128));
	GB_DB BGMapNoSt = ((_LCDC&8)>>3?0x9C00:0x9800);
	GB_DB WNMapNoSt = ((_LCDC&64)>>6?0x9C00:0x9800);
	//start address
	//Tile[X][Y]
	if (_LCDC & 1) {
		DrawBackGround(BGMapNoSt, TileSt, Mask);
		if ((_LCDC >> 5) & 1) {
			if (_LY >= _WY)
				DrawWindow(WNMapNoSt, TileSt, Mask);
		}
	}
	if (((_LCDC >> 1) & 1)) {
		DrawSprite();
	}
	ColorLine();
}

void GPU::DrawBackGround(GB_DB MapNoSt,GB_DB TileSt,GB_BY Mask) {
	GB_BY Xoffset = _SCX;
	GB_BY Yoffset = _SCY;
	GB_BY TileX= Xoffset / 8;
	GB_BY TileY = (((_LY + Yoffset) & 255) / 8);
	GB_DB FirstTileNo = (((_LY+Yoffset)&255)/8)*32+Xoffset/8;//get the first(left up corner tile)'s No.
	GB_BY MinorXoffset = Xoffset - TileX*8;
	GB_BY MinorYoffset = Yoffset+_LY - TileY*8;
	GB_BY X = 0;

	for (int i = MinorXoffset; i < 8; i++) {
		_Screen[X++][_LY] = TileSet[((GPURead(FirstTileNo+MapNoSt)+Mask)&0xff)+TileSt][i][MinorYoffset];
	}
	for (int k = 1; k < 20; k++) {
		for (int i = 0; i < 8; i++) {
			_Screen[X++][_LY] = TileSet[((GPURead(((TileX+k)&31)+(GB_DB)TileY*32+MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
		}
	}
	for (int i = 0; i < MinorXoffset; i++) {
		_Screen[X++][_LY] =TileSet[((GPURead(((TileX + 20) & 31) + (GB_DB)TileY * 32 + MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
	}
	
}
void GPU::DrawWindow(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask) {
	GB_BY Xoffset;
	GB_BY Yoffset;
	if (_WX >= 160 && _WX <= 255)return;
	Xoffset = _WX-7;
	Yoffset = _WY;
	GB_BY TileY = ((_LY-Yoffset) / 8);
	GB_DB FirstTileNo = TileY*32;
	GB_BY XMax;
	if (Xoffset > 0xF8) {
		XMax = Xoffset;
	}
	else {
		XMax = 160;
	}
	GB_BY MinorYoffset = (_LY - Yoffset) - TileY * 8;
	GB_BY X = 0;
	
	GB_BY End = 0;
	for (int k = 0; k < 20; k++) {
		for (int i = 0; i < 8; i++) {
			if (Xoffset + X > 0xF8) {
				X++;
			}
			else {
				_Window[Xoffset + X++][_LY] = TileSet[((GPURead((k & 31) + (GB_DB)TileY * 32 + MapNoSt) + Mask) & 0xff) + TileSt][i][MinorYoffset];
				
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
		_Screen[i][_LY] = _Window[i][_LY];
	}
}
//read the man for detailed explanation.
void GPU::UpdateSprite(int delta) {
	//logically,Spritelist should be updated here,but i just copy the current data.
	
	static int i = 0;
	delta >>= 1;
	if (delta == -1) {
		i = 0;
		return;//end.
	}
	while (delta != 0) {
		_OAM[i * 4] = Oam[i * 4];
		_OAM[i * 4+1] = Oam[i * 4+1];
		_OAM[i * 4+1] = Oam[i * 4+1];
		_OAM[i * 4+1] = Oam[i * 4+1];
		i++; 
		if (i == 40) {
			i = 0;
			return;
		}
	}

}

void GPU::DrawSprite() {
	
	memset(_Sprite, -1, sizeof(_Sprite));//-1 means sprite layer part is empty.
	//seems it just use one line,if I confirm it i will delete the unused part.

	spritelist.clear();

	GB_BY mode8x16 = (_LCDC >> 2) & 1;
	GB_BY PicSize = mode8x16 ? 16 : 8;
	//generate spritelist.
	for (int i = 0; i < 40; i++) {
		sprite.Y =Oam[i * 4];
		sprite.X =Oam[i * 4 + 1];
		if (sprite.Y == 0 && sprite.X == 0||
			(_LY<(sprite.Y-16)||_LY>((sprite.Y-16)+PicSize-1)))sprite.isRender = 0;
		else {
			sprite.Y -= 16;
			sprite.X -= 8;
			sprite.isRender = 1;
			//sprite.TileNo = GPURead(0xFE00 + i * 4 + 2) &~mode8x16;
			//GB_BY flag = GPURead(0xFE00 + i * 4 + 3);
			sprite.TileNo = Oam[i * 4 + 2] &~mode8x16;
			GB_BY flag = Oam[i * 4 + 3];
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
		
			GB_BY MinorYoffset = _LY - sp.Y;
			
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
				
				_Sprite[X][_LY] = TileSet[Second+(sp.TileNo)][j][MinorYoffset];
				if (sp.pirority) {
					if (_Screen[X][_LY] == 0 && _Sprite[X][_LY] != 0) {
						if (sp.PlaNo)
							_Screen[X][_LY] = (1 + _Sprite[X][_LY]) << 4;
						else
							_Screen[X][_LY] = (1 + _Sprite[X][_LY]) << 2;
					}
				}
				else {
					if (_Sprite[X][_LY] != 0) {
						if (sp.PlaNo)
							_Screen[X][_LY] = (1 + _Sprite[X][_LY]) << 4;
						else
							_Screen[X][_LY] = (1 + _Sprite[X][_LY]) << 2;
					}
				}
				j += Step;
			}
		}
}
void GPU::ColorLine() {
	//[X][Y]
	for (int i = 0; i < SCREEN_MAX_X; i++) {
		//for (int j = 0; j < SCREEN_MAX_Y; j++) {
			if (_Screen[i][_LY] < 4)

				_Screen[i][_LY] = (_BGP >> (2 * _Screen[i][_LY])) & 3;
			else
				if (_Screen[i][_LY] <= 16)//means this pixel is a sprite pixel. bit4 bit3:sprite, bit2 bit1:window and back
					_Screen[i][_LY] = (_OBP[0] >> (2 * ((_Screen[i][_LY] >> 2) - 1))) & 3;
				else
					_Screen[i][_LY] = (_OBP[1] >> (2 * ((_Screen[i][_LY] >> 4) - 1))) & 3;
		//}
	}
}
inline void GPU::CheckLCDCInter() {
	if (lcdc_happend) return;
	if (_LYC == _LY) {
		_STAT |= 0x4;
		if (_STAT & 0x40) {
			_InterHandler->Send(INTERRUPT_LCDC);
			lcdc_happend = 1;
		}
	}
	else {
		_STAT &= 0xFB;
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
void GPU::UpdateTile(GB_DB ad) {
	GB_DB TileNo = (ad - 0x8000) / 16;

	for (int i = 0; i < 8; i++) {
		GB_BY Byte1 = VRam[TileNo * 16 + i * 2];
		GB_BY Byte2 = VRam[TileNo * 16 + i * 2 + 1];
		for (int j = 0; j < 8; j++) {
			TileSet[TileNo][j][i] = ((Byte1 >> (7 - j)) & 1) + ((Byte2 >> (7 - j)) & 1) * 2;
		}
	}
}