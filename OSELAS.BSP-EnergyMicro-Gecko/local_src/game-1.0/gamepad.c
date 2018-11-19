#include <stdio.h>
#include <signal.h> //sigaction()

/* By enabling asynchronous notification, this application can receive a signal whenever data becomes available */

/* User programs have to execute two steps to enable asynchronous notification from an input file. First, they specify a process as the “owner” of the file. When a process invokes the F_SETOWN command using the fcntl system call, the process ID of the owner process is saved in filp->f_owner for later use. This step is necessary for the kernel to know just whom to notify. In order to actually enable asynchronous notifi- cation, the user programs must set the FASYNC flag in the device by means of the F_SETFL fcntl command.*/

FILE* gamepad;

int init_gamepad()
{
	/* Get input file, the gamepad driver */
	gamepad = fopen("/dev/gamepad", "r");
	if (!gamepad) {
		printf("Could not open device.")
		return EXIT_FAILURE;
	}
	
	/* fileno() returns the file descriptor number associated with a specified z/OS® XL C/C++ I/O stream, STDIN_FILENO */
	int gamepad_no = fileno(gamepad);
	int offlags, retval = 0;
	
	/* Setup asynchronous notification, p.35, chapter 6 */
	
	/* 1.Specify a process as the “owner” of the file, so that the kernel know who to notify.
	 * When a process invokes the F_SETOWN command using the fcntl system call, the process ID of the owner process is saved in filp->f_owner for later use.
	 *
	 * fcntl(STDIN_FILENO, F_SETOWN, getpid());
	 * getpid() returns the process ID of the current process
	 * 
	 * Returns negative number if unsuccessful
	 */
	
	retval = fcntl(gamepad_no, F_SETOWN, getpid());
	if (retval < 0) {
		printf("Could not specify process as file owner.");
		return EXIT_FAILURE;
	}
	
	/* 2. Enable asynchronous notification.
	 * The user programs must set the FASYNC flag in the device by means of the F_SETFL fcntl command.
	 * When F_SETFL is executed to turn on FASYNC, the driver’s fasync method is called.
	 * This method is called whenever the value of FASYNC is changed in filp->f_flags to notify the driver of the change, so it can respond properly.
	 *
	 * fcntl(STDIN_FILENO, F_SETFL, oflags | FASYNC);
	 * fcntl(STDIN_FILENO, F_GETFL);
	 * F_GETFL: Get the file access mode and the file status flags
	 *
	 * Returns negative number if unsuccessful.
	 */
	
	oflags = fcntl(gamepad_no, F_GETFL);
	retval = fcntl(gamepad_no, F_SETFL, oflags | FASYNC)
	
	if (retval < 0) {
		printf("Could not set FASYNC flag.")
		return EXIT_FAILURE;
	}
	
	/* 3. Set signal action.
	 *
	 * int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
	 *
	 * The sigaction() system call is used to change the action taken by a process on receipt of a specific signal.
	 * signum specifies the signal and can be any valid signal except SIGKILL and SIGSTOP.
	 * If act is non-null, the new action for signal signum is installed from act.
	 * If oldact is non-null, the previous action is saved in oldact.
	 *
	 * Returns 0 on success and -1 on error.
	 */
	
	retval = sigaction(SIGIO, &signal_handler, NULL);
	if (retval < 0) {
		printf("Could not register signal handler.");
		return EXIT_FAILURE;
	}
	
}

void close_gamepad()
{
	fclose(gamepad);
}

void signal_handler()
{
	/* Get input from file
	 * int fgetc(FILE *stream)
	 */
	
	int input = fgetc(gamepad); // 8 bits?
	
	/* Use input */ 
	
	
}