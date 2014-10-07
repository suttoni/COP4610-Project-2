#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

//static int start_shuttle(void);

//int issue_request(char passenger_type, int initial_terminal, int destination_terminal);

//int stop_shuttle(void);

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
  	proc_create("terminal", 0, NULL, &shuttle_fops);
  	return 0;
}

static void __exit shuttle_exit(void) {
  	remove_proc_entry("current_date", NULL);
}

module_init(shuttle_init);
module_exit(shuttle_exit);
