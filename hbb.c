#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <curl/curl.h>
#define MAX_INPUT 256

struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	
	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if(ptr == NULL) {
		printf("Runtime error fetching content: out of memory\n");
		return 0;
	}
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	
	return realsize;
}
void fetchURL(const char *geturl) {
	CURL *curl_handle;
	CURLcode res;
	
	struct MemoryStruct chunk;
	
	chunk.memory = malloc(1);
	chunk.size = 0;
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	
	if(curl_handle) {
		curl_easy_setopt(curl_handle, CURLOPT_URL, geturl);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "HomeBrewBrowser/I0.0.1"); // probably make this a little better
		res = curl_easy_perform(curl_handle);
		
		if(res != CURLE_OK) {
			fprintf(stderr, "Fetch failed: %s\n", curl_easy_strerror(res));
		} else {
			printf("%lu bytes retrieved from %s\n", (unsigned long)chunk.size, geturl);
			printf("Data: %s\n", chunk.memory);
		}
		
		curl_easy_cleanup(curl_handle);
		free(chunk.memory);
	}
}
int main() {
	printf("Launching HBB...\n");
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Launch error at SDL Video init: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *win = SDL_CreateWindow("Homebrew Browser", 100, 100, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!win) {
		printf("Launch error at window creation: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (TTF_Init() == -1) {
	    printf("Launch error at SDL TTF init %s\n", TTF_GetError());
	    SDL_DestroyWindow(win);
	    SDL_Quit();
	    return 1;
	}
	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
	if (!font) {
		printf("Launch error creating font: %s\n", TTF_GetError());
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}
	char urlin[MAX_INPUT] = "";
	char url[MAX_INPUT] = "";
	SDL_StartTextInput();
	SDL_Event main;
	int running = 1;

	while (running) {
		SDL_RenderClear(ren);
		while (SDL_PollEvent(&main)) {
			if (main.type == SDL_QUIT) {
				running = 0;
				break;
			}
			if (main.type == SDL_TEXTINPUT) {
				if (strlen(urlin) + strlen(main.text.text) < MAX_INPUT - 1) {
					strcat(urlin, main.text.text);
				}
				break;
			}
			if (main.type == SDL_KEYDOWN) {
				if (main.key.keysym.sym == SDLK_BACKSPACE && strlen(urlin) > 0) {
					urlin[strlen(urlin) - 1] = '\0';
				}
				else if (main.key.keysym.sym == SDLK_RETURN) {
					strncpy(url, urlin, MAX_INPUT);
					urlin[0] = '\0';
					printf("Submitted URL %s\n", url);
					fetchURL(url);
				}
				break;
			}
		}
		SDL_Color Black = {0, 0, 0};
		SDL_Color Grey1 = {128, 128, 128};
		if (strlen(urlin) > 0) {
			SDL_Surface* urlinput = TTF_RenderText_Blended(font, urlin, Black);
			if (urlinput) {
				SDL_Texture* urlintex = SDL_CreateTextureFromSurface(ren, urlinput);
				SDL_Rect dst = {10, 10, urlinput->w, urlinput->h};
				SDL_RenderCopy(ren, urlintex, NULL, &dst);
				SDL_FreeSurface(urlinput);
				SDL_DestroyTexture(urlintex);
			}
		} else {
			SDL_Surface* urldisplay = TTF_RenderText_Blended(font, url, Grey1);
			if (urldisplay) {
				SDL_Texture* urldistex = SDL_CreateTextureFromSurface(ren, urldisplay);
				SDL_Rect dst = {10, 10, urldisplay->w, urldisplay->h};
				SDL_RenderCopy(ren, urldistex, NULL, &dst);
				SDL_FreeSurface(urldisplay);
				SDL_DestroyTexture(urldistex);
			}
		}
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderPresent(ren);
		SDL_Delay(16);
	}

	SDL_DestroyWindow(win);
	TTF_Quit();
	SDL_Quit();

	return 0;
}
