// CPU program to handle the chip8 cpu

// includes
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "iohandle.h"

// function to init the cpu
int init_CPU(int &xval, char* fval);

// perform a single fetch-decode-ex CPU cycle
int CPU_cycle(std::vector<unsigned char> &keys, unsigned char &DEL_TIME, unsigned char &SOUND_TIME);

// function to load fonts to memory
int load_fonts();

// function to load programs into program memory
int load_program(std::string filename, unsigned int memVal);

// function to generate random 8 bit number
unsigned char random_val();

// function to handle opcode 0 instructions
// 00E0 = clear screen
// 0NNN = execute machine language instruction
// 00EE = return from subroutine
int op0();

// function to handle opcode 1 instructions
// 1NNN = jump to addr NNN
int op1();

// function to handle opcode 2 instructions
// 2NNN = call subroutine at NNN
int op2();

// function to handle opcode 3 instructions
// 3XNN = skip one 2-byte instruction if value in VX == NN
int op3();

// function to handle opcode 4 instructions
// 4XNN = skip one 2-byte instruction if value in VX != NN
int op4();

// function to handle opcode 5 instructions
// 5XY0 = skip one 2-byte instruction if value in VX == VY
int op5();

// function to handle opcode 9 instructions
// 9XY0 = skip one 2-byte instruction if value in VX != VY
int op9();

// function to handle opcode 6 instructions
// 6XNN = set register VX to NN
int op6();

// function to handle opcode 7 instructions
// 7XNN = add NN to VX
// NOTE: do not trigger an overflow flag or wrap-around
int op7();

// function to handle opcode 8 instructions
// 8XYF = F function flag as shown below
// 8XY0 = set VX with VY (VX = VY)
// 8XY1 = VX = VX OR VY (VY unaffected)
// 8XY2 = VX = VX AND VY (VY unaffected)
// 8XY3 = VX = VX XOR VY (VY unaffected)
// 8XY4 = ADD: VX = VX + VY (VY unaffected)(does set/clear carry flag)
// 8XY5 = SUB: VX = VX - VY (does alter carry flag)
// 8XY7 = SUB: VX = VY - VX (does alter carry flag)
// 8XY6 = shift right: VX = VX >> 1 (does alter carry flag)
// 8XYE = shift left: VX = VX << 1 (does alter carry flag)
int op8();

// handle opcode A instructions
// ANNN = set index register to NNN
int op10();

// handle opcode B instructions
// BNNN = jump with offset
// jump to NNN + value in V0 (used for jump table operations)
// note: this command was handeled in a different manner in other chip implimentations
// this is the most common method
int op11();

// handle opcode C instructions
// CXNN = generates a random number, binary ANDs it with the value NN
// store in VX
int op12();

// handle opcode D instructions
// DXYN = display sprite:
// N = number of pixels tall, starting at memory pointed to by I register
// X = starting X coordinate
// Y = starting Y coordinate
int op13();

// handle opcode E instructions
// EX9E = skip one instruction if the key value in X is pressed (1)
// EXA1 = skip one instruction if the key value in X is not pressed (0)
int op14(std::vector<unsigned char> &keys);

// handle opcode F instructions
// FX07 = sets VAR X to the current value of the delay timer
// FX15 = sets the delay timer to the value in X
// FX18 = sets the sound timer to the value in X
// FX1E = adds the value in X to the index register
// FX0A = blocks until a key is pressed, and puts key value into X
// FX29 = point to font character X
// FX33 = BCD operation (see code)
// FX55 = store memory
// FX65 = load memory
int op15(std::vector<unsigned char> &keys, unsigned char &DEL_TIME, unsigned char &SOUND_TIME);
