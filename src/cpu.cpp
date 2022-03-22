#include "cpu.h"

// define global variables
// MEMORY: has 4kb (4096) bytes of RAM in 8bit segments
// however, address 0x000 to 0x1FF are reserved - programs start at 0x200 (512)
std::vector<unsigned char> RAM(4096);

// STACK: origionally had only space 12 or 16 2-byte values
// makeing this larger won't hurt anything
std::vector<unsigned short> STACK(64);

// 16 bit program counter
unsigned short PC = 0;

// 16 bit memory index register
unsigned short IND = 0;

// 16 bit stack pointer register
unsigned short SP = 0;

// 16 bit current opcode register
unsigned short OPCODE = 0;

// 16 8-bit general purpose variable registers
// V0 - VF (0-15), VF is reserved as a flag register
std::vector<unsigned char> VAR(16);

// create a display of 64x32 pixels (64 wide, 32 tall)
// use a 1 to show on, 0 to show off
std::vector<std::vector<unsigned char>> display_matrix(32, std::vector<unsigned char>(64,0));

// font table
// this is the standard chip8 font table used by programs
// gets loaded in address 0x050 - 0x09F
std::vector<unsigned char> FONTS = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80}; // F

// function to load fonts to memory
int load_fonts()
{
    // fonts are loaded from the font table to RAM
    // address used: 0x050 to 0x09F
    int mem_val = 80;
    for(unsigned int i = 0; i < FONTS.size(); i++)
    {
        RAM.at(mem_val) = FONTS.at(i);
        mem_val = mem_val + 1;
    }
    return 0;
}

// function to load programs into program memory
int load_program(std::string filename, unsigned int memVal)
{
    // open the file:
    int fileSize;
    std::ifstream file(filename, std::ios::binary);

    // get its size:
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
    unsigned char fileData[fileSize];
    file.read((char*) &fileData[0], fileSize);
    file.close();

    for(int i = 0; i < fileSize; i++)
    {
        RAM[memVal] = fileData[i];
        memVal = memVal + 1;
    }
    return 0;
}

// function to generate random 8 bit number
unsigned char random_val()
{
    unsigned int ranVal = (unsigned char) rand();
    return ranVal;
}

// function to handle opcode 0 instructions
// 00E0 = clear screen
// 0NNN = execute machine language instruction
// 00EE = return from subroutine
int op0()
{
    // test if 0x00E0
    if(OPCODE == 0x00E0)
    {
        for(int i = 0; i < (int)display_matrix.size(); i++)
        {
            for(int j = 0; j < (int)display_matrix.at(i).size(); j++)
            {
                display_matrix.at(i).at(j) = 0;
            }
        }
        clear_screen();
    }
    // test if 0x00EE
    else if (OPCODE == 0x00EE)
    {
        // pop last PC from stack
        PC = STACK[SP];
        // decriment SP
        SP = SP - 1;
    }
    else
    {
        printf("%s","0x0NNN : ex ML inst - NOP\n");
    }
    return 0;
}

// function to handle opcode 1 instructions
// 1NNN = jump to addr NNN
int op1()
{
    // get NNN value
    // AND opcode with 00001111 11111111 (0x0FFF)
    unsigned short tmp = (OPCODE & 0x0FFF);
    // set program counter to tmp
    PC = tmp;
    return 0;
}

// function to handle opcode 2 instructions
// 2NNN = call subroutine at NNN
int op2()
{
    // incriment the stack pointer
    SP = SP + 1;
    // push current PC onto the stack
    STACK[SP] = PC;
    // get NNN value
    // AND opcode with 00001111 11111111 (0x0FFF)
    unsigned short tmp = (OPCODE & 0x0FFF);
    // set PC to tmp
    PC = tmp;
    return 0;
}

// function to handle opcode 3 instructions
// 3XNN = skip one 2-byte instruction if value in VX == NN
int op3()
{
    // extract X from opcode
    // shift right 8, then AND with 0x000F
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract NN from opcode
    unsigned char tmpn = (unsigned char)(OPCODE & 0x00FF);
    // compare VX and NN
    if (VAR[tmpx] == tmpn)
    {
        // if true, add 2 to PC to skip next instruction
        PC = PC + 2;
    }
    return 0;
}

// function to handle opcode 4 instructions
// 4XNN = skip one 2-byte instruction if value in VX != NN
int op4()
{
    // extract X from opcode
    // shift right 8, then AND with 0x000F
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract NN from opcode
    unsigned char tmpn = (unsigned char)(OPCODE & 0x00FF);
    // compare VX and NN
    if (VAR[tmpx] != tmpn)
    {
        // if true, add 2 to PC to skip next instruction
        PC = PC + 2;
    }
    return 0;
}

// function to handle opcode 5 instructions
// 5XY0 = skip one 2-byte instruction if value in VX == VY
int op5()
{
    // extract X from opcode
    // shift right 8, then AND with 0x000F
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract Y from opcode
    // shift right 4, then AND with 0x000F
    unsigned char tmpy = (unsigned char)((OPCODE >> 4) & 0x000F);
    // compare VX and VY
    if (VAR[tmpx] == VAR[tmpy])
    {
        // if true, add 2 to PC to skip next instruction
        PC = PC + 2;
    }
    return 0;
}

// function to handle opcode 9 instructions
// 9XY0 = skip one 2-byte instruction if value in VX != VY
int op9()
{
    // extract X from opcode
    // shift right 8, then AND with 0x000F
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract Y from opcode
    // shift right 4, then AND with 0x000F
    unsigned char tmpy = (unsigned char)((OPCODE >> 4) & 0x000F);
    // compare VX and VY
    if (VAR[tmpx] != VAR[tmpy])
    {
        // if true, add 2 to PC to skip next instruction
        PC = PC + 2;
    }
    return 0;
}

// function to handle opcode 6 instructions
// 6XNN = set register VX to NN
int op6()
{
    // extract X from opcode
    // shift right 8, then AND with 0x000F
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract NN from opcode
    unsigned char tmpn = (unsigned char)(OPCODE & 0x00FF);
    // set VX to NN
    VAR[tmpx] = tmpn;
    return 0;
}

// function to handle opcode 7 instructions
// 7XNN = add NN to VX
// NOTE: do not trigger an overflow flag or wrap-around
int op7()
{
    // extract X from opcode
    // shift right 8, then AND with 0x000F
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract NN from opcode
    unsigned char tmpn = (unsigned char)(OPCODE & 0x00FF);
    // add, do not trigger overflow flag
    VAR[tmpx] = VAR[tmpx] + tmpn;
    return 0;
}

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

int op8()
{
    // ambious instruction toggle
    int toggle = 0;
    // extract X from opcode
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract Y from opcode
    unsigned char tmpy = (unsigned char)((OPCODE >> 4) & 0x000F);
    // extract function flag
    unsigned char tmpf = (unsigned char)(OPCODE & 0x000F);
    // do function based on function flag
    switch (tmpf)
    {
    case 0:
        // set
        VAR[tmpx] = VAR[tmpy];
        break;
    case 1:
        // OR
        VAR[tmpx] = (VAR[tmpx] | VAR[tmpy]);
        break;
    case 2:
        // AND
        VAR[tmpx] = (VAR[tmpx] & VAR[tmpy]);
        break;
    case 3:
        // XOR
        VAR[tmpx] = (VAR[tmpx] ^ VAR[tmpy]);
        break;
    case 4:
        // ADD
        // check for overflow
        if (VAR[tmpx] + VAR[tmpy] >= 255)
        {
            VAR[tmpx] = VAR[tmpx] + VAR[tmpy];
            // set VF register to signal overflow
            VAR[15] = 1;
        }
        else
        {
            VAR[tmpx] = VAR[tmpx] + VAR[tmpy];
            VAR[15] = 0;
        }
        break;
    case 5:
        // VX = VX - VY
        // if VX > VY, then VF is set to 1, otherwise 0
        if (VAR[tmpx] > VAR[tmpy])
        {
            VAR[15] = 1;
            VAR[tmpx] = (VAR[tmpx] - VAR[tmpy]);
        }
        else
        {
            VAR[15] = 0;
            VAR[tmpx] = (VAR[tmpx] - VAR[tmpy]);
        }
        break;
    case 7:
        // VX = VY - VX
        // if VY > VX, then VF is set to 1, otherwise 0
        if (VAR[tmpy] > VAR[tmpx])
        {
            VAR[15] = 1;
            VAR[tmpx] = (VAR[tmpy] - VAR[tmpx]);
        }
        else
        {
            VAR[15] = 0;
            VAR[tmpx] = (VAR[tmpy] - VAR[tmpx]);
        }
        break;
    case 6:
        // VX = VX >> 1
        // VF = shifted out bit
        // first, check function toggle
        if (toggle)
        {
            // early chip8 would actually set VX to VY then shift
            // VX = VY; VX = VX >> 1
            // set VX to VY
            VAR[tmpx] = VAR[tmpy];
            // set VF to right-most bit
            VAR[15] = VAR[tmpx] & 0x01;
            // shift right 1
            VAR[tmpx] = VAR[tmpx] >> 1;
        }
        else
        {
            // "modern" chip[8 implimentation
            // VX = VX >> 1
            // set VF to right-most bit
            VAR[15] = VAR[tmpx] & 0x01;
            // shift right 1
            VAR[tmpx] = VAR[tmpx] >> 1;
        }
        break;
    case 14:
        // VX = VX << 1
        // VF = shifted out bit
        // first, check function toggle
        if (toggle)
        {
            // early chip8 would actually set VX to VY then shift
            // VX = VY; VX = VX << 1
            // set VX to VY
            VAR[tmpx] = VAR[tmpy];
            // set VF to left-most bit
            VAR[15] = VAR[tmpx] & 0x80;
            // shift left 1
            VAR[tmpx] = VAR[tmpx] << 1;
        }
        else
        {
            // "modern" chip[8 implimentation
            // VX = VX >> 1
            // set VF to left-most bit
            VAR[15] = ((VAR[tmpx] & 0x80) >> 7);
            // shift left 1
            VAR[tmpx] = VAR[tmpx] << 1;
        }
        break;
    default:
        break;
    }
    return 0;
}

// handle opcode A instructions
// ANNN = set index register to NNN
int op10()
{
    // extract NNN from OPCODE
    unsigned short tmpn = OPCODE & 0x0FFF;
    // set index to NNN
    IND = tmpn;
    return 0;
}

// handle opcode B instructions
// BNNN = jump with offset
// jump to NNN + value in V0 (used for jump table operations)
// note: this command was handeled in a different manner in other chip implimentations
// this is the most common method
int op11()
{
    // extract NNN from OPCODE
    unsigned short tmpn = OPCODE & 0x0FFF;
    // add V0 to tmpn;
    tmpn = tmpn + VAR[0];
    // jump to tmpn
    PC = tmpn;
    return 0;
}

// handle opcode C instructions
// CXNN = generates a random number, binary ANDs it with the value NN
// store in VX
int op12()
{
    // extract NN from OPCODE
    unsigned char tmpn = (unsigned char)OPCODE & 0x00FF;
    // extract VX
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // get random numper
    unsigned char tmpr = random_val();
    // AND with NN
    tmpr = tmpr & tmpn;
    // set VX
    VAR[tmpx] = tmpr;
    return 0;
}

// handle opcode D instructions
// DXYN = display sprite:
// N = number of pixels tall, starting at memory pointed to by I register
// X = starting X coordinate
// Y = starting Y coordinate
int op13()
{
    // extract X from opcode
    unsigned char tmpx = (unsigned char)VAR[((OPCODE >> 8) & 0x000F)] % 64;
    // extract Y from opcode
    unsigned char tmpy = (unsigned char)VAR[((OPCODE >> 4) & 0x000F)] % 32;
    // extract N
    unsigned char tmpn = (unsigned char)(OPCODE & 0x000F);
    // temp variable to store pixel data and count
    unsigned char tmpp = 0;
    unsigned char pxl = 0;
    // initial pixel colide state is zero
    VAR[15] = 0;
    // loop through N number of bytes, if N = 0 stop
    // dont wrap around bottom of screen
    for (unsigned int i = 0; i < tmpn; i++)
    {
        // test if overflow Y
        if (tmpy > 31)
        {
            //printf("%s","display overflow Y\n");
            break;
        }
        // get pixel data
        tmpp = RAM[IND+i];
        // loop through pixel data
        for (unsigned int j = 0; j < 8; j++)
        {
            // test if overflow X
            if (tmpx+j > 63)
            {
                //printf("%s","display overflow X\n");
                break;
            }
            // extract data from pixel byte
            pxl = ((tmpp << j) & 0x80) >> 7;
            // set pixel data to screen
            if (pxl == 1)
            {
                // if 1
                if (display_matrix.at(tmpy).at(tmpx+j) == 1)
                {
                    display_matrix.at(tmpy).at(tmpx+j) = 0; // erased
                    // set VF to 1
                    VAR[15] = 1;
                }
                else
                {
                    display_matrix.at(tmpy).at(tmpx+j) = 1;
                }
            }
        }
        tmpy = tmpy+1;
    }
    // draw the screen
    draw_screen_vector(display_matrix);
    return 0;
}

// handle opcode E instructions
// EX9E = skip one instruction if the key value in X is pressed (1)
// EXA1 = skip one instruction if the key value in X is not pressed (0)
int op14(std::vector<unsigned char> &keys)
{
    // extract value from X from opcode
    unsigned char tmpx = (unsigned char)VAR[((OPCODE >> 8) & 0x000F)];
    // extract toggle from opcode
    unsigned char tmpt = (unsigned char)OPCODE & 0x000F;

    if (tmpt == 1)
    {
        // EXA1 - skip if not pressed
        // check if not pressed
        if (keys.at(tmpx) == 0)
        {
            // checks if key input register (keys) at the value in X is 0
            // ex: if VX = 15, checks if F is 0
            PC = PC + 2; // skip next instruction
        }
    }
    else
    {
        // EX9E - skip if pressed
        // check if not pressed
        if (keys.at(tmpx) == 1)
        {
            PC = PC + 2; // skip next instruction
        }
    }
    return 0;
}

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
int op15(std::vector<unsigned char> &keys, unsigned char &DEL_TIME, unsigned char &SOUND_TIME)
{
    // extract X from opcode
    unsigned char tmpx = (unsigned char)((OPCODE >> 8) & 0x000F);
    // extract toggle from opcode
    unsigned short tmpt = (unsigned short)OPCODE & 0x00FF;
    // temp variable for key press
    unsigned int pressed = 0;

    // case statement to select specific instruction
    switch (tmpt)
    {
    case 0x07:
        // set X to current value in delay timer
        VAR[tmpx] = DEL_TIME;
        break;
    case 0x15:
        // set delay timer to value in X
        DEL_TIME = VAR[tmpx];
        break;
    case 0x18:
        // set sound timer to value in X
        SOUND_TIME = VAR[tmpx];
        break;
    case 0x1E:
        // add value in X to IND
        // if above 1000 (outside the normal addressing range), set F to 1
        IND = IND + VAR[tmpx];
        if (IND > 0x0FFF)
        {
            VAR[15] = 1;
        }
        break;
    case 0x0A:
        // loop through keys, if one is pressed (1) stop and set to X
        // else set PC to PC - 2 (blocking call)
        for (unsigned int i = 0; i < (unsigned int)keys.size(); i++)
        {
            if (keys.at(i) == 1)
            {
                pressed = i;
                break;
            }
        }
        if (pressed > 0)
        {
            // key pressed - set X to key
            VAR[tmpx] = pressed;
        }
        else
        {
            // dec PC
            PC = PC -2;
        }
        break;
    case 0x29:
        // index register = address of hex font in X
        // only uses last nibble in X
        // font starts at 0x50 and each is 5 bytes long
        IND = 0x50 + (5*(VAR[tmpx] & 0x0F));
        break;
    case 0x33:
        // BCD conversion
        // number in X is split into 100s, 10s, and 1s stored in IND-IND+2
        // ex: VAR[x] = 123
        // IND = 1
        // IND + 1 = 2
        // IND + 2 = 3
        RAM[IND + 0] = ((VAR[tmpx] / 100) % 10);
        RAM[IND + 1] = ((VAR[tmpx] / 10) % 10);
        RAM[IND + 2] = (VAR[tmpx] % 10);
        break;
    case 0x55:
        // store all registers in memory
        // dont change IND, use a temp variable
        for (unsigned int i = 0; i <= (unsigned int)tmpx; i++)
        {
            // iterate through VAR, and save to RAM at IND + i
            RAM[IND+i] = VAR[i];
        }
        break;
    case 0x65:
        // load all registers in memory
        for (unsigned int i = 0; i <= (unsigned int)tmpx; i++)
        {
            // iterate through VAR, and save RAM at IND + i to VAR
            VAR[i] = RAM[IND+i];
        }
        break;
    default:
        break;
    }
    return 0;
}

int init_CPU(int &xval, char* fval)
{
    // load font into memory
    load_fonts();
    printf("%s","Loaded fonts into memory\n");
    // load program into memory
    load_program(fval, 512);
    //load_program("bc_test.ch8", 512);
    printf("%s","Loaded program into memory\n");
    // init the screen
    SDL_screen_init(xval);

    // set PC to program start
    PC = 512;

    // initialize random seed
    srand (time(NULL));
    return 0;
}

int CPU_cycle(std::vector<unsigned char> &keys, unsigned char &DEL_TIME, unsigned char &SOUND_TIME)
{
    // fetch instruction (16 bit from 2 8-bit memory locations)
    OPCODE = (RAM[PC] << 8) | RAM[PC+1];
    // incriment PC by 2
    PC = PC + 2;
    // decode instruction
    // instructions are broken up by opcode and operand
    // get first nibble for opcode, save as op
    unsigned char op = OPCODE >> 12;
    //printf("%x\n",OPCODE);
    // execute instructions
    // switch statment to call different functions based on the opcode
    // 0 = clear screen, subroutine return, or a NOP
    // 1 = jump
    // 2 = subroutine call
    // 3/4/5/9 = various skip instruction functions
    // 6 = set function
    // 7 = add function
    // 8 = register atrithmatic/logical functions
    // A = set index (10)
    // B = jump with offset (11)
    // C = random number gen (12)
    // D = display (13)
    // E = skip if key press (14)
    // F = timer/index add/get key/font char/BCD conversion/load and store memory (15)
    switch (op)
    {
    case 0:
        if (OPCODE == 0) // check for all zero opcode = unallocated memory
        {
            printf("%s","ERROR: NULL OPCODE DETECTED\n");
            return 1;
        }
        op0();
        break;
    case 1:
        op1();
        break;
    case 2:
        op2();
        break;
    case 3:
        op3();
        break;
    case 4:
        op4();
        break;
    case 5:
        op5();
        break;
    case 6:
        op6();
        break;
    case 7:
        op7();
        break;
    case 8:
        op8();
        break;
    case 9:
        op9();
        break;
    case 10:
        op10();
        break;
    case 11:
        op11();
        break;
    case 12:
        op12();
        break;
    case 13:
        op13();
        break;
    case 14:
        op14(keys);
        break;
    case 15:
        op15(keys, DEL_TIME, SOUND_TIME);
        break;
    default:
        break;
    }
    return 0;
}
