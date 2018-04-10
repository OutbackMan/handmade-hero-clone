#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOCAL_PERSIST static
#define INTERNAL static

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAX_CONTROLLERS 4

#define kHz 000
#define PI32 3.14159265359f
// We are building on a 64-bit system
typedef float real32;
typedef double real64;

// Use SDL_QueueAudio() to avoid using a callback

INTERNAL void init_default_sdl_audio_device(uint32_t samples_per_second, uint32_t buffer_size, SDL_AudioDeviceID* audio_device_id)
{
	SDL_AudioSpec desired_audio_settings = {0};

	desired_audio_settings.freq = samples_per_second;
	desired_audio_settings.format = AUDIO_S16LSB;
	desired_audio_settings.channels = 2;
	desired_audio_settings.samples = buffer_size;
	desired_audio_settings.callback = NULL;

	SDL_AudioSpec actual_audio_settings = {0};
	*audio_device_id = SDL_OpenAudioDevice(NULL, 0, &desired_audio_settings, &actual_audio_settings, 0);

	if (actual_audio_settings.format != desired_audio_settings.format) {
		SDL_CloseAudioDevice(audio_device_id);	
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
	

	SDL_AudioDeviceID audio_device_id = {0};
	int sampling_rate = 44100; // 44.1kHz .Niquist requires double of max frequency
	size_t bytes_per_sample = sizeof(uint16_t) * 2; // 2 channels
	int sampling_buffer_size = sampling_rate * bytes_per_sample / 60; // assume 60fps. no 'correct' value
	init_default_audio_device(sampling_rate, sampling_buffer_size, &audio_device_id);

	int audio_frequency = 256; // low pitch (bass), high (treble)
	uint16_t audio_amplitude = 3000; 
	uint32_t running_sample_index = 0;

	while (is_running) {
		int latency_sampling_count = sampling_rate / 15;  // investigate this latency
		int desired_audio_bytes_to_queue = latency_sampling_count * bytes_per_sample;
		int actual_audio_bytes_to_queue = desired_audio_bytes_to_queue - SDL_GetQueuedAudioSize(audio_device_id);
		uint16_t* audio_buffer = malloc(actual_audio_bytes_to_queue);
		int sample_count = actual_bytes_to_write / bytes_per_sample; 
		for (int sample_index = 0; sample_index < sample_count; ++sample_index) {
			*audio_buffer++ = // refer to sound generator
			*audio_buffer++ = // refer to sound generator	
		}	
		SDL_QueueAudio(audio_device_id, audio_buffer, actual_audio_bytes_to_write);
		free(audio_buffer);

		if (SDL_GetAudioDeviceStatus(audio_device_id) != SDL_AUDIO_PLAYING) {
			SDL_PauseAudioDevice(audio_device_id, 0); // play
		}

		SDL_CloseAudioDevice(audio_device_id);
	}

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
