//Using SDL, SDL_image, standard IO, math, and strings
#include "iohandle.h"

//Screen dimension constants
int S_SCALE = 10;
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 320;

// The window we'll be rendering to
SDL_Window* gWindow = NULL;

// The window renderer
SDL_Renderer* gRenderer = NULL;

// SDL event variable
SDL_Event evnt;

// key state pointer
const Uint8* cKeyStates;

// init the SDL screen and variables
bool SDL_screen_init(int &xval)
{
	//Initialization flag
	bool success = true;

	// pixel size
	S_SCALE = xval;
	SCREEN_WIDTH = 64 * S_SCALE;
	SCREEN_HEIGHT = 32 * S_SCALE;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "CHIP 8 EMU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

// close/destroy the SDL program
void SDL_screen_close()
{
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

// clears the screen
int clear_screen()
{
	// set render color
	// r,g,b,a
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(gRenderer);
	// update the screen
	SDL_RenderPresent(gRenderer);
	return 0;
}

// draws a vector to the screen 
int draw_screen_vector(std::vector<std::vector<unsigned char>> screen_vec)
{
	// make a fill struct to hold pixel data
	// {x, y, l, w}
	SDL_Rect pixel;
	// set render color
	// r,g,b,a
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
    for(int i = 0; i < (int)screen_vec.size(); i++)
    {
        for(int j = 0; j < (int)screen_vec.at(i).size(); j++)
        {
            if (screen_vec.at(i).at(j) == 1)
            {
                // on pixel
				// set render color
				// r,g,b,a
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				pixel = {j*S_SCALE, i*S_SCALE, S_SCALE, S_SCALE};
            }
            else
            {
                // off pixel
				// set render color
				// r,g,b,a
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF );
				pixel = {j*S_SCALE, i*S_SCALE, S_SCALE, S_SCALE};
            }
			SDL_RenderFillRect(gRenderer, &pixel);
        }
    }
	// update the screen
	SDL_RenderPresent(gRenderer);
	return 0;
}

// gets input from SDL events
int SDL_input_event_handler(bool &exit_event, std::vector<unsigned char> &key_vector, int &kflag)
{
	// poll event
	SDL_PollEvent(&evnt);
	if (evnt.type == SDL_QUIT)
	{
		exit_event = true;
	}
	// current key states are updated every event poll, grab current instance
	cKeyStates = SDL_GetKeyboardState(NULL);
	// set keys in key vector according to SDL scan code
	// if kflag == 1, use special tetris keybindings
	if (kflag == 1)
	{
		key_vector.at(4) = cKeyStates[SDL_SCANCODE_SPACE];
		key_vector.at(5) = cKeyStates[SDL_SCANCODE_LEFT];
		key_vector.at(6) = cKeyStates[SDL_SCANCODE_RIGHT];
		key_vector.at(7) = cKeyStates[SDL_SCANCODE_DOWN];
	}
	else
	{
		key_vector.at(0) = cKeyStates[SDL_SCANCODE_0];
		key_vector.at(1) = cKeyStates[SDL_SCANCODE_1];
		key_vector.at(2) = cKeyStates[SDL_SCANCODE_2];
		key_vector.at(3) = cKeyStates[SDL_SCANCODE_3];
		key_vector.at(4) = cKeyStates[SDL_SCANCODE_4];
		key_vector.at(5) = cKeyStates[SDL_SCANCODE_5];
		key_vector.at(6) = cKeyStates[SDL_SCANCODE_6];
		key_vector.at(7) = cKeyStates[SDL_SCANCODE_7];
		key_vector.at(8) = cKeyStates[SDL_SCANCODE_8];
		key_vector.at(9) = cKeyStates[SDL_SCANCODE_9];
		key_vector.at(10) = cKeyStates[SDL_SCANCODE_A];
		key_vector.at(11) = cKeyStates[SDL_SCANCODE_B];
		key_vector.at(12) = cKeyStates[SDL_SCANCODE_C];
		key_vector.at(13) = cKeyStates[SDL_SCANCODE_D];
		key_vector.at(14) = cKeyStates[SDL_SCANCODE_E];
		key_vector.at(15) = cKeyStates[SDL_SCANCODE_F];
	}
	// return
	return 0;
}