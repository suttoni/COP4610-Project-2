#include <unistd.h>

int main(){
	syscall(321);
	syscall(323, 'A', 1, 3);
	syscall(323, 'L', 1, 5);
	syscall(323, 'C', 2, 1);
	syscall(322);
	return 0;
}
