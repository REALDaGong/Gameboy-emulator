#include"GB.h"
#include"GB_CPU.h"
#include <conio.h>
#define DEBUG
#ifdef DEBUG
void DrawPicture();
void Key(char a);
#endif // DEBUG

int PAUSE=0;
Memory memory;
Timer timer(memory);
GPU gpu(memory);
Z80 cpu(memory, gpu, timer);
int Gameloop();
int main() {
	Gameloop();
	return 0;
}

int Gameloop() {
	char a;
	//freopen("out.txt","w",stdout);
	PAUSE = 0;
	for (int i=0;i<0x100000;i++) {
		//if (kbhit()) {
		//	a = getch();
		//	Key(a);
		//}
		cpu.Step();
	}
	
	DrawPicture();
	//fclose(stdout);
	return 0;
}
#ifdef DEBUG
void DrawPicture() {
	cout << "drawed.";
	fstream out("out.ppm",ios_base::out|ios_base::binary);
	
	
	out << "P6 256 256 255" << endl;
	unsigned char color[4] = {0,85,170,255};
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			out.write((char*)&color[gpu._Screen[j][i]], 1);
			out.write((char*)&color[gpu._Screen[j][i]], 1);
			out.write((char*)&color[gpu._Screen[j][i]], 1);
		}
	}
	
	out.close();
}
void Key(char a) {
	switch (a)
	{
	case 'i'://up
		break;
	case 'k'://down
		break;
	case 'j'://left
		break;
	case 'l'://right
		break;
	case 'z'://a
		break;
	case 'x'://b
		break;
	case 'q'://select
		break;
	case 'w'://back
		break;
	default:
		break;
	}
}
#endif // DEBUG

