#ifndef CHIP8_SCREEN
#define CHIP8_SCREEN
#include <stdio.h>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define PIXEL_SIZE 10

typedef unsigned char BYTE;
typedef unsigned short int WORD;

class chip8_screen {
    private:
    SDL_Window* window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    BYTE m_ScreenData[64][32];

    public:
    chip8_screen(){
        clear();
    }

    void free(){
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void drawPixels() {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set the pixel color to white
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set the background color to black


        for (int x = 0; x < 64; ++x) {
            for (int y = 0; y < 32; ++y) {
                if (m_ScreenData[x][y] == 255) {
                    SDL_Rect pixelRect = { x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
                    SDL_RenderFillRect(renderer, &pixelRect);
                }
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