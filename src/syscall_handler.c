#include "data.h"

//Extern Data
struct mutex shuttle_lock;
struct mutex terminal_lock;
struct task_struct *shuttle_thread;
struct shuttle_data shuttle;
struct terminal_data terminal[NUM_TERMINALS];

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
		shuttle_thread = kthread_run(shuttle_service, NULL, "shuttle");
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
		if(passenger->type == 'C'){
			mutex_lock(&terminal_lock);
			list_add(&passenger->passenger_list, &terminal[passenger->initial_terminal-1].queue);
			terminal[passenger->initial_terminal-1].queue_size++;
			mutex_unlock(&terminal_lock);
		}
		else{
			mutex_lock(&terminal_lock);
			list_add_tail(&passenger->passenger_list, &terminal[passenger->initial_terminal-1].queue);
			terminal[passenger->initial_terminal-1].queue_size++;
			mutex_unlock(&terminal_lock);
		}
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
	if(shuttle.keep_running == true){
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
