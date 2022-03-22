// chip8 emulator written in cpp
// written "for fun" as a first emulator project

// includes
//#include <vector>
//#include <fstream>
//#include <stdio.h>
//#include <stdlib.h>
#include <time.h>
#include <thread>
#include <getopt.h>
#include "cpu.h"
#include "iohandle.h"

// shutdown indicator
bool shutdown_flag = false;

// vector to store keystates
std::vector<unsigned char> KEYS(16);

// 8 bit delay timer
unsigned char DEL_TIME = 0;

// 8-bit sound timer
unsigned char SOUND_TIME = 0;

// arguments
int c;
int hflag = 0;
int sval = 0;
int xval = 0;
int kflag = 0;
char *fval = NULL;

// main cpu function
void cpu_thread()
{
    printf("%s","CPU thread started\n");
    // first, init the CPU
    init_CPU(xval, fval);
    // pause to let screen init
    nanosleep((const struct timespec[]){{1, 0L}}, NULL);

    // run CPU cycles in a loop, as a long as shutdown variable is false
    printf("%s","running...\n");
    while (!shutdown_flag)
    {
        if(CPU_cycle(KEYS, DEL_TIME, SOUND_TIME) != 0)
        {
            // CPU cycle return non-zero
            // shutdown bool = true
            shutdown_flag = true;
        }
        // throttle the CPU - sleep until time for next clock cycle
        // this is essentially the "clock"
        switch (sval)
        {
        case 0:
            nanosleep((const struct timespec[]){{0, 10000L}}, NULL);
            break;
        case 1:
            nanosleep((const struct timespec[]){{0, 100000L}}, NULL);
            break;
        case 2:
            nanosleep((const struct timespec[]){{0, 1000000L}}, NULL);
            break;
        default:
            nanosleep((const struct timespec[]){{0, 100000L}}, NULL);
            break;
        }
    }
}

// input handler thread
void input_thread()
{
    printf("%s","Input thread started\n");
    // pause to let screen init
    nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
    // loop while the shudown flag is off
    while (!shutdown_flag)
    {
        // get input to shutdown bool (exit event) and KEYS (keystate event)
        SDL_input_event_handler(shutdown_flag, KEYS, kflag);
        // sleep 10ns just as a simple throttle to avoid a constant poll
        nanosleep((const struct timespec[]){{0, 10L}}, NULL);
    }
    // exit while loop == shutdown
    SDL_screen_close();
}

// timer thread to handle the sound and delay timers
// decriment at a rate of 60Hz
void timer_thread()
{
    printf("%s","Timer thread started\n");
    while (!shutdown_flag)
    {
        // check if DEL_TIME is non-zero
        if (DEL_TIME > 0)
        {
            // subtract 1
            DEL_TIME = DEL_TIME - 1;
        }
        // check if SOUND_TIME is non-zero
        if (SOUND_TIME > 0)
        {
            // subtract 1
            SOUND_TIME = SOUND_TIME - 1;
        }
        // sleep for 1/60th of a second
        nanosleep((const struct timespec[]){{0, 16666666L}}, NULL);
    }
}

int main(int argc, char* argv[])
{
    // check if no arguments
    if (argc < 2)
    {
        printf("%s\n","======== HELP MENU ========");
        printf("%s\n","basic chip8 emulator:");
        printf("%s\n","-h: displays this help text");
        printf("%s\n","-f: .ch8 file to run");
        printf("%s\n","-s: emulation clock speed (0, 1, 2)");
        printf("%s\n","    0 = slower");
        printf("%s\n","    1 = default");
        printf("%s\n","    2 = faster");\
        printf("%s\n","-x: pixel graphics size (recommend 10 or 20)");
        printf("%s\n","-k: use custom tetris keybindings (optional argument)");
        printf("%s\n","EXAMPLES:");
        printf("%s\n","./chip8 -fkeypad.ch8 -s1 -x10");
        printf("%s\n","[run keypad test with default speed, pixel size 10, and default keys]");
        printf("%s\n","./chip8 -ftetris.ch8 -s1 -x20 -k");
        printf("%s\n","[run tetris with default speed, pixel size 20, and tetris keys]");
        return 0;
    }

    // parse opts with "getopt"
    while((c = getopt(argc, argv, "hks:x:f:")) != -1) 
    {
        switch(c)
        {
            case 'h':
                hflag = 1;
                break;
            case 'k':
                kflag = 1;
                break;
            case 's':
                sval = atoi(optarg);
                
                if((sval != 0) & (sval != 1) & (sval != 2))
                {
                    printf("%s","invalid sval, setting to 1\n");
                    sval = 1;
                }
                break;
            case 'x':
                xval = atoi(optarg);
                
                if((xval == 0) | (xval > 50))
                {
                    printf("%s","invalid xval, setting to 10\n");
                    xval = 10;
                }
                break;
            case 'f':
                fval = optarg;
                break;
            default:
                break;
        }
    }

    // if hflag set, print help menu and exit    
    if (hflag == 1)
    {
        printf("%s\n","======== HELP MENU ========");
        printf("%s\n","basic chip8 emulator:");
        printf("%s\n","-h: displays this help text");
        printf("%s\n","-f: .ch8 file to run");
        printf("%s\n","-s: emulation clock speed (0, 1, 2)");
        printf("%s\n","    0 = slower");
        printf("%s\n","    1 = default");
        printf("%s\n","    2 = faster");\
        printf("%s\n","-x: pixel graphics size (recommend 10 or 20)");
        printf("%s\n","-k: use custom tetris keybindings (optional argument)");
        printf("%s\n","EXAMPLES:");
        printf("%s\n","./chip8 -f./roms/keypad.ch8 -s1 -x10");
        printf("%s\n","[run keypad test (in ./roms/) with default speed, pixel size 10, and default keys]");
        printf("%s\n","./chip8 -f./roms/tetris.ch8 -s1 -x20 -k");
        printf("%s\n","[run tetris (in ./roms/) with default speed, pixel size 20, and tetris keys]");
        return 0;    
    }
    printf("%s","chip8 main started\n");
    printf("args: k = %d, s = %d, x = %d, file = %s\n", kflag, sval, xval, fval);

    // make cpu and input threads
    std::thread cpu_thread_obj(cpu_thread);
    std::thread input_thread_obj(input_thread);
    std::thread timer_thread_obj(timer_thread);

    // wait for threads to join
    cpu_thread_obj.join();
    input_thread_obj.join();
    timer_thread_obj.join();
    printf("%s","threads joined - exiting\n");

    return 0;
}