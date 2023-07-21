#ifndef CHIP8_SCREEN
#define CHIP8_SCREEN
#include <stdio.h>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

#define PIXEL_SIZE 10
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH_SCALED (SCREEN_WIDTH * PIXEL_SIZE)
#define SCREEN_HEIGHT_SCALED (SCREEN_HEIGHT * PIXEL_SIZE)

typedef unsigned char BYTE;
typedef unsigned short int WORD;

class chip8_screen {
    private:
    SDL_Window* window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        SCREEN_WIDTH_SCALED, SCREEN_HEIGHT_SCALED, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    BYTE m_ScreenData[SCREEN_WIDTH][SCREEN_HEIGHT];

    public:
    chip8_screen(){
        clear();
    }

    void free(){
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

void drawPixels() {
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            if (m_ScreenData[x][y] == 1) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
                SDL_Rect pixelRect = { x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
                SDL_RenderFillRect(renderer, &pixelRect);
        }
    }

    SDL_RenderPresent(renderer);
}


    void setPixel(int x, int y, int value){
        m_ScreenData[x][y] = value;
    }

    int getPixel(int x, int y){
        return m_ScreenData[x][y];
    }

    void clear(){
        memset(m_ScreenData, 0, sizeof(m_ScreenData));
    }

};

#endif