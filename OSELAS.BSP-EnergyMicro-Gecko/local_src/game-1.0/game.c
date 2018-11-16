#include <stdio.h>
#include <stdlib.h>
#include "move.c"
#include "spawn.c"
#include "flip.c"


int BOARD[24][10] = { 0 };
int ACTIVE_PIECE[4][2];
int ACTIVE_PIECE_N;
int i;
int j;
char c;
char input;
time_t t;

void print_board();

int check_game_over();

int main(int argc, char *argv[])
{
	srand((unsigned) time(&t));
	ACTIVE_PIECE_N = rand() % 7;
	spawn_piece(BOARD, ACTIVE_PIECE, ACTIVE_PIECE_N);
	print_board();
	while(1) {
		printf("Enter move, w/a/s/d: ");
		scanf("%c", &input);
		c = input;
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
