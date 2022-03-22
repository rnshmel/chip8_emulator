#ifndef DISPLAY_H
#define DISPLAY_H

//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <vector>

// init the SDL screen and variables
bool SDL_screen_init(int &xval);

// close/destroy the SDL program
void SDL_screen_close();

// clear screen
int clear_screen();

// draws a vector to the screen
int draw_screen_vector(std::vector<std::vector<unsigned char>> screen_vec);

// input handler for SDL-based events
int SDL_input_event_handler(bool &exit_event, std::vector<unsigned char> &key_vector, int &kflag);

#endif