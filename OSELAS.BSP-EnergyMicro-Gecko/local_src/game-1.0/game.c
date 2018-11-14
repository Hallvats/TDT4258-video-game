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

int check_game_over();

int main(int argc, char *argv[])
{
	spawn_piece(BOARD, ACTIVE_PIECE, rand() % 7);
	print_board();
	while(1) {
		printf("Enter move, l/r/d: ");
		scanf("%c", &input);
		c = input;
		if(c == 'l' || c =='r' || c == 'd') {
			if(move(BOARD, ACTIVE_PIECE, c) == 0) {
				if(check_game_over() == 1) {
					break;
				}
				spawn_piece(BOARD, ACTIVE_PIECE, rand() % 7);
			}
		}
		print_board();
	}
	printf("GAME OVER!\n");
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
