#include <stdio.h>
#include <iostream>  // For basic I/O operations (optional but often used)
#include <vector>    // Include the vector header
#include <cstring> // Include this header for using memset
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
    while (!quit) {
        keyboard.poll_status(quit);
        cpu.run_program();
       
        SDL_Delay(2.5);
    }

    window.free();
    return 0;
}
