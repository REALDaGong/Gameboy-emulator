#include"GB.h"
#include"GB_CPU.h"
#include <conio.h>
#define DEBUG
#ifdef DEBUG
void DrawPicture();
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
	//freopen("out.txt","w",stdout);
	for (int i=0;i<0x2000000;i++) {
		cpu.Step();
		
	}
	//DrawPicture();
	cout << "1";
	for (int i = 0; i<0x2000000; i++) {
		cpu.Step();

	}
	
	return 0;

}
#ifdef DEBUG
void DrawPicture() {
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
#endif // DEBUG

