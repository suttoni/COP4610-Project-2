# README #

The project was written by Ian Donnelly.

## Compiling ##

Copy the [src/shuttle_syscalls](/src/shuttle_syscalls) folder to the root of your linux kernel source folder.		
Run the command `mv ./shuttle_syscalls/kernel_makefile ./Makefile`		
Copy [syscalls.h](/src/shuttle_syscalls/syscalls.h) to `include/linux/` in the kernel source folder.		
Copy [syscall_64.tlb](/src/shuttle_syscalls_syscall_64.tlb) to `arch/x86/syscalls/` in the kernel source folder.

Once using the patched kernel, the module can be built by running the `make` command in the [src](/src) folder.

## Loading The Module ##

The `shuttle_module` module can be loaded by running the following command:		
`sudo insmod shuttle_module.ko`

## Using the System Calls ##

The following system calls are implimented by this project:		
- `start_shuttle()` as syscall # 321
- `stop_shuttle()` as syscall # 322
- `request_shuttle(char passenger_type, int initial_terminal, int destination_terminal)` as syscall #323


