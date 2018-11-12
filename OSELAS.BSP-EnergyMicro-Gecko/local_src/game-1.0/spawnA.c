#include <stdio.h>
#include <stdlib.h>


int i_piece[4][4] = {{0,0,1,0},
                     {0,0,1,0},
                     {0,0,1,0},
                     {0,0,1,0}};

int j_piece[4][4] = {{0,0,1,0},
                     {0,0,1,0},
                     {0,1,1,0},
                     {0,0,0,0}};

int l_piece[4][4] = {{0,1,0,0},
                     {0,1,0,0},
                     {0,1,1,0},
                     {0,0,0,0}};

int o_piece[4][4] = {{0,1,1,0},
                     {0,1,1,0},
                     {0,0,0,0},
                     {0,0,0,0}};

int s_piece[4][4] = {{0,0,1,1},
                     {0,1,1,0},
                     {0,0,0,0},
                     {0,0,0,0}};

int z_piece[4][4] = {{0,1,1,0},
                     {0,0,1,1},
                     {0,0,0,0},
                     {0,0,0,0}};

int t_piece[4][4] = {{0,0,1,0},
                     {0,1,1,1},
                     {0,0,0,0},
                     {0,0,0,0}};

typedef int (*shape_array_t)[4];

shape_array_t tetrominos[] = { i_piece, j_piece, l_piece, o_piece, s_piece, z_piece, t_piece };

void spawn_piece(int by, int bx, int board[by][bx], int p) {
  int i;
	for(i = 0; i < 4; i++) {
      int j;
	    for(j = 0; j < 4; j++)
		    board[i][3+j] = tetrominos[p][i][j];
	}
}
