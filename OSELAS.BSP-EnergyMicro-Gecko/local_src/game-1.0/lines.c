#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int i;
int j;
int check;
int cleared_lines[4] = { 0 };

void check_lines(int active_piece[4][2], int board[24][10]) {
  for(i = 0; i < 4; i++) {
    check = 1;
    for(j = 0; j < 10; j++) {
      if(board[active_piece[i][0]][j] == 0) {
        check = 0;
        break;
      }
    }
    if(check == 0) {
      continue;
    } else {
      cleared_lines[i] = active_piece[i][0];
      printf("%d\n", cleared_lines[i]);
      for(j = 0; j < 10; j++) {
        board[active_piece[i][0]][j] = 0;
      }
    }
  }
  for(i = 0; i < 4; i++) {
    if(cleared_lines[i] > 0) {
      for(j = 23 - cleared_lines[i]; j < 20; j++) {
        memcpy(board[23-j], board[22-j], sizeof(board[0]));
      }
    }
    cleared_lines[i] = 0;
  }
}
