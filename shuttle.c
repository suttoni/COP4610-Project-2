#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

//Define Stuff
#define TOTAL_SEATS 25
#define NUM_TERMINALS 5

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
	short initial_terminal;
	short destination;
	struct list_head passenger_list;
};

struct shuttle_data{
	shuttle_status status;
	short current_terminal;
	short destination_terminal;
	float seats_used;
};

struct terminal_data{
	int queue_size;
	struct list_head queue; 
};

//Globals
struct shuttle_data shuttle;
struct terminal_data terminal[NUM_TERMINALS];

long start_shuttle(void){
	if(shuttle.status == OFFLINE){
		printk(KERN_ALERT "Shuttle starting.");
		shuttle.status = PARKED;
		shuttle.current_terminal = 3;
		shuttle.seats_used = 0;
		for(int i = 0; i < NUM_TERMINALS; i++){
			terminal[i].queue_size = 0;
			INIT_LIST_HEAD(&terminal[i].queue);
		}
		return 0;
	}
	else{
		printk(KERN_ALERT "Shuttle already started.");
	}
	return -1;
}

long issue_request(char passenger_type, int initial_terminal, int destination_terminal){
	if(((passenger_type == 'A') || (passenger_type == 'C') || (passenger_type == 'L'))
	&& ((initial_terminal >= 1) && (initial_terminal <= 5))
	&& ((destination_terminal >= 1) && (destination_terminal <= 5))){  
		struct passenger_data * passenger = NULL;
		passenger = kmalloc(sizeof(struct passenger_data), GFP_KERNEL);
		passenger->type = passenger_type;
		passenger->initial_terminal = initial_terminal;
		passenger->destination = destination_terminal;
		INIT_LIST_HEAD(&passenger->passenger_list);
		list_add_tail(&passenger->passenger_list, &terminal[passenger->initial_terminal-1].queue);
		//terminal[passenger.initial_terminal].queue
		terminal[passenger->initial_terminal-1].queue_size++;
		printk(KERN_ALERT "Request issued.");
		return 0;
	}
	else{
		printk(KERN_ALERT "Invalid passenger information. %c %d %d", passenger_type, initial_terminal, destination_terminal);
	}
	return -1;
}

long stop_shuttle(void){
	shuttle.status = OFFLINE; //Obviously this is dirty
	printk(KERN_ALERT "Shuttle stopping.");
	return 0;
}


int shuttle_show(struct seq_file *m, void *v) {
	seq_printf(m, "Status:\t\nSeats:\t\nPassengers:\nLocation:\nDestination:\nDelivered:\n------\nTerminal 1:\nTerminal 2:\nTerminal 3:\nTerminal 4:\nTerminal 5:\n");
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

  	remove_proc_entry("terminal", NULL);
}

module_init(shuttle_init);
module_exit(shuttle_exit);
