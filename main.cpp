#include <stdio.h>
#include <iostream>  // For basic I/O operations (optional but often used)
#include <vector>    // Include the vector header
#include <cstring> // Include this header for using memset

typedef unsigned char BYTE;
typedef unsigned short int WORD;

BYTE m_GameMemory[0xFFF] ; // 0xFFF bytes of memory, 4095 bytes
BYTE m_Registers[16] ; // 16 registers, 1 byte each
WORD m_AddressI ; // the 16-bit address register I
WORD m_ProgramCounter ; // the 16-bit program counter
std::vector<WORD> m_Stack; // the 16-bit stack

BYTE m_ScreenData[64][32];

WORD GetNextOpcode( )
{
   WORD res = 0 ;
   res = m_GameMemory[m_ProgramCounter] ; // in example res is 0xAB
   res <<= 8 ; // shift 8 bits left. In our example res is 0xAB00
   res |= m_GameMemory[m_ProgramCounter+1] ; //In example res is 0xABCD
   m_ProgramCounter+=2 ;
   return res ;
}

void CPUReset()
{
   m_AddressI = 0 ;
   m_ProgramCounter = 0x200 ;
   memset(m_Registers, 0, sizeof(m_Registers)) ; // set registers to 0

   // load in the game
   FILE *in;
   in = fopen( "INVADERS", "rb" );
   fread( &m_GameMemory[0x200], 0xfff, 1, in);
   fclose(in);
}

void Opcode1NNN(WORD opcode)
{
   m_ProgramCounter = opcode & 0x0FFF;
}


void Opcode00E0()
{
    memset(m_ScreenData, 255, sizeof(m_ScreenData));
}


void Opcode00EE()
{
    m_ProgramCounter = m_Stack.back();
    m_Stack.pop_back();
}

int main(int argc, char* argv[])
{
    printf("size %i\n", sizeof(BYTE));
    printf("size %i\n", sizeof(WORD));
    Opcode00E0();
    return 0;
    CPUReset();

    WORD opcode = GetNextOpcode( ) ; // assume this returns 0x1234

    // decode the opcode
    switch (opcode & 0xF000)
    {
        case 0x1000: 
            Opcode1NNN(opcode); 
            break ; // jump opcode
        case 0x0000: // need to break this opcode down further
        {
            switch(opcode & 0x000F)
            {
                case 0x0000: 
                    Opcode00E0();
                    break; // clear screen opcode
                case 0x000E: 
                    Opcode00EE(); 
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
        break ;
        default : break ; // opcode yet to be handled
    }
    return 0;
}
