#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define CLOCK 0
#define COUNTER 1
#define TEXT 2
#define BOARD 3
#define SPIN 4
#define MAZE 5



#define BUF_SIZE 64

#define KEY_RELEASE 0
#define KEY_PRESS 1

#define KEY_DEVICE "/dev/input/event0"
#define SWITCH_DEVICE "/dev/fpga_push_switch"

#define FND_DEVICE "/dev/fpga_fnd"
#define LCD_DEVICE "/dev/fpga_text_lcd"
#define LED_DEVICE "/dev/mem"
#define DOT_DEVICE "/dev/fpga_dot"
#define MOT_DEVICE "/dev/fpga_step_motor"
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16
#define MAX_SWITCH 9
#define MAX_LCD 32
#define DELAY 30000
#define SEC 26300

typedef struct _NODE{
    char row;
    char col;
}NODE;

NODE stack[70];
char top;


