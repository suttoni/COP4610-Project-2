#include "data.h"

//Extern data
extern struct mutex shuttle_lock;
extern struct mutex terminal_lock;
extern struct shuttle_data shuttle;
extern struct terminal_data terminal[NUM_TERMINALS];
extern int adults_delivered, children_delivered, luggage_delivered;
//Stubs
extern long (* STUB_start_shuttle)(void);
extern long (* STUB_stop_shuttle)(void);
extern long (* STUB_request_shuttle)(char passenger_type, int initial_terminal, int destination_terminal);

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
