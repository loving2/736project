#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

#include "frame.h"
#include "readwrite.h"
#include "rs485uart.h"

#define VERBOSE 1

// Remove newline if it exists
void chomp(char *str) {
    char *pos;
    if ((pos=strchr(str, '\n')) != NULL)
        *pos = '\0';
}

// Signal handler to clean up nicely
static volatile int keepRunning = 1;
void intHandler(int dummy) {
    keepRunning = 0;
}

int main(void){

    //initialize serial comms variables
    char cmd[MAXBUFFER];
    rs485uart *uart = &UART4;
    
    if (rs485uart_open(uart) != 0) {
        printf("ERROR: couldn't initialize UART!\n");
        return -1;
    }

    // Loop forever, writing command and reading response
    signal(SIGINT, intHandler);
    Frame readframe, writeframe;    
    while (keepRunning) {
        printf("What is your command?\n");
        fgets(cmd, MAXBUFFER, stdin);
        chomp(cmd);
        if ((strcmp(cmd, "exit") == 0) || (!keepRunning))
            break;

        // Write command
        frameinit(&writeframe, cmd);
        int check = frame_write(&writeframe, uart->fd);
        if (VERBOSE)
            printf("WR: %s\n", cmd);
        if (check < 0)
            printf("Error writing\n");

        // Read response
        frameinit(&readframe, "read");
        if (frame_read(&readframe, uart->fd) != 0) {
            printf("Error reading\n");
        }
        else {
            printframe(&readframe);
            if (VERBOSE && (strlen(readframe.msg) > 0))
                printf("RD: %s\n", readframe.msg);
        }        
    }
    rs485uart_close(&UART4);
    return 0;
}
