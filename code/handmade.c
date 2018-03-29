#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOCAL_PERSIST static
#define INTERNAL static

INTERNAL void resize_sdl_texture(SDL_Texture* texture, SDL_Renderer* renderer, int width, int height)
{
	if (texture != NULL) {
		SDL_DestroyTexture(texture);	
	}	

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
					width, height);
}

INTERNAL void update_sdl_window(SDL_Window* window, SDL_Texture* window_texture)
{
	int window_width = 0;
	int window_height = 0;

	SDL_GetWindowSize(window, &window_width, &window_height);

	SDL_Renderer* renderer = SDL_GetRenderer(window);

	void* texture_pixels = NULL;
	int texture_pitch = 0; // bytes per row
	if (SDL_LockTexture(window_texture, NULL, &texture_pitch, &texture_width) < 0) {
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return;
	}

	memset(texture_pixels, 0, texture_pitch * window_height);

	SDL_UnlockTexture(window_texture);

	SDL_RenderCopy(renderer, window_texture, NULL, NULL); 
	SDL_RenderPresent(renderer);
}

INTERNAL bool handle_events_recieve_quit(SDL_Event* event)
{
	bool quit_recieved = false;

	switch (event->type) {
	 case SDL_QUIT:
	 {
		quit_recieved = true;
		break;
	 }
	 case SDL_WINDOWEVENT:
	 {
		switch (event->window.event) {
		 case SDL_WINDOWEVENT_RESIZED:
		 {
			printf("Resized window: (%d %d)\n", event->window.data1, event->window.data2);
			resize_sdl_texture(texture, event->window.data1, event->window.data2);
			break;
		 }
		 case SDL_WINDOWEVENT_FOCUS_GAINED:
		 {
			puts("Focused on window");
			break;
		 }
		 case SDL_WINDOWEVENT_EXPOSED:
		 {
			SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);

			update_sdl_window(window);

			break;
		 }
		break;
		}
	 }
	 default:
	 {
		break;
	 }
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

	// TODO(Ryan) Add gcc cleanup (we want to handle cleanups in waves, i.e. aggregates)
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
