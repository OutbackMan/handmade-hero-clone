#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static bool handle_events_recieve_quit(SDL_Event* event)
{
	bool quit_recieved = false;

	switch (event->type) {
	case SDL_QUIT:
		quit_recieved = true;
		break;
	case SDL_WINDOWEVENT:
		switch (event->window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			printf("Resized window: (%d %d)\n", event.window.data1, event.window.data2);
			break;
		}
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			puts("Focused on window");
			break;
		}
		case SDL_WINDOWEVENT_EXPOSED:
			SDL_Window* window = SDL_GetWindowFromID(event.windowID);
			SDL_Renderer* renderer = SDL_GetRenderer(window);
			static bool is_white = false;
			if (is_white) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);	
			} else {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);	
			}
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
			break;
		}
		break;
	default:
		break;
	}		

	return quit_recieved;
}

int main(int argc, char* argv[argc + 1])
{
	/*
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Handmade Clone", "Hello there", NULL) < 0) {
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
	}
	*/

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		// TODO(Ryan) Custom logging and error handling
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED, 
							SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		// TODO(Ryan) Custom logging and error handling
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return EXIT_FAILURE;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	// TODO(Ryan) Add gcc cleanup
	if (renderer == NULL) {
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return EXIT_FAILURE;
	}

	while (true) {
		SDL_Event event = {0};
		SDL_WaitEvent(&event);
		if (handle_events_recieve_quit(&event)) {
			break;		
		}
	}

	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;	
}
