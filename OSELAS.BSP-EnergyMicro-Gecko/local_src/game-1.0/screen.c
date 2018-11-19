int init_screen(){

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
