# Project 2 Part 3 Report #

## Design ##

### General Overview ###

The design follows the specifications outlined in the project
description document. Namely, there are three system calls
which are added to the Linux Kernel implemented in the following
functions:	

- `start_shuttle` will start the shuttle if it is current offline and the module is loaded properly.
- `issue_request` will add a passenger to the queue only if it is a valid passenger.
- `stop_shuttle` will deactive the shuttle so that it stops picking up passengers, and then go offline once the shuttle has dropped off current passengers.

These three functions are loaded via a kernel module called `shuttle_module`
and allow the syscalls to work. These fucntions are called by a wrapper
which gets built into the kernel so that if a user trys calling the
syscalls without the module, the wrappers will alert them to this
and no unexpected behavior will happen. 

### Data Strctures ###

The data structures used in this project are all declared in [data.h](/src/data.h).		
An excerpt from that file with the main data structures is:		

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

As you can see, there are data structures used for passengers, the shuttle, and terminals. 
This project also relies on `linux/list.h` in order to impliment a linked list to keep
track of passengers. The `passengers` element in `shuttle_data` contains a list of passengers 
currently on the shuttle. The `queue` element in `terminal_data` contains a list of passengers
in line at the given terminal.

Additionally, there are two mutexes, `shuttle_lock` and `terminal_lock` which are used in critical
sections to protect the integrety of data in those objects.


### Algorithm ###

The `start_shuttle` function starts a new kthread, `shuttle` which operates the shuttle, visting terminals, loading and unloading passengers.
The `issue_request` function adds each valid passenger to the queue of their initial terminal, it adds children to the front of the queue and adds adults and adults with luggage to the back of the queue (this way the shuttle picks up children first). 
The `stop_shuttle` function sets the `keep_running` boolean value of the shuttle to `false`. At this point the shuttle stops
picking up new passengers, instead it only unloads passengers at their destination terminal. Once the shuttle has dropped off all of its current
passengers, it will go to an `OFFLINE` state.

The shuttle itself uses an algorithm similar to `C-SCAN` in order to vist all the terminals. It visits the terminals in order, 1, 2, 3, 4, 5 and then loops again visting terminal 1 after terminal 5. This way, there is no starvation since the shuttle is sure to visit all terminals regularly. Additionally, since children are loaded first into each terminal's queue, they are the first to be picked up by the schedule which imporves the average passengers per minute when the shuttle runs.

## Errors ##

There are no known errors with my implementation of this project. 

## Contributions ##

Please see my [GitHub Page](https://github.com/easyxtarget/COP4610-Project-2/graphs/contributors) for this project.

