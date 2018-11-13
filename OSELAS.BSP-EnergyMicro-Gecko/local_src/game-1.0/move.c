#include <stdio.h>
#include <stdlib.h>


int horizontal = 0;
int vertical = 0;
int i;
int j;
int is_active_piece;

int move_legal(int board[24][10], int active_piece[4][2], char direction) {
	horizontal = 0;
	vertical = 0;
	if(direction == 'l') {
		horizontal = -1;
	} else if(direction == 'r') {
		horizontal = 1;
	} else if(direction == 'd') {
		vertical = 1;
	}
	for(i = 0; i < 4; i++) {
		if((active_piece[i][0] + vertical < 0) || (active_piece[i][0] + vertical > 23)) {
			return 0;
		}
		if((active_piece[i][1] + horizontal < 0) || (active_piece[i][1] + horizontal > 9)) {
			return 0;
		}
		if(board[active_piece[i][0] + vertical][active_piece[i][1] + horizontal] == 1) {
			is_active_piece = 0;
			for(j = 0; j < 4; j++) {
				if((active_piece[i][0] + vertical == active_piece[j][0]) && (active_piece[i][1] + horizontal == active_piece[j][1])) {
					is_active_piece = 1;
				}
			}
			if(!is_active_piece) {
				return 0;
			}
		}
	}
	return 1;
}

int move(int board[24][10], int active_piece[4][2], char direction) {
	horizontal = 0;
	vertical = 0;
	if(direction == 'l') {
		horizontal = -1;
	} else if(direction == 'r') {
		horizontal = 1;
	} else if(direction == 'd') {
		vertical = 1;
	}
	if(move_legal(board, active_piece, direction)) {
		for(i = 0; i < 4; i++) {
			board[active_piece[i][0]][active_piece[i][1]] = 0;
		}
		for( i = 0; i < 4; i++) {
			board[active_piece[i][0] + vertical][active_piece[i][1] + horizontal] = 1;
			active_piece[i][0] = active_piece[i][0] + vertical;
			active_piece[i][1] = active_piece[i][1] + horizontal;
		}
		return 1;
	} else {
		if(direction == 'd') {
			return 0;
		}
	}
	return 1;
}
