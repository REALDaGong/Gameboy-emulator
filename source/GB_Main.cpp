#include"GB.h"
#include"GB_CPU.h"
#include <conio.h>
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
	while (1) {
		cpu.Step();
	}
}