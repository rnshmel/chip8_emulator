#OBJS specifies which files to compile as part of the project
OBJS0 = ./src/chip8.cpp ./src/iohandle.cpp ./src/cpu.cpp

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS0 = -Wall

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS0 = -pthread -lSDL2main -lSDL2 -lSDL2_image

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME0 = chip8

#This is the target that compiles our executable
all :
	$(CC) $(OBJS0) $(COMPILER_FLAGS0) $(LINKER_FLAGS0) -o $(OBJ_NAME0)