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

void FixedRenderString(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color, int x, int y, int maxWidth, int maxHeight) {
	char *saveptr_line;
	char *copy = strdup(text);
	char *line = strtok_r(copy, "\n", &saveptr_line);
	int lineHeight = TTF_FontHeight(font);
	while (line) {
		char currentLine[2048] = "";
		char tempLine[1024];
		char *saveptr_word;
		char *word = strtok_r(line, " ", &saveptr_word);
		while (word) {
			if (y + lineHeight > maxHeight) {
				return;
			}
			snprintf(tempLine, sizeof(tempLine), "%s %s", currentLine, word);
			int w = 0;
			TTF_SizeText(font, tempLine, &w, NULL);
			if (w > maxWidth) {
				SDL_Surface* surface = TTF_RenderText_Blended(font, currentLine, color);
				SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
				SDL_Rect dst = {x, y, surface->w, surface->h};
				SDL_RenderCopy(renderer, texture, NULL, &dst);
				SDL_FreeSurface(surface);
				SDL_DestroyTexture(texture);
				y += lineHeight;
				snprintf(currentLine, sizeof(currentLine), "%s", tempLine);
			} else {
				snprintf(currentLine, sizeof(currentLine), "%s", tempLine);
			}
			word = strtok_r(NULL, " ", &saveptr_word);
		}
		if (strlen(currentLine) > 0) {
			if (y + lineHeight > maxHeight) {
				return;
			}
			SDL_Surface* surface = TTF_RenderText_Blended(font, currentLine, color);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_Rect dst = {x, y, surface->w, surface->h};
			SDL_RenderCopy(renderer, texture, NULL, &dst);
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
			y += lineHeight;
		}
		line = strtok_r(NULL, "\n", &saveptr_line);
	}
	free(copy);
}

char *fetchURL(const char *geturl) {
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
		return chunk.memory;
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
	char *data = NULL;
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
					if (data) {
					    free(data);
					    data = NULL;
					}
					data = fetchURL(url);
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
			if (data) {
				int winW, winH;
				SDL_GetRendererOutputSize(ren, &winW, &winH);
				FixedRenderString(ren, font, data, Black, 10, 60, winW, winH);
			}
		}
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderPresent(ren);
		SDL_Delay(16);
	}

	SDL_DestroyWindow(win);
	TTF_Quit();
	SDL_Quit();
	if (data) {
		free(data);
		data = NULL;
	}

	return 0;
}
