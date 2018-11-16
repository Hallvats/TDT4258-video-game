#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int i_piece[4][2] = {{0,2},{1,2},{2,2},{3,2}};
int j_piece[4][2] = {{1,2},{2,2},{3,2},{3,1}};
int l_piece[4][2] = {{1,1},{2,1},{3,1},{3,2}};
int o_piece[4][2] = {{2,1},{2,2},{3,1},{3,2}};
int s_piece[4][2] = {{2,3},{2,2},{3,2},{3,1}};
int z_piece[4][2] = {{2,1},{2,2},{3,2},{3,3}};
int t_piece[4][2] = {{2,2},{3,1},{3,2},{3,3}};

typedef int (*shape_array_pieces)[2];

shape_array_pieces tetrominos[] = { i_piece, j_piece, l_piece, o_piece, s_piece, z_piece, t_piece };

void spawn_piece(int board[24][10], int active_piece[4][2], int p) {
  int i;
	for(i = 0; i < 4; i++) {
	    board[tetrominos[p][i][0]][3+tetrominos[p][i][1]] = 1;
	}
  memcpy(active_piece, tetrominos[p], sizeof (int) * 4 * 2);
  for(i = 0; i < 4; i++) {
    active_piece[i][1] += 3;
  }
}
