#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOCAL_PERSIST static
#define INTERNAL static

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAX_CONTROLLERS 4

// Use SDL_QueueAudio() to avoid using a callback

INTERNAL void init_sdl_audio(uint32_t samples_per_second, uint32_t buffer_size)
{
	SDL_AudioSpec audio_settings = {0};

	audio_settings.freq = samples_per_second;
	audio_settings.format = AUDIO_S16LSB;
	audio_settings.channels = 2;
	audio_settings.samples = buffer_size / 2;

	SDL_OpenAudio(&audio_settings, 0);

	if (audio_settings.format != AUDIO_S16LSB) {
		SDL_CloseAudio();		
	}
}

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
		// (unsigned char)("abcdefg"[loop_counter])

		bool alt_key_was_down = (event->key.keysym.mod & KMOD_ALT);
		if (keycode == SDLK_F4 && alt_key_was_down) {
			// ...		
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

INTERNAL void open_sdl_game_controllers(size_t max_controllers, SDL_GameController controllers[max_controllers], SDL_Haptic rumbles[max_controllers])
{
	int num_joysticks = SDL_NumJoysticks();
	int controller_index = 0;

	for (int joystick_index = 0; joystick_index < num_joysticks; ++joystick_index) {
		if (!SDL_IsGameController(joystick_index)) {
			continue;		
		}		
		if (controller_index >= MAX_CONTROLLERS) {
			break;	
		}	

		controllers[controller_index] = SDL_GameControllerOpen(joystick_index);
		rumbles[controller_index] = SDL_HapticOpen(joystick_index);

		if (rumbles[controller_index] != NULL && SDL_HapticRumbleInit(rumbles[controller_index]) != 0) {
			SDL_HapticClose(rumbles[controller_index]);		
			rumbles[controller_index] = 0;
		}

		++controller_index;
	}
}

INTERNAL void close_sdl_game_controllers(size_t max_controllers, SDL_GameController controllers[max_controllers], SDL_Haptic rumbles[max_controllers])
{
	for (int controller_index = 0; controller_index < max_controllers; ++controller_index) {
		if (controllers[controller_index] && rumbles[controller_index]) {
			SDL_HapticClose(rumbles[controller_index]);		
			SDL_GameControllerClose(controllers[controller_index]);
		}
	}
}

int main(int argc, char* argv[argc + 1])
{
	bool is_running = false;
	/*
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Handmade Clone", "Hello there", NULL) < 0) {
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
	}
	*/

	// Can initalise subsystems later: SDL_InitSubsystem() 
	// Can check for initialisation with: !SDL_WasInit()
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		// TODO(Ryan) Custom logging and error handling
		fprintf(stderr, "Error encountered: %s\n", SDL_GetError());	
		return EXIT_FAILURE;
	}

	SDL_GameController* controller_handles[MAX_CONTROLLERS] = {0};
	SDL_Haptic* rumble_handles[MAX_CONTROLLERS] = {0};
	open_sdl_controllers();

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
	
	int samples_per_second = 48000; // 44.1kHz roughly gives a maximum frequency of 20kHz, so generally sufficient. (Niquist requires double of max)
	// accuracy of the frequency

	int tone_hz = 256; // named thusly to avoid ambiguity with sampling frequency. low pitch (bass), high (treble)
	// adding various sine waves can create any sound
	// this will play 256 48kHz samples on each polarity, giving 512 samples every full wave 

	uint16_t tone_volume = 3000; 
	uint32_t running_sample_index = 0;
	int square_wave_period = samples_per_second / tone_hz; // how many samples per repeating unit
	int half_square_wave_period = square_wave_period / 2;  // number of samples wave is high or low
	int bytes_per_sample = sizeof(uint16_t) * 2; // bytes per channel
	open_sdl_audio(48000, samples_per_second * bytes_per_sample / 60);
	bool sound_is_playing = false;

	while (is_running) {
		SDL_Event event = {0};
		while (SDL_PollEvent(&event)) {
			if (handle_events_recieve_quit(&event)) {
				is_running = false;		
			}
		}

		// To handle controllers added halfway: SDL_CONTROLLERDEVICEADDED
		for (int controller_index = 0; controller_index < max_controllers; ++controller_index) {
			if (controllers[controller_index] != 0 && SDL_GameControllerGetAttached(controllers[controller_index])) {
				bool up = SDL_GameControllerGetButton(controllers[controller_index], SDL_CONTROLLER_BUTTON_DPAD_UP);		
				uint16_t stick_x = SDL_GameControllerGetButton(controllers[controller_index], SDL_CONTROLLER_AXIS_LEFTX);	

				if (up) {
					if (rumbles[controller_index]) {
						SDL_HapticRumblePlay(rumbles[controller_index], 0.5f, 2000);
					}		
				}
			}		
		} 

		// may get latency as some times runs < 800 samples, so have no audio data
		int target_queue_bytes = samples_per_second * bytes_per_sample;
		int bytes_to_write = target_queue_bytes - SDL_GetQueuedAudioSize(1); 
		if (bytes_to_write) {
			uint16_t sample_out = malloc(bytes_to_write);
			int sample_count = bytes_to_write / bytes_per_sample;
			for (int sample_index = 0; sample_index < sample_count; ++sample_index) {
				uint16_t sample_value = ((running_sample_index++ / half_square_wave_period % 2) ? tone_volume: -tone_volume;
				*sample_out++ = sample_value;
				*sample_out++ = sample_value;
			}
			SDL_QueueAudio(1, sound_buffer, bytes_to_write);
			free(sound_buffer);

			if (!sound_is_playing) {
				SDL_PauseAudio(0);
				sound_is_playing = true;
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

INTERNAL void sound_generator(void)
{
	double 12th_root_of_2 = pow(2.0, 1.0 / 12.0);

	double frequency = 110 * pow(12th_root_of_2, 5); // 5 semi-tones

	double output = amplitude * sin(frequency * 2 * 3.14 * time); // add another sin() with offset frequency to create a chord

	return (output > 0 ? 0.3 : -0.3);
}

// #ifdef (_MSC_VER)
// #define RDTSC __rdtsc()
// #else
// #define RDTSC _rdtsc()
