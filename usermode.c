#include <unistd.h>

int main(){
	syscall(321);
	syscall(323, 'A', 1, 3);
	syscall(322);
	return 0;
}
