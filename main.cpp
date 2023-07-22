#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <SDL2/SDL.h>

#include "chip8_cpu.h"
#include "chip8_screen.h"
#include "chip8_keyboard.h"

int main(int argc, char* argv[])
{
    std::string executable = argv[1];

    chip8_screen window;
    chip8_keyboard keyboard;
    chip8_cpu cpu(window, keyboard, executable);

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Failed to initialize the SDL2 library\n";
        return -1;
    }

    bool quit = false;
    const int DESIRED_FRAME_TIME = 1000 / 480;

    std::chrono::high_resolution_clock::time_point lastFrameTime = std::chrono::high_resolution_clock::now();
    while (!quit) {
        keyboard.poll_status(quit);
        cpu.run_program();
        window.drawPixels();

        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime).count();

        if (elapsedTime < DESIRED_FRAME_TIME) {
            SDL_Delay(DESIRED_FRAME_TIME - elapsedTime);
        }

        lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    window.free();
    return 0;
}
