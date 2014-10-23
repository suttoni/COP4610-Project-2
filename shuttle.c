#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");

//Stubs
extern long (* STUB_start_shuttle)(void);
extern long (* STUB_stop_shuttle)(void);
extern long (* STUB_request_shuttle)(char passenger_type, int initial_terminal, int destination_terminal);

//Data
typedef enum{
	OFFLINE = 0,
	DEACTIVATING = 1,
	PARKED = 2,
	MOVING = 3
}  shuttle_status;

struct passenger_data{
	char type;
	char initial_terminal;
	char destination;
};

struct shuttle_data{
	shuttle_status status;
	int current_terminal;
	int destination_terminal;
	float seats_used;
};

struct terminal_data{
	int queue_size;
	struct list_head queue; 
};

//Constants and Globals
const int total_seats = 25;
struct shuttle_data shuttle;

long start_shuttle(void){
	if(shuttle.status == OFFLINE){
		printk(KERN_ALERT "Shuttle starting.");
		shuttle.status = PARKED;
		shuttle.current_terminal = 3;
		shuttle.seats_used = 0;
		return 0;
	}
	else{
		printk(KERN_ALERT "Shuttle already started.");
	}
	return -1;
}

long issue_request(char passenger_type, int initial_terminal, int destination_terminal){
	printk(KERN_ALERT "Request issued.");
	return 0;
}

long stop_shuttle(void){
	printk(KERN_ALERT "Shuttle stopping.");
	return 0;
}


int shuttle_show(struct seq_file *m, void *v) {
	seq_printf(m, "Status:\t\nSeats:\t\nPassengers:\nLocation:\nDestination:\nDelivered:\n------\nTermianl 1:\nTerminal 2:\nTerminal 3:\nTerminal 4:\nTerminal 5:\n");
	return 0;
}


int shuttle_open(struct inode *inode, struct  file *file) {
  	return single_open(file, shuttle_show, NULL);
}

const struct file_operations shuttle_fops = {
  	.owner = THIS_MODULE,
  	.open = shuttle_open,
  	.read = seq_read,
  	.release = single_release,
};

int __init shuttle_init(void) {
	STUB_start_shuttle = &start_shuttle;
	STUB_stop_shuttle = &stop_shuttle;
	STUB_request_shuttle = &issue_request;

  	proc_create("terminal", 0, NULL, &shuttle_fops);
  	return 0;
}

void __exit shuttle_exit(void) {
	STUB_start_shuttle = NULL;
	STUB_stop_shuttle = NULL;
	STUB_request_shuttle = NULL;

  	remove_proc_entry("current_date", NULL);
}

module_init(shuttle_init);
module_exit(shuttle_exit);
