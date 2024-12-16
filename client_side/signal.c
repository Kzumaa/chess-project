#include "signal.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "board.h"

#define RED "\x1B[31m"
#define RESET "\x1B[0m"
#define GREEN "\x1B[32m"

void print_error_message(char error_code) {
    switch (error_code) {
        case '0':
            mvprintw(15, 15, RED "  ↑ ('-' missing)\n" RESET);
            break;
        case '1':
            mvprintw(15, 15, RED "↑ (should be letter)\n" RESET);
            break;
        case '2':
            mvprintw(15, 15, RED "   ↑ (should be letter)\n" RESET);
            break;
        case '3':
            mvprintw(15, 15, RED " ↑ (should be number)\n" RESET);
            break;
        case '4':
            mvprintw(15, 15, RED " ↑ (out of range)\n" RESET);
            break;
        case '5':
            mvprintw(15, 15, RED "   ↑ (should be number)\n" RESET);
            break;
        case '6':
            mvprintw(15, 15, RED "   ↑ (out of range)\n" RESET);
            break;
        case '7':
            mvprintw(15, 15, RED "(out of range)\n" RESET);
            break;
        default:
            break;
    }
}

void handle_informative_message(char *buffer, int *player) {
    // mvprintw(14,15, buffer);
    if (buffer[0] == 'i') {
        if (buffer[2] == 't') {
            mvprintw(15, 15, "\nMake your move: \n");
        }
        if (buffer[2] == 'n') {
            mvprintw(15, 15, "\nWaiting for opponent...\n");
        }
        // if (buffer[2] == 'p') {
        //     *player = atoi(&buffer[3]);
        //     if (*player == 2) {
        //         // mvprintw(15, 15, "You're blacks (%c)\n", buffer[3]);
        //     } else {
        //         // mvprintw(15, 15, "You're whites (%c)\n", buffer[3]);
        //     }
        // }
    }
    if (buffer[2] == 't') {
        mvprintw(15, 15, "\nMake your move: \n");
    }
    if (buffer[2] == 'n') {
        mvprintw(15, 15, "\nWaiting for opponent...\n");
    }
    refresh();
    // mvprintw(15,15, "INFORMATIVE");
}

void handle_error_message(char *buffer) {
    if (buffer[0] == 'e' && buffer[2] == '0') {
        print_error_message(buffer[3]);
    }
    mvprintw(15, 15, "\nerror %s\n", buffer);
    refresh();
}

void handle_game_end(char *buffer, void *sockfd) {
    int socket = *(int *)sockfd;
    if (buffer[0] == 'w') {
        clear();
        mvprintw(0, 0, "You win\n");
        refresh();
        write(socket, "rematch", 8);
        // exit(1);
    } else if (buffer[0] == 'l') {
        clear();
        mvprintw(0, 0, "You lose\n");
        refresh();
        write(socket, "rematch", 8);
        // exit(1);
    } else if (buffer[0] == 'b') {
        mvprintw(15, 15, "Your opponent has left the room. Please leave here\n");
        // exit(1);
    } else if (buffer[0] == 'g' && buffer[2] == 'e') {
        mvprintw(15, 15, "The opponent surrendered. Yes or No\n");
    } else if (buffer[0] == 'g' && buffer[2] == 'o') {
        mvprintw(15, 15, "You lose\n");
        // exit(1);
    } else if (buffer[0] == 'g' && buffer[2] == 'n') {
        mvprintw(15, 15, "The opponent refused. Let's move\n");
    }
    refresh();
}

void handle_board(char *buffer, int player) {
    // system("clear");
    clear();
    if (player == 1) {
        print_board_buff(buffer);
    } else {
        print_board_buff_inverted(buffer);
    }
    // mvprintw(15, 15, "RENDERED BOARD\n");
    // mvprintw(22, 60, "BOARD");
    refresh();
    // refresh();
}

void *on_signal(void *sockfd) {
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // Black squares
    init_pair(2, COLOR_BLACK, COLOR_WHITE);  // White squares
    init_pair(5, COLOR_BLUE, COLOR_BLACK);   // White pieces on black squares
    init_pair(6, COLOR_RED, COLOR_BLACK);    // Black pieces on black squares
    init_pair(7, COLOR_BLUE, COLOR_WHITE);   // White pieces on white squares
    init_pair(8, COLOR_RED, COLOR_WHITE);    // Black pieces on white squares

    char buffer[64];
    int n;
    int socket = *(int *)sockfd;
    int *player = (int *)malloc(sizeof(int));

    // clear();

    // if(*player == 1){
    //     mvprintw(15, 15, "You're whites\n");
    // } else {
    //     mvprintw(15, 15, "You're blacks\n");
    // }

    while (1) {
        bzero(buffer, 64);
        n = read(socket, buffer, 64);
        if(strcmp(buffer, "quit") == 0){
            endwin();
            exit(0);
        }
        // clear();
        // mvprintw(15, 15, "Enter your chess-move:\n");

        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        if (buffer[0] != 'i' && buffer[0] != 'e' && buffer[0] != 'w' && buffer[0] != 'l' && buffer[0] != 'b' && !(buffer[0] == 'g' && buffer[2] == 'e')) {
            handle_board(buffer, *player);
            handle_informative_message(buffer, player);
        }

        // handle_board(buffer, *player);
        // Handle informative messages
        // handle_informative_message(buffer, player);

        // // Get chess move input from keyboard

        // // Handle error messages
        if (buffer[0] == 'e' && strcmp(buffer, "resign") != 0) {
            handle_error_message(buffer);
        }

        // Handle game ending scenarios
        handle_game_end(buffer, sockfd);

        // Handle board display

        bzero(buffer, 64);
    }

    endwin();

    return NULL;
}
