#include <stdio.h>
#include <stdlib.h>
#include "move.c"
#include "spawn.c"


int BOARD[24][10] = { 0 };
int ACTIVE_PIECE[4][2];
int i;
int j;
char c;
char input;

void print_board();

int main(int argc, char *argv[])
{
	spawn_piece(BOARD, ACTIVE_PIECE, 3);
	print_board();
	while(1) {
		printf("Enter move, l/r/d: ");
		scanf("%c", &input);
		c = input;
		if(c == 'l' || c =='r' || c == 'd') {
			move(BOARD, ACTIVE_PIECE, c);
		}
		print_board();
	}
	exit(EXIT_SUCCESS);
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
