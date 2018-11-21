#include <linux/fb.h>


int init_screen(){

	//TODO Side 55 i "Compendium"

	//TODO: Usikker på om 0 er riktig startverdi
	rect.dx = 0;
	rect.dy = 0;
	rect.width = 0;
	rect.height = 0;

	frameBuffer = open("/dev/fb0", 0_RDWR); //fbfd
	if(!frameBuffer){
		printf("FrameBuffer was not opened correctly");
		return EXIT_FAILURE;
	}

	screenBuffer = (unsigned short*) mmap(0, WIDTH * HEIGHT * 2, PROT_READ | PROT_WRITE, MAP_SHARED, frameBuffer, 0);
	if (screen == MAP_FAILED){
		return EXIT_FAILURE;
	}

	return 0;
}

void update_screen(int board[24][10]){
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
			 if(board[i][j] == 1) {
				 for(y = i * 10 + 20; y < i * 10 + 20 + 10; y++) {
					 for(x = j * 10 + 110; x < j * 10 + 110 + 10; y++) {
						 screenBuffer[y * 320 + x] = 0xF;
					 }
				 }
			 }
		 }
	 }
	 rect.dx = 240;
	 rect.dy = 320;

	 ioctl(screenBuffer, 0x4680, &rect);
}

int clean_screen(){
	munmap(screenBuffer, sizeof(screenBuffer)); //fbp -> screen
	close(frameBuffer);
}
