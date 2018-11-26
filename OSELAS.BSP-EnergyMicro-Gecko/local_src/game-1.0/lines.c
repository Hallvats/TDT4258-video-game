#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int i;
int j;
int check;
int cleared_lines[4] = { 0 };

/*
   Checks the board to see if any lines are full.
   Removes any that are and moves the above lines down.
*/
void check_lines(int active_piece[4][2], int board[24][10]) {
  /* First for loop sets cleared_lines' values if it finds full lines */
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
      /* Removes full lines set in cleared_lines */
      cleared_lines[i] = active_piece[i][0];
      for(j = 0; j < 10; j++) {
        board[active_piece[i][0]][j] = 0;
      }
    }
  }
  /* For every line removed, moves all above lines down one step */
  for(i = 0; i < 4; i++) {
    if(cleared_lines[i] > 0) {
      for(j = 23 - cleared_lines[i]; j < 20; j++) {
        memcpy(board[23-j], board[22-j], sizeof(board[0])); // This method copies a line to the line below
      }
    }
    cleared_lines[i] = 0;
  }
}
