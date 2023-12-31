#ifndef CHIP8_CPU
#define CHIP8_CPU
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
#include <fstream>

#include "chip8_screen.h"
#include "chip8_keyboard.h"

#define GAME_MEMORY_START_LOCATION 0x200
typedef unsigned char BYTE;
typedef unsigned short int WORD;

class chip8_cpu {
    private:
        BYTE m_GameMemory[0xFFF]; // 0xFFF bytes of memory, 4095 bytes
        BYTE m_Registers[16]; // 16 registers, 1 byte each
        WORD m_AddressI; // the 16-bit address register I
        WORD m_ProgramCounter; // the 16-bit program counter
        std::vector<WORD> m_Stack; // the 16-bit stack
        uint8_t delayTimer;
        chip8_screen& m_Window;
        chip8_keyboard& m_Keyboard;
        std::string m_Executable;
        WORD GetNextOpcode()
        {
            WORD res = 0;
            res = m_GameMemory[m_ProgramCounter]; // in example res is 0xAB
            res <<= 8; // shift 8 bits left. In our example res is 0xAB00
            res |= m_GameMemory[m_ProgramCounter+1];
            m_ProgramCounter+=2;
            return res;
        }

        int save_state(){
            std::ofstream outputFile("state.emu", std::ios::binary);
            if (!outputFile.is_open()) {
                std::cout << "Error opening the file." << std::endl;
                return 1;
            }

            outputFile.write(reinterpret_cast<const char*>(m_GameMemory), sizeof(m_GameMemory));
            outputFile.write(reinterpret_cast<const char*>(m_Registers), sizeof(m_Registers));
            outputFile.write(reinterpret_cast<const char*>(&m_AddressI), sizeof(m_AddressI));
            outputFile.write(reinterpret_cast<const char*>(&m_ProgramCounter), sizeof(m_ProgramCounter));

            int vectorSize = m_Stack.size();
            outputFile.write(reinterpret_cast<const char*>(&vectorSize), sizeof(vectorSize));
            for(int i = 0; i < vectorSize; i++){
                outputFile.write(reinterpret_cast<const char*>(&(m_Stack[i])), sizeof(m_Stack[i]));
            }

            outputFile.write(reinterpret_cast<const char*>(&delayTimer), sizeof(delayTimer));
            outputFile.write(reinterpret_cast<const char*>(m_Window.getScreenData()), sizeof(m_Window.getScreenData()));

            outputFile.close();
            std::cout << "Saving done" << std::endl;
            return 0;
        }

        int load_state(){
            std::ifstream inputFile("state.emu", std::ios::binary);

            if (!inputFile.is_open()) {
                std::cout << "Error opening the file." << std::endl;
                return 1;
            }
            inputFile.read(reinterpret_cast<char*>(m_GameMemory), sizeof(m_GameMemory));
            inputFile.read(reinterpret_cast<char*>(m_Registers), sizeof(m_Registers));
            inputFile.read(reinterpret_cast<char*>(&m_AddressI), sizeof(m_AddressI));
            inputFile.read(reinterpret_cast<char*>(&m_ProgramCounter), sizeof(m_ProgramCounter));
            int vectorSize;
            inputFile.read(reinterpret_cast<char*>(&vectorSize), sizeof(vectorSize));
            for(int i = 0; i < vectorSize; i++){
                inputFile.read(reinterpret_cast<char*>(&(m_Stack[i])), sizeof(m_Stack[i]));
            }
            inputFile.read(reinterpret_cast<char*>(&delayTimer), sizeof(delayTimer));
            inputFile.read(reinterpret_cast<char*>(m_Window.getScreenData()), sizeof(m_Window.getScreenData()));
            m_Window.drawPixels();
            std::cout << "Loading done" << std::endl;
            return 0;
        }

        void check_for_state_action() {
            int keypressed = m_Keyboard.get_key_pressed();

            // Check if keypressed is either 16 (save key) or 17 (load key)
            if (keypressed == 16 || keypressed == 17) {
                // Wait for the key release before proceeding
                while (m_Keyboard.get_key_value(keypressed) != 0) {
                    bool quit = false;
                    m_Keyboard.poll_status(quit);
                }

                if (keypressed == 16) {
                    save_state();
                } else {
                    load_state();
                }
            }
        }

    public: 
    
        chip8_cpu (chip8_screen& window, chip8_keyboard& keyboard, const std::string& executable) : m_Window(window), m_Keyboard(keyboard), m_Executable(executable) {
            CPUReset();
        }

        void CPUReset()
        {
            m_AddressI = 0;
            m_ProgramCounter = GAME_MEMORY_START_LOCATION;
            delayTimer = 0;
            memset(m_Registers, 0, sizeof(m_Registers));
            m_Window.clear();

            // load in the game
            FILE *in;
            in = fopen(m_Executable.c_str(), "rb");
            fread( &m_GameMemory[GAME_MEMORY_START_LOCATION], 0xfff, 1, in);
            fclose(in);
        }

        void run_program(){
            check_for_state_action();
            if (delayTimer > 0) {
                --delayTimer;
            }

            int opcode = GetNextOpcode();
            //std::cout << "instruction " << std::hex << opcode << std::endl;
            switch (opcode & 0xF000)
            {   
                case 0x1000:
                {
                    m_ProgramCounter = opcode & 0x0FFF;
                    break;
                } 
                case 0x0000:
                {
                    switch(opcode & 0x000F)
                    {
                        case 0x0000:{
                            m_Window.clear();
                            break;
                        } 
                        case 0x000E: {
                            m_ProgramCounter = m_Stack.back();
                            m_Stack.pop_back();
                            break;
                        }
                    }
                    break;
                }
                case 0x2000:
                {
                    m_Stack.push_back(m_ProgramCounter);
                    m_ProgramCounter = opcode & 0x0FFF;
                    break;
                }
                case 0x3000:
                {
                    WORD register_index = (opcode & 0x0F00) >> 8;
                    WORD constant = opcode & 0x00FF;
                    if(m_Registers[register_index] == constant){
                        m_ProgramCounter+=2;
                    }
                    break;
                }
                case 0x4000:
                {
                    WORD register_index = (opcode & 0x0F00) >> 8;
                    WORD constant = opcode & 0x00FF;
                    if(m_Registers[register_index] != constant){
                        m_ProgramCounter+=2;
                    }
                    break;
                }
                case 0x5000:
                {
                    WORD registerX_index = (opcode & 0x0F00) >> 8;
                    WORD registerY_index = (opcode & 0x00F0) >> 4;
                    if(m_Registers[registerX_index] == m_Registers[registerY_index]){
                        m_ProgramCounter+=2;
                    }
                    break;
                }
                case 0x6000:
                {
                    WORD register_index = (opcode & 0x0F00) >> 8;
                    WORD constant = opcode & 0x00FF;
                    m_Registers[register_index] = constant;
                    break;
                }
                case 0x7000:
                {
                    WORD register_index = (opcode & 0x0F00) >> 8;
                    WORD constant = opcode & 0x00FF;
                    m_Registers[register_index] += constant;
                    break;
                }
                case 0x8000:
                {
                    switch(opcode & 0x000F){
                        case 0x0000:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;
                            m_Registers[registerX_index] = m_Registers[registerY_index];
                            break;
                        }
                        case 0x0001:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;
                            m_Registers[registerX_index] |= m_Registers[registerY_index];
                            m_Registers[0xF] = 0;
                            break;
                        }
                        case 0x0002:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;
                            m_Registers[registerX_index] &= m_Registers[registerY_index];
                            m_Registers[0xF] = 0;
                            break;
                        }
                        case 0x0003:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;
                            m_Registers[registerX_index] ^= m_Registers[registerY_index];
                            m_Registers[0xF] = 0;
                            break;
                        }
                        case 0x0004:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;
                            m_Registers[0xF] = 0;
                            int value = m_Registers[registerX_index] + m_Registers[registerY_index];
                            if(value > 255){
                                m_Registers[0xF] = 1;
                            }
                            
                            m_Registers[registerX_index] += m_Registers[registerY_index];
                            break;
                        }
                        case 0x0005:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;
                            m_Registers[0xF] = 1;
                            
                            int value = m_Registers[registerX_index] - m_Registers[registerY_index];
                            if(value < 0){
                                m_Registers[0xF] = 0;
                            }
                            
                            m_Registers[registerX_index] -= m_Registers[registerY_index];
                            break;
                        }
                        case 0x0006:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;

                            m_Registers[registerX_index] = m_Registers[registerY_index];

                            m_Registers[0xF] = m_Registers[registerX_index] & 0x1;
                            m_Registers[registerX_index] >>= 1;
                            
                            break;
                        }
                        case 0x0007:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;
                            m_Registers[0xF] = 1;

                            int value = m_Registers[registerY_index] - m_Registers[registerX_index];
                            if(value < 0)
                                m_Registers[0xF] = 0;

                            m_Registers[registerX_index] = value;
                            break;
                        }
                        case 0x000E:
                        {
                            WORD registerX_index = (opcode & 0x0F00) >> 8;
                            WORD registerY_index = (opcode & 0x00F0) >> 4;

                            m_Registers[registerX_index] = m_Registers[registerY_index];

                            m_Registers[0xF] = m_Registers[registerX_index] >> 7;
                            m_Registers[registerX_index] <<= 1;
                            break;
                        }
                    }
                    break;
                }
                case 0x9000:
                {
                    WORD registerX_index = (opcode & 0x0F00) >> 8;
                    WORD registerY_index = (opcode & 0x00F0) >> 4;
                    if(m_Registers[registerX_index] != m_Registers[registerY_index]){
                        m_ProgramCounter+=2;
                    }
                    break;
                }
                case 0xA000:
                {
                    m_AddressI = opcode & 0x0FFF;
                    break;
                }
                case 0xB000:
                {
                    m_ProgramCounter = m_Registers[0] + (opcode & 0x0FFF);
                    break;
                }
                case 0xC000:
                {
                    int regx = (opcode & 0x0F00) >> 8;
                    m_Registers[regx] = rand() & (opcode & 0x00FF);
                    break;
                }
                case 0xD000:
                {
                    int regx = (opcode & 0x0F00) >> 8;
                    int regy = (opcode & 0x00F0) >> 4;

                    int x_cord = m_Registers[regx] & 63;
                    int y_cord = m_Registers[regy] & 31;
                    int height = opcode & 0x000F;

                    m_Registers[0xF] = 0;

                    for (int row = 0; row < height; row++) {
                        BYTE data = m_GameMemory[m_AddressI + row];

                        int xpixelinv = 7;
                        for (int xpixel = 0; xpixel < 8; xpixel++, xpixelinv--) {
                            int mask = 1 << xpixelinv;
                            if (data & mask) {
                                int x = (x_cord + xpixel) % SCREEN_WIDTH; // Wrap around the screen horizontally
                                int y = (y_cord + row) % SCREEN_HEIGHT;   // Wrap around the screen vertically
                                int color = 1;
                                if (m_Window.getPixel(x, y) == 1) {
                                    color = 0;
                                    m_Registers[0xF] = 1;
                                }

                                m_Window.setPixel(x, y, color);
                            }
                        }
                    }
                    break;
                }

                case 0xE000:
                {
                    switch(opcode & 0x000F){
                        case 0x000E:
                        {
                            int regx = (opcode & 0x0F00) >> 8;
                            int key_in_Vx = m_Registers[regx];

                            if(m_Keyboard.get_key_value(key_in_Vx) == 1){
                                m_ProgramCounter +=2;
                            }
                            break;
                        }
                        case 0x0001:
                        {
                            int regx = (opcode & 0x0F00) >> 8;
                            int key_in_Vx = m_Registers[regx];
                            if(m_Keyboard.get_key_value(key_in_Vx) == 0){
                                m_ProgramCounter +=2;
                            }
                            break;
                        }
                    }
                }
                case 0xF000:
                {
                    switch(opcode & 0x00FF){
                        case 0x0007:
                        {
                            int regx = (opcode & 0x0F00) >> 8;
                            m_Registers[regx] = delayTimer;
                            break;
                        }
                        case 0x000A:
                        {
                            int regx = (opcode & 0x0F00) >> 8;
                            int keypressed = m_Keyboard.get_key_pressed();

                            if (keypressed == 0)
                                m_ProgramCounter -= 2;
                            else{
                                m_Registers[regx] = keypressed;
                                
                                while(m_Keyboard.get_key_pressed() != 0){
                                    bool f;
                                    m_Keyboard.poll_status(f);
                                }
                            }
                            break;
                        }
                        case 0x0015:
                        {
                            int regx = (opcode & 0x0F00) >> 8;
                            delayTimer = m_Registers[regx];
                            break;
                        }
                        case 0x0018:
                        {
                            // Sets the sound timer to VX.
                            break;
                        }
                        case 0x001E:
                        {
                            int regx = (opcode & 0x0F00) >> 8;
                            auto Vx = m_Registers[regx];
                            m_AddressI += Vx;
                            break;
                        }
                        case 0x0029:
                        {
                            int regx = (opcode & 0x0F00) >> 8;
                            m_AddressI = m_Registers[regx];
                            break;
                        }
                        case 0x0033:
                        {
                            int regx = (opcode & 0x0F00) >> 8;

                            int value = m_Registers[regx];

                            int hundreds = value / 100;
                            int tens = (value / 10) % 10;
                            int units = value % 10;

                            m_GameMemory[m_AddressI] = hundreds;
                            m_GameMemory[m_AddressI+1] = tens;
                            m_GameMemory[m_AddressI+2] = units;
                            break;
                        }
                        case 0x0055:
                        {
                            int regx = (opcode & 0x0F00) >> 8;

                            for (int i = 0; i <= regx; i++)
                            {
                                m_GameMemory[m_AddressI+i] = m_Registers[i];
                            }

                            m_AddressI= m_AddressI + regx + 1;
                            break;
                        }
                        case 0x0065:
                        {
                            int regx = (opcode & 0x0F00) >> 8;

                            for (int i = 0; i <= regx; i++)
                            {
                                m_Registers[i] = m_GameMemory[m_AddressI+i] ;
                            }

                            m_AddressI= m_AddressI + regx + 1;
                            break;
                        }
                    }

                    break;
                }
                default : {
                    std::cerr << "Can't interpret instruction " << opcode;
                    break;
                }
        }
}

};

#endif