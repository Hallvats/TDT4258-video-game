#include <linux/fb.h>


int init_screen(){

	//TODO Side 55 i "Compendium"

	//TODO: Usikker på om 0 er riktig startverdi
	rect.dx = 0;
	rect.dy = 0;
	rect.width = 0;
	rect.height = 0;

	frameBuffer = open("/dev/fb0", 0_RDWR);
	if(!frameBuffer){
		printf("FrameBuffer was not opened correctly");
		return EXIT_FAILURE;
	}

	screen = mmap(0, WIDTH * HEIGHT * 2, PROT_READ | PROT_WRITE, MAP_SHARED, frameBuffer, 0);
	if (screen == MAP_FAILED){
		return EXIT_FAILURE;
	}

	return 0;
}

int clean_screen(){
	munmap(fbp, screensize_bytes);
	close(frameBuffer);
}
