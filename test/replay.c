// replay.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>  // Include ncurses library for terminal graphics
#include "board.h"    // Include header file to use print_board_buff

// Function to parse and make a move
void make_move(char *board, const char *move) {
    printw("Move: %s\n", move);  // Print the move to the terminal
    int start_col = move[0] - 'a';  // Convert 'a' to 0, 'b' to 1, etc.
    int start_row = 8 - (move[1] - '0');  // Convert '1' to 7, '2' to 6, etc.
    int end_col = move[3] - 'a';
    int end_row = 8 - (move[4] - '0');

    // Move piece
    board[end_row * 8 + end_col] = board[start_row * 8 + start_col];
    board[start_row * 8 + start_col] = ' ';  // Empty the start position
}

// Function to reverse a move (for moving backward)
void reverse_move(char *board, const char *move) {
    int start_col = move[3] - 'a';  // Reverse the move
    int start_row = 8 - (move[4] - '0');
    int end_col = move[0] - 'a';
    int end_row = 8 - (move[1] - '0');

    // Reverse the piece move
    board[end_row * 8 + end_col] = board[start_row * 8 + start_col];
    board[start_row * 8 + start_col] = ' ';
}

void cleanup() {
    endwin(); // This will restore the terminal to its original state
}

int main() {
    // Initialize ncurses
    initscr();                // Start ncurses mode
    atexit(cleanup);
    cbreak();                 // Disable line buffering
    noecho();                 // Don't echo input
    curs_set(0);              // Hide cursor
    // keypad(stdscr, TRUE);     // Enable special keys (but we're using 'a' and 'd' now)
    
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // Black squares
    init_pair(2, COLOR_BLACK, COLOR_WHITE);  // White squares
    init_pair(5, COLOR_BLUE, COLOR_BLACK);   // White pieces on black squares
    init_pair(6, COLOR_RED, COLOR_BLACK);    // Black pieces on black squares
    init_pair(7, COLOR_BLUE, COLOR_WHITE);   // White pieces on white squares
    init_pair(8, COLOR_RED, COLOR_WHITE);    // Black pieces on white squares

    // Initialize chessboard with the new piece IDs
    char board[64] = {
        '1', '2', '3', '4', '5', '3', '2', '1',  // White pieces: R, N, B, Q, K, B, N, R
        '6', '6', '6', '6', '6', '6', '6', '6',  // White pawns: P
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // Empty spaces
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // Empty spaces
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // Empty spaces
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // Empty spaces
        'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c',  // Black pawns: p
        'b', '8', '9', 'a', 'b', '9', '8', '7'   // Black pieces: r, n, b, q, k, b, n, r
    };

    // Open the log file (relative path from the test directory)
    FILE *file = fopen("../chess_log.txt", "r");  // Go up one directory to access the log file
    if (!file) {
        perror("Failed to open log file");
        endwin();  // End ncurses mode before exiting
        return 1;
    }

    char line[256];
    char moves[128][6];  // Store moves (e.g., "e2-e4")
    int move_count = 0;

    // Read all moves from the log into the moves array
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "move:") != NULL) {
            // Extract the move from the line
            sscanf(line, "%*s %*s %5s", moves[move_count]);
            move_count++;
        }
    }

    fclose(file);  // Close the log file

    // Print initial chessboard
    print_board_buff(board);

    int current_move = 0;  // Start at the first move
    while (1) {
        // Wait for user input (press 'a' for backward, 'd' for forward)
        // int ch = getch();
        // printf("ch: %d\n", ch);
        int ch = getchar();
        // mvprintw(0, 0, "Press 'a' to move backward, 'd' to move forward, or 'q' to quit");
        if (ch == 'd' && current_move < move_count) {  // Move forward with 'd'
            // Make the current move
            
            make_move(board, moves[current_move]);
            // for(int i = 0; i < 8; i++){
            //     for(int j = 0; j < 8; j++){
            //         printf("%c ", board[i*8 + j]);
            //     }
            //     printf("\n");
            // }

            // Print the updated chessboard
            // refresh();  // Refresh the screen
            clear();
            print_board_buff(board);
            printw("Move: %s\n", moves[current_move]);
            // print_board_buff_inverted(board);

            // Move to the next move in the log
            current_move++;
        }
        else if (ch == 'a' && current_move > 0) {  // Move backward with 'a'
            // Reverse the current move
            reverse_move(board, moves[current_move - 1]);

            // Print the updated chessboard
            print_board_buff(board);

            // Move to the previous move in the log
            current_move--;
        }
        // Optionally, you can add a key for quitting
        if (ch == 'q') {  // If 'q' is pressed, quit the program
            break;
        }
        if(current_move >= move_count){
            break;
        }

        // Small delay to make sure the user can see the update
        usleep(500000);  // Delay 0.5 seconds between moves
    }

    // End ncurses mode and clean up
    endwin();
    return 0;
}
