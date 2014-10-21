#include <linux/kernel.h>
#inlcude <linux/syscalls.h>

asmlinkage long sys_start_shuttle(void){
	printk(KERN_ALERT "Shuttle starting.");
	return 0;
}

asmlinkage long sys_stop_shuttle(void){
	printk(KERN_ALERT "Shuttle stopping.");
	return 0;
}

asmlinkage long sys_request_shuttle(char passenger_type, int initial_terminal, int destination_terminal){
	printk(KERN_ALERT "Request issued.");
	return 0;
}
