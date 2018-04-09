#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOCAL_PERSIST static
#define INTERNAL static

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAX_CONTROLLERS 4

INTERNAL void texture_set_colour(SDL_Texture* texture, SDL_Color* colour)
{
	int texture_width = 0;
	int texture_height = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);

	void* texture_pixels = NULL;	
	int texture_pitch = 0;

	uint32_t* texture_pixel = NULL;

	if (SDL_LockTexture(texture, NULL, &texture_pitch, &texture_width) < 0) {
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return;
	}

	for (int row = 0; row < texture_width; ++row) {
		texture_pixel = (uint32_t *)((uint8_t *)(texture_pixels) + row * texture_pitch)
		for (int col = 0; col < texture_height; ++col) {
			*texture_pixel++ = (0xFF000000 | (colour->r << 16) | (colour->g << 8) | (colour->b));
		}
	}	
	
	SDL_UnlockTexture(texture);	
}

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

	// Obtain desired colour from somewhere
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
	 case SDL_KEYDOWN:
	 case SDL_KEYUP:
	 {
		SDL_Keycode keycode = event->key.keysym.sym;
		bool key_is_down = (event->key.state == SDL_PRESSED);
		bool key_was_down = false;
		if (event->key.state == SDL_RELEASED || event->key.repeat != 0) {
			key_was_down = true;		
		}

		if (event->key.repeat == 0) {
			if (keycode == SDLK_w) {
				printf("w %s\n", (key_is_down ? "is down" : "was down"));
			} else if (keycode == SDLK_a) {
				
			}
		}
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

INTERNAL void open_sdl_game_controllers(void)
{
		
}

int main(int argc, char* argv[argc + 1])
{
	SDL_GameController* controller_handles[MAX_CONTROLLERS] = {0};
	SDL_Haptic* rumble_handles[MAX_CONTROLLERS] = {0};

	bool is_running = false;
	/*
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Handmade Clone", "Hello there", NULL) < 0) {
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
	}
	*/

	// Can initalise subsystems later: SDL_InitSubsystem() 
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		// TODO(Ryan) Custom logging and error handling
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED, 
							SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		// TODO(Ryan) Custom logging and error handling
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return EXIT_FAILURE;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	if (renderer == NULL) {
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return EXIT_FAILURE;
	}

	is_running = true;
	SDL_Texture bg_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
					width, height);
	
	while (is_running) {
		SDL_Event event = {0};
		while (SDL_PollEvent(&event)) {
			if (handle_events_recieve_quit(&event)) {
				is_running = false;		
			}
		}
		texture_colour(texture, (&SDL_Color){10, 10, 10});
		update_sdl_window(window);
	}
	
	close_game_controllers();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;	
}
