#include"GB.h"
#include"GB_CPU.h"
#include <conio.h>
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
	for (int i=0;i<0x1000000;i++) {
		cpu.Step();
	
	}
	getch();

	for (int i = 0; i<0x100000; i++) {
		cpu.Step();

	}
	return 0;

}