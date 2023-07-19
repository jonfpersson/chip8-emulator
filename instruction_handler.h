#ifndef INSTRUCTION_HANDLER
#define INSTRUCTION_HANDLER
#include <iostream>  // For basic I/O operations (optional but often used)

class instruction_handler {
    public:
        instruction_handler() {

        }
        int execute(int opcode) {
            
            switch (opcode & 0xF000)
            {
                case 0x1000: 
                    m_ProgramCounter = opcode & 0x0FFF;
                    break ; // jump opcode
                case 0x0000: // need to break this opcode down further
                {
                    switch(opcode & 0x000F)
                    {
                        case 0x0000: 
                            memset(m_ScreenData, 255, sizeof(m_ScreenData));
                            break; // clear screen opcode
                        case 0x000E: 
                            m_ProgramCounter = m_Stack.back();
                            m_Stack.pop_back();
                            break; // return subroutine
                    }
                }
                case 0x2000: // 2NNN
                {
                    m_Stack.push_back(m_ProgramCounter);
                    m_ProgramCounter = opcode & 0x0FFF;
                }
                case 0x3000:
                {
                    WORD register_index = opcode & 0x0F00;
                    WORD constant = opcode & 0x00FF;
                    if(m_Registers[register_index >>= 8] == constant){
                        m_ProgramCounter+=2;
                    }
                }
                case 0x4000:
                {
                    WORD register_index = opcode & 0x0F00;
                    WORD constant = opcode & 0x00FF;
                    if(m_Registers[register_index >>= 8] != constant){
                        m_ProgramCounter+=2;
                    }
                }
                case 0x5000:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    if(m_Registers[registerX_index >>= 8] == m_Registers[registerY_index >>= 4]){
                        m_ProgramCounter+=2;
                    }
                }
                case 0x6000:
                {
                    WORD register_index = opcode & 0x0F00;
                    WORD constant = opcode & 0x00FF;
                    m_Registers[register_index >>= 8] = constant;
                }
                case 0x7000:
                {
                    WORD register_index = opcode & 0x0F00;
                    WORD constant = opcode & 0x00FF;
                    m_Registers[register_index >>= 8] += constant;
                }
                case 0x8000:
                {
                    switch(opcode & 0x000F){
                        WORD registerX_index = opcode & 0x0F00;
                        WORD registerY_index = opcode & 0x00F0;
                        registerX_index >>= 8;
                        registerY_index >>= 4;
                        case 0x0001:
                        {
                            m_Registers[registerX_index] |= m_Registers[registerY_index];
                        }
                        case 0x0002:
                        {
                            m_Registers[registerX_index] &= m_Registers[registerY_index];
                        }
                        case 0x0003:
                        {
                            m_Registers[registerX_index] ^= m_Registers[registerY_index];
                        }
                        case 0x0004:
                        {
                            m_Registers[0xF] = 0 ;
                            int value = m_Registers[registerX_index] + m_Registers[registerY_index];
                            if(value > 255){
                                m_Registers[0xF] = 1;
                            }
                            
                            m_Registers[registerX_index] += m_Registers[registerY_index];
                        }
                        case 0x0005:
                        {
                            m_Registers[0xF] = 1 ;
                            
                            int value = m_Registers[registerX_index] - m_Registers[registerY_index];
                            if(value < 0){
                                m_Registers[0xF] = 0;
                            }
                            
                            m_Registers[registerX_index] -= m_Registers[registerY_index];
                        }case 0x0006:
                        {
                            m_Registers[0xF] = m_Registers[registerX_index] & 0x1;
                            m_Registers[registerX_index] >>= 1;
                            
                        }
                        case 0x0007:
                        {
                            m_Registers[0xF] = 1 ;

                            int value = m_Registers[registerY_index] - m_Registers[registerX_index];
                            if(value < 0)
                                m_Registers[0xF] = 0;

                            m_Registers[registerX_index] = value;
                        }
                        case 0x000E:
                        {
                            m_Registers[0xF] = m_Registers[registerX_index] >> 7;
                            m_Registers[registerX_index] <<= 1;
                        }
                    }
                }
                case 0x9000:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    if(m_Registers[registerX_index >>= 8] != m_Registers[registerY_index >>= 4]){
                        m_ProgramCounter+=2;
                    }
                }
                case 0xA000:
                {
                    m_AddressI = 0xA000 & 0x0FFF;
                }
                case 0xB000:
                {
                    m_ProgramCounter = m_Registers[0] + (opcode & 0x0FFF);
                }
                case 0xC000:
                {
                    int regx = opcode & 0x0F00;
                    regx >>= 8;
                    m_Registers[regx] = rand() & (opcode & 0x00FF);
                }
                case 0xD000:
                {
                    //draw stuff
                }
                case 0xE000:
                {
                    switch(opcode & 0x000F){
                        case 0x000E:
                        {
                            //if (key() == Vx)

                        }
                        case 0x0001:
                        {
                            //if (key() != Vx)
                            
                        }
                    }
                }
                case 0xF000:
                {
                    switch(opcode & 0x00FF){
                        case 0x0007:
                        {
                            //Sets VX to the value of the delay timer.

                        }
                        case 0x000A:
                        {
                            //A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event).
                        }
                        case 0x0015:
                        {
                            // Sets the delay timer to VX.

                        }
                        case 0x0018:
                        {
                            // Sets the sound timer to VX.
                        }
                        case 0x001E:
                        {
                            int constant = opcode & 0x0F00;
                            constant >>= 8;
                            auto Vx = m_Registers[constant];
                            m_AddressI += Vx;
                        }
                        case 0x0029:
                        {
                            int regx = opcode & 0x0F00;
                            regx >>= 8 ;
                            m_AddressI = m_Registers[regx];
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
                        }
                    }

                }
                break ;
                default : 
                    std::cerr << "Can't interpret instruction " << opcode;
                    break ; // opcode yet to be handled
            }
    
        }
    private:


};

#endif
