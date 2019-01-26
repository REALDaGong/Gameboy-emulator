#pragma once
#include "GB.h"
class GPU:public Hardware 
{
	typedef uint32_t GPU_CLOCK;
	typedef GB_BY Pixel;

public:
	Pixel _Screen[SCREEN_MAX_X][SCREEN_MAX_Y];
	Pixel _Sprite[SCREEN_MAX_X][SCREEN_MAX_Y];
	Pixel _Window[SCREEN_MAX_X][SCREEN_MAX_Y];
	GPU(Hardware *interHandler) : _InterHandler(interHandler) {
		Init();
	};
	~GPU() {};
	void Init() {
		memset(_Screen, 0, sizeof(_Screen));
		memset(_Sprite, 0, sizeof(_Sprite));
		memset(_Window, 0, sizeof(_Window));
		memset(TileSet, 0, sizeof(TileSet));
		memset(VRam, 0, sizeof(VRam));
		memset(Oam, 0, sizeof(Oam));
		gpuclock = 456;
		NewFrameFlag = 0;
		lcdc_happend = 0;
		_LCDC = _WX = _WY=_LYC=_SCX=_SCY=_STAT=_OBP[0]=_OBP[1]=_BGP=_DMA=_LY = 0;
		spritelist.empty();
	}
	void AddClock(GPU_CLOCK delta) {
		if (Power) {
			gpuclock -= delta;
			GPUStep(delta);
		}
	}

	inline GB_BY GPURead(GB_DB ad) {
		if (ad < 0xA000) {
			if ((_STAT & 0x3) != 3 || _DMA)
				return VRam[ad - 0x8000];
			else
				return 0xFF;
		}
		else if(ad<0xFEA0){
			if ((_STAT & 0x3) == 0 || (_STAT & 0x3) == 1 || _DMA)
				return Oam[ad & 0xFF];
			else
				return 0xFF;
		}
		else {
			switch (ad) {
			case LY:
				return _LY;
				break;
			case LYC:
				return _LYC;
				break;
			case STAT:
				return _STAT|0x80;
				break;
			case LCDC:
				return _LCDC;
				break;
			case WX:
				return _WX;
				break;
			case WY:
				return _WY;
				break;
			case SCX:
				return _SCX;
				break;
			case SCY:
				return _SCY;
				break;
			case OBP0:
				return _OBP[0];
				break;
			case OBP1:
				return _OBP[1];
				break;
			case BGP:
				return _BGP;
				break;
			default:
				return 0xFF;//DMA still in memory part.
			}
		}
	}
	inline void GPUWrite(GB_DB ad,GB_BY val) {
		if (ad < 0xA000) {
			if ((_STAT & 0x3) != 3  || (_LCDC & 0x80) == 0) {
				VRam[ad - 0x8000] = val;
				if (ad <= 0x97FF) {
					UpdateTile(ad);
				}
			}
		}
		else if (ad<0xFEA0) {
			if ((_STAT & 0x3) == 0 || (_STAT & 0x3) == 1  || (_LCDC & 0x80) == 0) {
				Oam[ad & 0xFF] = val;
			}
		}
		else {
			switch (ad) {
			case LY:
				_LY=0;
				break;
			case LYC:
				_LYC=val;
				lcdc_happend = 0;//if LYC not changed?
				break;
			case STAT:
				_STAT = (_STAT & 0x7) | (val&0xFC);
				/*if ((val & 0x7) != 0) {
					val &= ~0x7;
				}
				if (val & 0x80) {
					_STAT = (_STAT & 0x7) + val;
				}
				else if (val & 0x40) {
					_STAT = (_STAT & 0x87) + val;
				}
				else if (val & 0x20) {
					_STAT = (_STAT & 0xC7) + val;
				}
				else if (val & 0x10) {
					_STAT = (_STAT & 0xA7) + val;
				}
				else if (val & 0x8) {
					_STAT = (_STAT & 0xF7) + val;
				}*/
				break;
			case LCDC:
				_LCDC=val;
				if ((val & 0x80) == 0) {
					if (Power == 1) {
						LCDoff();
					}
				}
				else {
					if (Power == 0) {
						LCDon();
					}
				}
				break;
			case WX:
				_WX=val;
				break;
			case WY:
				_WY=val;
				break;
			case SCX:
				_SCX=val;
				break;
			case SCY:
				_SCY=val;
				break;
			case OBP0:
				_OBP[0]=val;
				break;
			case OBP1:
				_OBP[1]=val;
				break;
			case BGP:
				_BGP = val;
				break;
			case DMA:
				_DMA = val;
				break;
			default:
				break;
			}
		}
	}
	inline void SetNewFrameFlag() {NewFrameFlag = 1;}
	inline void ResetNewFrameFlag() { NewFrameFlag = 0; }
	inline int GetNewFrameFlag() {return NewFrameFlag;}
	inline void Send(GB_BY nothing){}
private:
	
	int gpuclock;
	void GPUStep(int delta);
	Hardware* _InterHandler;
	GB_BY lcdc_happend;
	int justclosed;
	GB_BY Power;

	GB_BY _LY;
	GB_BY _LCDC;
	GB_BY _STAT;
	GB_BY _LYC;
	GB_BY _WX,_WY;
	
	GB_BY _SCX, _SCY;
	GB_BY VRam[0x2000];
	GB_BY Oam[0xA0];
	GB_BY _OAM[0xA0];
	GB_BY TileSet[384][8][8];
	GB_BY _OBP[2],_BGP;
	GB_BY _DMA;//only used for mark DMA start or not,DMA can access ram at anytime.
	int NewFrameFlag;

	void Newline();
	void DrawBackGround(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask);
	void DrawWindow(GB_DB MapNoSt, GB_DB TileSt, GB_BY Mask);
	void DrawSprite();
	void UpdateSprite(int delta);//OAM entry happens every 2 clocks.
	void ColorLine();
	void CheckLCDCInter();

	void LCDoff();
	//LCD ONLY can be stopped mannually during VBLANK mode,performing at any other time
	//will damage the gameboy hardware badly,for example,a LCD line may be burn out and become black forever.
	//i'm trying to implement this feature in my emulator.
	void LCDon();
	
	void UpdateTile(GB_DB ad);//pre-tranlate the tile data into images,making GPU work faster.

	struct Sprite {
		GB_BY X, Y, TileNo;
		GB_BY pirority;
		GB_BY Xfilp, Yfilp;
		GB_BY PlaNo;
		GB_BY isRender;
		GB_BY No;
	}sprite;

	vector<Sprite>spritelist;

	static bool comp(const Sprite&a, const Sprite&b);
};
