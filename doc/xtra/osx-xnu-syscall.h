http://uninformed.org/index.cgi?v=4&a=3&p=12

///

int $0x81 is used on intel

////

Next: MIG Up: Mach Traps (system calls) Previous: Mach Traps (system calls)   Contents

List of mach traps in xnu-792.6.22

/* 26 */	mach_reply_port
/* 27 */	thread_self_trap
/* 28 */	task_self_trap
/* 29 */	host_self_trap
/* 31 */	mach_msg_trap
/* 32 */	mach_msg_overwrite_trap
/* 33 */	semaphore_signal_trap
/* 34 */	semaphore_signal_all_trap
/* 35 */	semaphore_signal_thread_trap
/* 36 */	semaphore_wait_trap
/* 37 */	semaphore_wait_signal_trap
/* 38 */	semaphore_timedwait_trap
/* 39 */	semaphore_timedwait_signal_trap
/* 41 */	init_process
/* 43 */	map_fd
/* 45 */ 	task_for_pid
/* 46 */	pid_for_task
/* 48 */	macx_swapon
/* 49 */	macx_swapoff
/* 51 */	macx_triggers
/* 52 */	macx_backing_store_suspend
/* 53 */	macx_backing_store_recovery
/* 59 */ 	swtch_pri
/* 60 */	swtch
/* 61 */	thread_switch
/* 62 */	clock_sleep_trap
/* 89 */	mach_timebase_info_trap
/* 90 */	mach_wait_until_trap
/* 91 */	mk_timer_create_trap
/* 92 */	mk_timer_destroy_trap
/* 93 */	mk_timer_arm_trap
/* 94 */	mk_timer_cancel_trap
/* 95 */	mk_timebase_info_trap
/* 100 */	iokit_user_client_trap

When executing one of these traps the number on the left hand side (multiplied by -1) must be placed into the eax register. (intel) Each of the arguments must be pushed to the stack in reverse order. Although I could go into a low level description of how to send a mach msg here, the paper [11] in the references has already done this and the author is a lot better at it than me. I strongly suggest reading this paper if you are at all interested in the subject matter. 
