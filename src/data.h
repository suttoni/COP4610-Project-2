#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

//Define Stuff
#define TOTAL_SEATS 50
#define NUM_TERMINALS 5

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
	int seats_used;
	bool keep_running;
	struct list_head passengers;
};

struct terminal_data{
	int queue_size;
	int delivered;
	struct list_head queue; 
};


//Function Declarations
int shuttle_show(struct seq_file *m, void *v);
int shuttle_open(struct inode *inode, struct  file *file);
int __init shuttle_init(void);
void __exit shuttle_exit(void);

long start_shuttle(void);
long issue_request(char passenger_type, int initial_terminal, int destination_terminal);
long stop_shuttle(void);

short unload_passengers(void);
short load_passengers(void);
int shuttle_service(void * data);
