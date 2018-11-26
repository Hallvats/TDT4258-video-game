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
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#include <time.h>
#include <sys/time.h>

#include "move.c"
#include "spawn.c"
#include "flip.c"

/* Bitmap for gamepad input */
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
struct fb_copyarea rect;

/* Prototypes */
void print_board();
int check_game_over();
int init_gamepad();
void close_gamepad();
void init_screen();
void update_screen();
void clean_screen();
void signal_handler();
int open(const char *path, int oflag, ... );

char inputTracker = '_';
uint16_t* screenBuffer;

int main(int argc, char *argv[])
{	init_gamepad();
	init_screen();
	srand((unsigned) time(&t));
	ACTIVE_PIECE_N = rand() % 7;
	spawn_piece(BOARD, ACTIVE_PIECE, ACTIVE_PIECE_N);
	print_board();
	while(1) {
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
		c = '_'; //Clear input TODO: Sjekk om dette er riktig pointer-bruk
		inputTracker = '_';
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
	gamepad = fopen("/dev/gamepad", O_RDONLY);
	if (!gamepad) {
		printf("Could not open device.");
		return EXIT_FAILURE;
	}
	
	/* fileno() returns the file descriptor number associated with a specified z/OS® XL C/C++ I/O stream, STDIN_FILENO */
	gamepad_no = fileno(gamepad);
	
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
	/* Get input from file
	 * int fgetc(FILE *stream)
	 */
	
	int input = ~(fgetc(gamepad));
	
	if(input & SW1){
		inputTracker = 'a';
	} else if (input & SW2){
		inputTracker = 'w';
	} else if (input & SW3){
		inputTracker = 'd';
	} else if (input & SW4){
		inputTracker = 's';
	}
	
}

void init_screen(){

	rect.dx = 0;
	rect.dy = 0;
	rect.width = 0;
	rect.height = 0;

	frame = open("/dev/fb0", O_RDWR); //fbfd
	if(!frame){
		printf("Frame was not opened correctly");
		return EXIT_FAILURE;
	}

	screenBuffer = (uint16_t *) mmap(0, WIDTH * HEIGHT * 2, PROT_READ | PROT_WRITE, MAP_SHARED, frame, 0);
	if (screenBuffer == MAP_FAILED){
		return EXIT_FAILURE;
	}
}

void update_screen(){
	/*
	 * 320 pixler per linje
	 * x0 = 110
	 * y0 = 20
	 *
	 * Move:
	 * Finne kordinat på brikke
	 * update 7x7 matrise
	 *
	 * clear_lines:
	 * update hele brettet
	 * x = [110, 210], y = [20, 220]
	 *
	 * */
	 int i;
	 int j;
	 int y;
	 int x;

	 for(i = 0; i < 320*240; i++) {
		 screenBuffer[i] = 0x0;
	 }
	 for(i = 0; i < 24; i++) {
		 for(j = 0; j < 10; j++) {
			 if(BOARD[i][j] == 1) {
				 for(y = i * 10 + 20; y < i * 10 + 20 + 10; y++) {
					 for(x = j * 10 + 50; x < j * 10 + 50 + 10; y++) {
						 screenBuffer[y * 320 + x] = 0xF;
					 }
				 }
			 }
		 }
	 }
	 rect.dx = 240;
	 rect.dy = 320;

	 ioctl(frame, 0x4680, &rect);
}

void clean_screen(){
	munmap(screenBuffer, sizeof(screenBuffer)); //fbp -> screen
	close(frame);
}