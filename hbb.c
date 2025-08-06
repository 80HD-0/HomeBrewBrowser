#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#define MAX_INPUT 256

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
	SDL_Quit();

	return 0;
}
