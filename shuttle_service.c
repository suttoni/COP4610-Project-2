#include "data.h"

extern struct mutex shuttle_lock;
extern struct mutex terminal_lock;
extern struct task_struct *shuttle_thread;
extern struct shuttle_data shuttle;
extern struct terminal_data terminal[NUM_TERMINALS];
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
