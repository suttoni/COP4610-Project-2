#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/export.h>

//Define stubs for calls
long (* STUB_start_shuttle)(void) = NULL;
long (* STUB_stop_shuttle)(void) = NULL;
long (* STUB_request_shuttle)(char passenger_type, int initial_terminal, int destination_terminal) = NULL;

//Export them
EXPORT_SYMBOL(STUB_start_shuttle);
EXPORT_SYMBOL(STUB_stop_shuttle);
EXPORT_SYMBOL(STUB_request_shuttle);

asmlinkage long sys_start_shuttle(void){
	if(STUB_start_shuttle){
		return STUB_start_shuttle();
	}
	printk(KERN_WARNING "Can't use start_shuttle syscall without loading the kernel module.\n");
	return -ENOSYS;
}

asmlinkage long sys_stop_shuttle(void){
	if(STUB_stop_shuttle){
		return STUB_stop_shuttle();
	}
	printk(KERN_WARNING "Can't use stop_shuttle syscall without loading the kernel module.\n");
	return -ENOSYS;
}

asmlinkage long sys_request_shuttle(char passenger_type, int initial_terminal, int destination_terminal){
	if(STUB_request_shuttle){
		return STUB_request_shuttle(passenger_type, initial_terminal, destination_terminal);
	}
	printk(KERN_WARNING "Can't use request_shuttle syscall without loading the kernel module.\n");
	return -ENOSYS;
}
