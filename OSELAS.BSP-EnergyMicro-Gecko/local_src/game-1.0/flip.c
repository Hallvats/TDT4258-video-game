#include <stdio.h>
#include <stdlib.h>


int state = 3;
int i;
int is_active_piece_flip;

int i_flip[4][4][2] = {{{ 3, 1},{ 2, 0},{ 1,-1},{ 0,-2}},
                       {{-3,-1},{-2, 0},{-1, 1},{ 0, 2}},
                       {{ 3, 1},{ 2, 0},{ 1,-1},{ 0,-2}},
                       {{-3,-1},{-2, 0},{-1, 1},{ 0, 2}}};
int j_flip[4][4][2] = {{{ 2, 1},{ 1, 0},{ 0,-1},{-1, 0}},
                       {{ 0,-2},{-1,-1},{-2, 0},{-1, 1}},
                       {{-2, 0},{-1, 1},{ 0, 2},{ 1, 1}},
                       {{ 0, 1},{ 1, 0},{ 2,-1},{ 1,-2}}};
int l_flip[4][4][2] = {{{ 1, 2},{ 0, 1},{-1, 0},{ 0,-1}},
                       {{ 1,-1},{ 0, 0},{-1, 1},{-2, 0}},
                       {{ 0,-1},{ 1, 0},{ 2, 1},{ 1, 2}},
                       {{-2, 0},{-1,-1},{ 0,-2},{ 1,-1}}};
int o_flip[4][4][2] = {{{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}},
                       {{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}},
                       {{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}},
                       {{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}}};
int s_flip[4][4][2] = {{{ 1,-1},{ 0, 0},{-1,-1},{-2, 0}},
                       {{-1, 1},{ 0, 0},{ 1, 1},{ 2, 0}},
                       {{ 1,-1},{ 0, 0},{-1,-1},{-2, 0}},
                       {{-1, 1},{ 0, 0},{ 1, 1},{ 2, 0}}};
int z_flip[4][4][2] = {{{-1, 1},{ 0, 0},{-1,-1},{ 0,-2}},
                       {{ 1,-1},{ 0, 0},{ 1, 1},{ 0, 2}},
                       {{-1, 1},{ 0, 0},{-1,-1},{ 0,-2}},
                       {{ 1,-1},{ 0, 0},{ 1, 1},{ 0, 2}}};
int t_flip[4][4][2] = {{{ 0, 0},{-2, 0},{-1,-1},{ 0,-2}},
                       {{ 1, 0},{ 1, 2},{ 0, 1},{-1, 0}},
                       {{-1,-1},{ 1,-1},{ 0, 0},{-1, 1}},
                       {{ 0, 1},{ 0,-1},{ 1, 0},{ 2, 1}}};

typedef int (*shape_array_flips)[4][2];

shape_array_flips flip_operations[] = { i_flip, j_flip, l_flip, o_flip, s_flip, z_flip, t_flip };

void reset_state() {
  state = 3;
}

int flip_legal(int active_piece[4][2], int board[24][10], int n) {
  if(state == 3) {
    state = 0;
  } else {
    state = state + 1;
  }
	for(i = 0; i < 4; i++) {
		if((active_piece[i][0] + flip_operations[n][state][i][0] < 0) || (active_piece[i][0] + flip_operations[n][state][i][0] > 23)) {
			return 0;
		}
		if((active_piece[i][1] + flip_operations[n][state][i][1] < 0) || (active_piece[i][1] + flip_operations[n][state][i][1] > 9)) {
			return 0;
		}
		if(board[active_piece[i][0] + flip_operations[n][state][i][0]][active_piece[i][1] + flip_operations[n][state][i][1]] == 1) {
			is_active_piece_flip = 0;
			for(j = 0; j < 4; j++) {
				if((active_piece[i][0] + flip_operations[n][state][i][0] == active_piece[j][0]) && (active_piece[i][1] + flip_operations[n][state][i][1] == active_piece[j][1])) {
					is_active_piece_flip = 1;
				}
			}
			if(!is_active_piece_flip) {
				return 0;
			}
		}
	}
	return 1;
}

void flip(int active_piece[4][2], int board[24][10], int n) {
  if(flip_legal(active_piece, board, n)) {
    for(i = 0; i < 4; i++) {
      board[active_piece[i][0]][active_piece[i][1]] = 0;
      active_piece[i][0] += flip_operations[n][state][i][0];
      active_piece[i][1] += flip_operations[n][state][i][1];
      board[active_piece[i][0]][active_piece[i][1]] = 1;
    }
  } else {
    if(state == 0) {
      state = 3;
    } else {
      state = state - 1;
    }
  }
}
