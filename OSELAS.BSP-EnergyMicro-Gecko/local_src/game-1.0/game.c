#include <stdio.h>
#include <stdlib.h>
#include "spawnB.c"


int BOARD[24][10] = { 0 };
int ACTIVE_PIECE[4][2] = { 0 };

int main(int argc, char *argv[])
{
	printf("Start.\n");
	int by = 24;
	int bx = 10;
	printf("Board initialized\n");
	int i;
	int j;
	for(i = 0; i < by; i++) {
		for(j = 0; j < bx; j++) {
			printf("%d ", BOARD[i][j]);
		}
		printf("\n");
	}
	spawn_piece(by, bx, BOARD, ACTIVE_PIECE, 0);
	printf("Piece spawned\n");
	for(i = 0; i < by; i++) {
		for(j = 0; j < bx; j++) {
			printf("%d ", BOARD[i][j]);
		}
		printf("\n");
	}
	printf("Active piece coordinates:\n");
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 2; j++) {
			printf("%d ", ACTIVE_PIECE[i][j]);
		}
		printf("\n");
	}

	exit(EXIT_SUCCESS);
}
