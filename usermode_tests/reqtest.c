#include <unistd.h>	

int main(void){

	syscall(323, 'A', 1, 5);
	syscall(323, 'L', 1, 5);
	syscall(323, 'C', 1, 5);
	syscall(323, 'C', 1, 5);
	syscall(323, 'A', 1, 2);
	syscall(323, 'A', 1, 4);
	syscall(323, 'A', 1, 3);
	syscall(323, 'A', 3, 1);
	syscall(323, 'C', 3, 1);

	return 0;
}
