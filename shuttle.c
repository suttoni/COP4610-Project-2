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
	int seats_used;
	bool keep_running;
	struct list_head passengers;
};

struct terminal_data{
	int queue_size;
	int delivered;
	struct list_head queue; 
};

//Globals
struct mutex shuttle_lock;
struct mutex terminal_lock;
struct task_struct *shuttle_thread;
struct shuttle_data shuttle;
struct terminal_data terminal[NUM_TERMINALS];
int adults_delivered, children_delivered, luggage_delivered;


//Shuttle service functions
short unload_passengers(void){
	short unloaded = 0;
	struct list_head *pos, *q;
	struct passenger_data *temp;
	list_for_each_safe(pos, q, &shuttle.passengers){
			temp = list_entry(pos, struct passenger_data, passenger_list);
			if(temp->destination == shuttle.current_terminal){
				switch (temp->type){
					case 'A':
						shuttle.seats_used -= 2;
						adults_delivered++;
						terminal[shuttle.current_terminal-1].delivered++;
						unloaded++;
						break;
					case 'C':
						shuttle.seats_used -= 1;
						children_delivered++;
						terminal[shuttle.current_terminal-1].delivered++;
						unloaded++;
						break;
					case 'L':
						shuttle.seats_used -= 4;
						luggage_delivered++;
						terminal[shuttle.current_terminal-1].delivered++;
						unloaded++;
						break;
				}
				list_del(pos);
				kfree(temp);
			}
	}

	return unloaded;
}

short load_passengers(void){
	short loaded = 0;
	struct list_head *pos, *q;
	struct passenger_data *temp;
	list_for_each_safe(pos, q, &terminal[shuttle.current_terminal-1].queue){
			temp = list_entry(pos, struct passenger_data, passenger_list);
			if(shuttle.seats_used < TOTAL_SEATS){
				switch (temp->type){
					case 'A':
						if((shuttle.seats_used+2) <= TOTAL_SEATS){
							shuttle.seats_used += 2;
							list_del(pos);
							list_add(pos, &shuttle.passengers);
							loaded++;
						}
						break;
					case 'C':
						if((shuttle.seats_used+1) <= TOTAL_SEATS){
							shuttle.seats_used += 1;
							list_del(pos);
							list_add(pos, &shuttle.passengers);
							loaded++;
						}
						break;
					case 'L':
						if((shuttle.seats_used+4) <= TOTAL_SEATS){
							shuttle.seats_used += 4;
							list_del(pos);
							list_add(pos, &shuttle.passengers);
							loaded++;
						}
						break;
				}
			}
	}

	return loaded;
}

int shuttle_service(void * data){
	int travel_time = 0;
	short passenger_changes;
	while(shuttle.keep_running){
		//Do shuttle things
		if(shuttle.status == PARKED){
			//Unload and then load
			passenger_changes = 0;
			mutex_lock(&shuttle_lock);
			passenger_changes += unload_passengers();
			mutex_lock(&terminal_lock);
			passenger_changes += load_passengers();
			shuttle.destination_terminal = (shuttle.current_terminal%NUM_TERMINALS)+1;
			mutex_unlock(&shuttle_lock);
			mutex_unlock(&terminal_lock);
			if(passenger_changes <= 4)
				passenger_changes = 0;
			msleep(100 + passenger_changes*30);
		}
		mutex_lock(&shuttle_lock);
		shuttle.status = MOVING;
		travel_time = abs(shuttle.current_terminal - shuttle.destination_terminal)*300;
		mutex_unlock(&shuttle_lock);
		msleep(travel_time);
		mutex_lock(&shuttle_lock);
		shuttle.current_terminal = shuttle.destination_terminal;
		shuttle.destination_terminal = 0;
		shuttle.status = PARKED;
		mutex_unlock(&shuttle_lock);
	}

	shuttle.status = DEACTIVATING;	

	while(shuttle.seats_used != 0){
		//Unload passengers
		passenger_changes = 0;
		mutex_lock(&shuttle_lock);
		unload_passengers();
		mutex_unlock(&shuttle_lock);
		if(passenger_changes <= 4)
			passenger_changes = 0;
		msleep(100 + passenger_changes*30);
		mutex_lock(&shuttle_lock);
		shuttle.destination_terminal = (shuttle.current_terminal%NUM_TERMINALS)+1;
		travel_time = abs(shuttle.current_terminal - shuttle.destination_terminal)*300;
		shuttle.current_terminal = shuttle.destination_terminal;
		shuttle.destination_terminal = 0;
		mutex_unlock(&shuttle_lock);
		msleep(travel_time);
	}
	
	shuttle.status = OFFLINE;
	kthread_stop(shuttle_thread);
	
	return 0;
}

//Syscall functions
long start_shuttle(void){
	mutex_lock(&shuttle_lock);
	if(shuttle.status == OFFLINE){
		printk(KERN_ALERT "Shuttle starting.");
		shuttle.status = PARKED;
		shuttle.current_terminal = 3;
		shuttle.seats_used = 0;
		shuttle.keep_running = true;
		mutex_unlock(&shuttle_lock);
		shuttle_thread = kthread_run(shuttle_service, NULL, "Shuttle thread");
		printk(KERN_ALERT "Shuttle thread started.");
		return 0;
	}
	else{
		mutex_unlock(&shuttle_lock);
		printk(KERN_ALERT "Shuttle already started.");
	}
	return 1;
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
		mutex_lock(&terminal_lock);
		list_add_tail(&passenger->passenger_list, &terminal[passenger->initial_terminal-1].queue);
		terminal[passenger->initial_terminal-1].queue_size++;
		mutex_unlock(&terminal_lock);
		printk(KERN_ALERT "Request issued.");
		return 0;
	}
	else{
		printk(KERN_ALERT "Invalid passenger information. %c %d %d", passenger_type, initial_terminal, destination_terminal);
	}
	return 1;
}

long stop_shuttle(void){
	mutex_lock(&shuttle_lock);
	if(shuttle.keep_runing == true){
		shuttle.keep_running = false;
		mutex_unlock(&shuttle_lock);
		printk(KERN_ALERT "Shuttle stopping.");
		return 0;
	}
	else{
		printk(KERN_ALERT "Shuttle is deactivating or parked already.");
	}
	mutex_unlock(&shuttle_lock);
	return 1;
}

//Module functions
int shuttle_show(struct seq_file *m, void *v) {
	struct list_head *pos;
	struct passenger_data *temp;
	char *status_string;
	int adults;
	int children;
	int luggage;
	mutex_lock(&shuttle_lock);
	mutex_lock(&terminal_lock);
	switch (shuttle.status){
		case OFFLINE:
			status_string = "OFFLINE";
			break;
		case DEACTIVATING:
			status_string = "DEACTIVATING";
			break;
		case PARKED:
			status_string = "PARKED";
			break;
		case MOVING:
			status_string = "MOVING";
			break;
	}
	seq_printf(m, "Status:\t\t%s\n", status_string);
	
	if(shuttle.seats_used%2 == 1)
		seq_printf(m, "Seats:\t\t%d.5 used %d.5 available\n", shuttle.seats_used/2, (TOTAL_SEATS-shuttle.seats_used)/2);
	else
		seq_printf(m, "Seats:\t\t%d used %d available\n", shuttle.seats_used/2, (TOTAL_SEATS-shuttle.seats_used)/2);
	adults = 0;
	children = 0;
	luggage = 0;
	list_for_each(pos, &shuttle.passengers){
		temp = list_entry(pos, struct passenger_data, passenger_list);
		switch (temp->type){
			case 'A':
				adults++;
				break;
			case 'C':
				children++;
				break;
			case 'L':
				luggage++;
				break;
		}
	}
	
	seq_printf(m, "Passengers:\t%d  (%d adults with luggage, %d adults without luggage, %d children)\n", (adults+children+luggage), luggage, adults, children);
	seq_printf(m, "Location:\t%hd\n", shuttle.current_terminal);
	seq_printf(m, "Destination:\t%hd\n", shuttle.destination_terminal);
	seq_printf(m, "Delivered:\t%d  (%d adults with luggage, %d adults without luggage, %d children)\n", (adults_delivered+children_delivered +luggage_delivered), luggage_delivered, adults_delivered, children_delivered);
	seq_printf(m, "-----\n");
	for(int i = 0; i < NUM_TERMINALS; i++){
		adults = 0;
		children = 0;
		luggage = 0;
		seq_printf(m, "Terminal %d:", i+1);
		list_for_each(pos, &terminal[i].queue){
			temp = list_entry(pos, struct passenger_data, passenger_list);
			switch (temp->type){
				case 'A':
					adults++;
					break;
				case 'C':
					children++;
					break;
				case 'L':
					luggage++;
					break;
			}
		}
		seq_printf(m, "\t%d adult with luggage, %d adult without luggage, %d children in queue. %d passengers delivered so far.\n", luggage, adults, children, terminal[i].delivered);
	}
	mutex_unlock(&shuttle_lock);
	mutex_unlock(&terminal_lock);
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

	adults_delivered = 0;
	children_delivered = 0;
	luggage_delivered = 0;

	mutex_init(&shuttle_lock);
	mutex_init(&terminal_lock);

	mutex_lock(&shuttle_lock);
	shuttle.status = OFFLINE;
	shuttle.current_terminal = 0;
	shuttle.seats_used = 0;
	INIT_LIST_HEAD(&shuttle.passengers);
	mutex_unlock(&shuttle_lock);

	mutex_lock(&terminal_lock);
	for(int i = 0; i < NUM_TERMINALS; i++){
		terminal[i].queue_size = 0;
		terminal[i].delivered = 0;
		INIT_LIST_HEAD(&terminal[i].queue);
	}
	mutex_unlock(&terminal_lock);

  	proc_create("terminal", 0, NULL, &shuttle_fops);
  	return 0;
}

void __exit shuttle_exit(void) {
	STUB_start_shuttle = NULL;
	STUB_stop_shuttle = NULL;
	STUB_request_shuttle = NULL;
	
	mutex_lock(&shuttle_lock);
	shuttle.keep_running = false;
	mutex_unlock(&shuttle_lock);
	
	while(1){
	//Wait for shuttle to stop before removing module
		mutex_lock(&shuttle_lock);
		if(shuttle.status == OFFLINE){
			mutex_unlock(&shuttle_lock);	
			break;
		}
		mutex_unlock(&shuttle_lock);
		msleep(1000);
	}

	mutex_destroy(&shuttle_lock);
	mutex_destroy(&terminal_lock);

	remove_proc_entry("terminal", NULL);
}

module_init(shuttle_init);
module_exit(shuttle_exit);
