#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

//Stubs
extern long (* STUB_start_shuttle)(void);
extern long (* STUB_stop_shuttle)(void);
extern long (* STUB_request_shuttle)(char passenger_type, int initial_terminal, int destination_terminal);

static int start_shuttle(void){
	printk(KERN_ALERT "Shuttle starting.");
	return 0;
}

int issue_request(char passenger_type, int initial_terminal, int destination_terminal){
	printk(KERN_ALERT "Request issued.");
	return 0;
}

int stop_shuttle(void){
	printk(KERN_ALERT "Shuttle stopping.");
	return 0;
}


static int shuttle_show(struct seq_file *m, void *v) {
	seq_printf(m, "Status:\t\nSeats:\t\nPassengers:\nLocation:\nDestination:\nDelivered:\n------\nTermianl 1:\nTerminal 2:\nTerminal 3:\nTerminal 4:\nTerminal 5:\n");
	return 0;
}


static int shuttle_open(struct inode *inode, struct  file *file) {
  	return single_open(file, shuttle_show, NULL);
}

static const struct file_operations shuttle_fops = {
  	.owner = THIS_MODULE,
  	.open = shuttle_open,
  	.read = seq_read,
  	.release = single_release,
};

static int __init shuttle_init(void) {
	STUB_start_shuttle = &start_shuttle;
	STUB_stop_shuttle = &stop_shuttle;
	STUB_request_shuttle = &issue_request;

  	proc_create("terminal", 0, NULL, &shuttle_fops);
  	return 0;
}

static void __exit shuttle_exit(void) {
	STUB_start_shuttle = NULL;
	STUB_stop_shuttle = NULL;
	STUB_request_shuttle = NULL;

  	remove_proc_entry("current_date", NULL);
}

module_init(shuttle_init);
module_exit(shuttle_exit);
