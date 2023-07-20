#ifndef CHIP8_CPU
#define CHIP8_CPU
#include <stdio.h>
#include <iostream>
#include <vector>
#include "chip8_screen.h"
#include "chip8_keyboard.h"

typedef unsigned char BYTE;
typedef unsigned short int WORD;

class chip8_cpu {
    private:
        BYTE m_GameMemory[0xFFF]; // 0xFFF bytes of memory, 4095 bytes
        BYTE m_Registers[16] ; // 16 registers, 1 byte each
        WORD m_AddressI; // the 16-bit address register I
        WORD m_ProgramCounter; // the 16-bit program counter
        std::vector<WORD> m_Stack; // the 16-bit stack
        uint8_t delayTimer;
        chip8_screen& m_Window;
        chip8_keyboard& m_Keyboard;

        WORD GetNextOpcode()
        {
            WORD res = 0 ;
            res = m_GameMemory[m_ProgramCounter]; // in example res is 0xAB
            res <<= 8 ; // shift 8 bits left. In our example res is 0xAB00
            res |= m_GameMemory[m_ProgramCounter+1]; //In example res is 0xABCD
            m_ProgramCounter+=2;
            return res ;
        }
    public: 
    
        chip8_cpu (chip8_screen& window, chip8_keyboard& keyboard) : m_Window(window), m_Keyboard(keyboard) {
            CPUReset();
        }

        void CPUReset()
        {
            m_AddressI = 0 ;
            m_ProgramCounter = 0x200;
            delayTimer = 0;
            memset(m_Registers, 0, sizeof(m_Registers)) ;
            m_Window.clear();

            // load in the game
            FILE *in;
            in = fopen("quirks", "rb");
            m_GameMemory[0x1FF] = 0;
            fread( &m_GameMemory[0x200], 0xfff, 1, in);
            fclose(in);
        }

        void start_program(){
            if (delayTimer > 0) {
                --delayTimer;
            }

            int opcode = GetNextOpcode();
            std::cout << "instruction " << std::hex << opcode << std::endl;
            switch (opcode & 0xF000)
            {   
                case 0x1000:
                {
                    m_ProgramCounter = opcode & 0x0FFF;
                    break ;
                } 
                case 0x0000:
                {
                    switch(opcode & 0x000F)
                    {
                        case 0x0000:{
                            m_Window.clear();

                            break; // clear screen opcode
                        } 
                        case 0x000E: {
                            m_ProgramCounter = m_Stack.back();
                            m_Stack.pop_back();
                            break; // return subroutine
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
                    WORD register_index = opcode & 0x0F00;
                    WORD constant = opcode & 0x00FF;
                    if(m_Registers[register_index >>= 8] != constant){
                        m_ProgramCounter+=2;
                    }
                    break;
                }
                case 0x5000:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    if(m_Registers[registerX_index >>= 8] == m_Registers[registerY_index >>= 4]){
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
                            m_Registers[0xF] = 0 ;
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
                            m_Registers[0xF] = 1 ;
                            
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
                            m_Registers[0xF] = 1 ;

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
                    int regx = opcode & 0x0F00;
                    regx >>= 8;
                    m_Registers[regx] = rand() & (opcode & 0x00FF);
                    break;
                }
                case 0xD000:
                {
                    int regx = (opcode & 0x0F00) >> 8;
                    int regy = (opcode & 0x00F0) >> 4;

                    int x_cord = m_Registers[regx];
                    int y_cord = m_Registers[regy];
                    int height = opcode & 0x000F;
                    m_Registers[0xF] = 0;

                    for (int row = 0; row < height; row++) {
                        BYTE data = m_GameMemory[m_AddressI + row];

                        int xpixel = 0;
                        int xpixelinv = 7;
                        for (xpixel = 0; xpixel < 8; xpixel++, xpixelinv--) {
                            int mask = 1 << xpixelinv;
                            if (data & mask) {
                            int x = (x_cord + xpixel) % 64; // Wrap X coordinate if necessary
                            int y = (y_cord + row) % 32;    // Wrap Y coordinate if necessary

                                int color = 0;
                                if (m_Window.getPixel(x,y) == 0) {
                                    color = 255;
                                    m_Registers[0xF] = 1;
                                }

                                m_Window.setPixel(x,y,color);
                            }
                        }
                    }
                    m_Window.drawPixels();
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
                            int regx = opcode & 0x0F00;
                            regx >>= 8;
                            int keypressed = m_Keyboard.get_key_pressed();

                            if (keypressed == -1)
                                m_ProgramCounter -= 2 ;
                            else
                                m_Registers[regx] = keypressed ;
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
                            int constant = opcode & 0x0F00;
                            constant >>= 8;
                            auto Vx = m_Registers[constant];
                            m_AddressI += Vx;
                            break;
                        }
                        case 0x0029:
                        {
                            int regx = opcode & 0x0F00;
                            regx >>= 8 ;
                            m_AddressI = m_Registers[regx];
                            break;
                        }
                        case 0x0033:
                        {
                            int regx = opcode & 0x0F00 ;
                            regx >>= 8 ;

                            int value = m_Registers[regx] ;

                            int hundreds = value / 100 ;
                            int tens = (value / 10) % 10 ;
                            int units = value % 10 ;

                            m_GameMemory[m_AddressI] = hundreds ;
                            m_GameMemory[m_AddressI+1] = tens ;
                            m_GameMemory[m_AddressI+2] = units ;
                            break;
                        }
                        case 0x0055:
                        {
                            int regx = opcode & 0x0F00 ;
                            regx >>= 8 ;

                            for (int i = 0 ; i <= regx; i++)
                            {
                                m_GameMemory[m_AddressI+i] = m_Registers[i] ;
                            }

                            m_AddressI= m_AddressI+regx+1 ;
                            break;
                        }
                        case 0x0065:
                        {
                            int regx = opcode & 0x0F00 ;
                            regx >>= 8 ;

                            for (int i = 0 ; i <= regx; i++)
                            {
                                m_Registers[i] = m_GameMemory[m_AddressI+i]  ;
                            }

                            m_AddressI= m_AddressI+regx+1 ;
                            break;
                        }
                    }

                    break;
                }
                break ;
                default : {
                    std::cerr << "Can't interpret instruction " << opcode;
                    break ; // opcode yet to be handled
                }
        }
}

};

#endif