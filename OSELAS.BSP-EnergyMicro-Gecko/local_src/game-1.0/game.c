#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h> //sigaction()
#include <string.h>
#include <fcntl.h>

#include <time.h>
#include <sys/time.h>

#include "move.c"
#include "spawn.c"
#include "flip.c"

/*
 * Bitmap for gamepad input
 */
#define SW1 (1<<0)
#define SW2 (1<<1)
#define SW3 (1<<2)
#define SW4 (1<<3)
#define SW5 (1<<4)
#define SW6 (1<<5)
#define SW7 (1<<6)
#define SW8 (1<<7)

#define WIDTH 320
#define HEIGHT 240

FILE* gamepad;
int frame;

int BOARD[24][10] = { 0 };
int ACTIVE_PIECE[4][2];
int ACTIVE_PIECE_N;
int i;
int j;
char c;
char input;
time_t t;
struct fb_copyarea area;

/* Prototypes */
void print_board();
int check_game_over();
int init_gamepad();
void close_gamepad();
int init_screen();
void update_screen();
void clean_screen();
void signal_handler();
int open(const char *path, int oflag, ... );
void update_pixels(int x, int y, int val);
int ns;
char inputTracker = '_';
unsigned short* screenBuffer;

int main(int argc, char *argv[])
{	init_gamepad();
	init_screen();
	printf("initeded");
	srand((unsigned) time(&t));
	printf("sranded");
	ACTIVE_PIECE_N = rand() % 7;
	printf("actived_pice\n");
	spawn_piece(BOARD, ACTIVE_PIECE, ACTIVE_PIECE_N);
	printf("spawned oices\n");
	print_board();
	printf("board_printeded\n");
	struct timespec tim, tim2, tim3;
	while(1) {
        tim.tv_sec = 1;
        tim.tv_nsec = 500000000L;;
        tim2.tv_sec = 1;
        tim2.tv_nsec = 1;
        tim3.tv_sec = 0;
        tim3.tv_nsec = 1;
        ns = nanosleep(&tim, &tim2);
		c = inputTracker;
		if(c == 'a' || c =='s' || c == 'd') {
			if(move(BOARD, ACTIVE_PIECE, c) == 0) {
				if(check_game_over() == 1) {
					break;
				}
				ACTIVE_PIECE_N = rand() % 7;
				spawn_piece(BOARD, ACTIVE_PIECE, ACTIVE_PIECE_N);
				reset_state();
			}
		} else if(c == 'w') {
			flip(ACTIVE_PIECE, BOARD, ACTIVE_PIECE_N);
		}
		update_screen();
		c = '_'; //Clear input
		inputTracker = 's';
	}
	printf("GAME OVER!\n");
	close_gamepad();
	clean_screen();
	exit(EXIT_SUCCESS);
}

int check_game_over() {
	for(i = 0; i < 4; i++) {
		if(ACTIVE_PIECE[i][0] < 4) {
			return 1;
		}
	}
	return 0;
}

void print_board() {
	printf("\n");
	for(i = 0; i < 4; i++) {
		BOARD[ACTIVE_PIECE[i][0]][ACTIVE_PIECE[i][1]] = 1;
	}
	for(i = 0; i < 24; i++){
		for(j = 0; j < 10; j++) {
			printf("%d ", BOARD[i][j]);
		}
		printf("\n");
	}
}

/* By enabling asynchronous notification, this application can receive a signal whenever data becomes available */

/* User programs have to execute two steps to enable asynchronous notification from an input file. First, they specify a process as the “owner” of the file. When a process invokes the F_SETOWN command using the fcntl system call, the process ID of the owner process is saved in filp->f_owner for later use. This step is necessary for the kernel to know just whom to notify. In order to actually enable asynchronous notifi- cation, the user programs must set the FASYNC flag in the device by means of the F_SETFL fcntl command.*/

int init_gamepad()
{
	int oflags, retval, gamepad_no;
	struct sigaction sa;
	
	/* Get input file, the gamepad driver */
	gamepad = open("/dev/gamepad", O_RDWR);
	if (!gamepad) {
		printf("Could not open device.");
		return EXIT_FAILURE;
	}
	
	/* fileno() returns the file descriptor number associated with a specified z/OS® XL C/C++ I/O stream, STDIN_FILENO */
	gamepad_no = gamepad;
	
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
	retval = fcntl(gamepad_no, F_SETFL, oflags | FASYNC);
	
	if (retval < 0) {
		printf("Could not set FASYNC flag.");
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
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler; // Which function should run on signal?
	sa.sa_flags = 0;
	retval = sigaction(SIGIO, &sa, NULL); // Interrupt on reciving SIGIO signal
	if (retval < 0) {
		printf("Could not register signal handler.");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}

void close_gamepad()
{
	fclose(gamepad);
}

void signal_handler()
{
	/* Get input from file */
	char buf[100];
	int input = read(gamepad, buf , sizeof buf);
	if(buf[0] == 254){
		inputTracker = 'a';
	} else if (buf[0] == 253){
		inputTracker = 'w';
	} else if (buf[0] == 251){
		inputTracker = 'd';
	} else if (buf[0] == 247){
		inputTracker = 's';
	}
	
}

int init_screen(){
	frame = open("/dev/fb0", 2);
	screenBuffer = (unsigned short*) mmap(NULL, 2*320*240, PROT_WRITE | PROT_READ, MAP_SHARED, frame, 0);
	
	area.width = 1;
	area.height = 1;
	int k;
	int h;
	for(h = 0; h < 24; h++) {
		for(k = 0; k < 32; k++) {
			update_pixels(k-11, h, 0x0990);
			}
	}
	return 0;
}

void update_screen(){	
	int y;
	int x;
	for(y = 0; y < 24; y++) {
		for(x = 0; x < 10; x++) {
			if(BOARD[y][x] == 1) {
				update_pixels(x, y, 0x04ff);
			}
			else if(BOARD[i][j] == 0){
				update_pixels(x, y, 0x0);
			}
		}
	}
	 
}

void update_pixels(int x, int y, int val){
	int i;
	int j;
	
	area.width = 10;
	area.height = 10;
	
	int ystart = y*320*10;
	int xstart = x*10 + 110;
	int start = ystart + xstart;
	for(i=0; i<10; i++) { //Height
		for(j=0; j<10; j++){ //Width
			screenBuffer[start + i*320  + j] = val;
		}
	}
	
	area.dx = x*10+110;
	area.dy = y*10;
	
	ioctl(frame, 0x4680, &area);
}

void clean_screen(){
	/*munmap(screenBuffer, sizeof(screenBuffer)); //fbp -> screen
	close(frame);*/
}