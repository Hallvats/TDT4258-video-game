#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int i_piece[4][2] = {{0,2},{1,2},{2,2},{3,2}};
int j_piece[4][2] = {{0,2},{1,2},{2,2},{2,1}};
int l_piece[4][2] = {{0,1},{1,1},{2,1},{2,2}};
int o_piece[4][2] = {{0,1},{0,2},{1,1},{1,2}};
int s_piece[4][2] = {{0,2},{0,3},{1,1},{1,2}};
int z_piece[4][2] = {{0,1},{0,2},{1,2},{1,3}};
int t_piece[4][2] = {{0,2},{1,1},{1,2},{1,3}};

typedef int (*shape_array_t)[2];

shape_array_t tetrominos[] = { i_piece, j_piece, l_piece, o_piece, s_piece, z_piece, t_piece };

void spawn_piece(int by, int bx, int board[by][bx], int active_piece[4][2], int p) {
  int i;
	for(i = 0; i < 4; i++) {
	    board[tetrominos[p][i][0]][3+tetrominos[p][i][1]] = 1;
	}
  memcpy(active_piece, tetrominos[p], sizeof (int) * by * bx);
}
