#include <stdio.h>
#include <iostream>  // For basic I/O operations (optional but often used)
#include <vector>    // Include the vector header
#include <cstring> // Include this header for using memset
#include <SDL2/SDL.h>
#include <unordered_map>

typedef unsigned char BYTE;
typedef unsigned short int WORD;

BYTE m_GameMemory[0xFFF] ; // 0xFFF bytes of memory, 4095 bytes
BYTE m_Registers[16] ; // 16 registers, 1 byte each
WORD m_AddressI ; // the 16-bit address register I
WORD m_ProgramCounter ; // the 16-bit program counter
std::vector<WORD> m_Stack; // the 16-bit stack
uint8_t delayTimer;
BYTE m_ScreenData[64][32] = {{0}};

BYTE m_keys[16];

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;
const int PIXEL_SIZE = 10; // Adjust this to control the size of each pixel


SDL_Window* window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        640, 320, SDL_WINDOW_SHOWN);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

// Initialize the mapping of SDL keys to CHIP-8 keycodes
std::unordered_map<SDL_Keycode, uint8_t> keyMap = {
    {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
    {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
    {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
    {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF}
};


WORD GetNextOpcode( )
{
   WORD res = 0 ;
   res = m_GameMemory[m_ProgramCounter]; // in example res is 0xAB
   res <<= 8 ; // shift 8 bits left. In our example res is 0xAB00
   res |= m_GameMemory[m_ProgramCounter+1]; //In example res is 0xABCD
   m_ProgramCounter+=2;
   return res ;
}

int get_key_pressed(){
    int res = -1;

	for (int i = 0 ; i < 16; i++)
	{
		if (m_keys[i] > 0)
			return i;
	}

	return res;
}
void CPUReset()
{
    m_AddressI = 0 ;
    m_ProgramCounter = 0x200;
    delayTimer = 0;
    memset(m_Registers, 0, sizeof(m_Registers)) ; // set registers to 0
    memset(m_ScreenData, 255, sizeof(m_ScreenData));

    // load in the game
    FILE *in;
    in = fopen( "INVADERS", "rb" );
    fread( &m_GameMemory[0x200], 0xfff, 1, in);
    fclose(in);
}
void drawPixels(SDL_Renderer* renderer, unsigned char pixels[64][32]) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set the background color to black
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set the pixel color to white

    for (int x = 0; x < 64; ++x) {
        for (int y = 0; y < 32; ++y) {
            if (pixels[x][y] == 255) {
                SDL_Rect pixelRect = { x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
                SDL_RenderFillRect(renderer, &pixelRect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void handle_instruction(int opcode){
    // decode the opcode
    std::cout << "instruction " << std::hex <<opcode << std::endl;
    switch (opcode & 0xF000)
    {   
        case 0x1000:
        {
            m_ProgramCounter = opcode & 0x0FFF;
            break ; // jump opcode
        } 
        case 0x0000: // need to break this opcode down further
        {
            switch(opcode & 0x000F)
            {
                case 0x0000:{
                    memset(m_ScreenData, 255, sizeof(m_ScreenData));

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
        case 0x2000: // 2NNN
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
                
                case 0x0001:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    registerX_index >>= 8;
                    registerY_index >>= 4;
                    m_Registers[registerX_index] |= m_Registers[registerY_index];
                    break;
                }
                case 0x0002:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    registerX_index >>= 8;
                    registerY_index >>= 4;
                    m_Registers[registerX_index] &= m_Registers[registerY_index];
                    break;
                }
                case 0x0003:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    registerX_index >>= 8;
                    registerY_index >>= 4;
                    m_Registers[registerX_index] ^= m_Registers[registerY_index];
                    break;
                }
                case 0x0004:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    registerX_index >>= 8;
                    registerY_index >>= 4;
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
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    registerX_index >>= 8;
                    registerY_index >>= 4;
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
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    registerX_index >>= 8;
                    registerY_index >>= 4;
                    m_Registers[0xF] = m_Registers[registerX_index] & 0x1;
                    m_Registers[registerX_index] >>= 1;
                    
                    break;
                }
                case 0x0007:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    WORD registerY_index = opcode & 0x00F0;
                    registerX_index >>= 8;
                    registerY_index >>= 4;
                	m_Registers[0xF] = 1 ;

                    int value = m_Registers[registerY_index] - m_Registers[registerX_index];
                    if(value < 0)
                		m_Registers[0xF] = 0;

                    m_Registers[registerX_index] = value;
                    break;
                }
                case 0x000E:
                {
                    WORD registerX_index = opcode & 0x0F00;
                    registerX_index >>= 8;
                    m_Registers[0xF] = m_Registers[registerX_index] >> 7;
                    m_Registers[registerX_index] <<= 1;
                    break;
                }
            }
            break;
        }
        case 0x9000:
        {
            WORD registerX_index = opcode & 0x0F00;
            WORD registerY_index = opcode & 0x00F0;
            if(m_Registers[registerX_index >>= 8] != m_Registers[registerY_index >>= 4]){
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
                        if (m_ScreenData[x][y] == 0) {
                            color = 255;
                            m_Registers[0xF] = 1;
                        }

                        m_ScreenData[x][y] = color;
                    }
                }
            }
            drawPixels(renderer, m_ScreenData);
            break;
        }

        case 0xE000:
        {
            switch(opcode & 0x000F){
                case 0x000E:
                {
                    int regx = (opcode & 0x0F00) >> 8;
                    int key_in_Vx = m_Registers[regx];
                    std::cout << "key_in_Vx " <<key_in_Vx << std::endl;
                    std::cout << "m_keys[key_in_Vx]" << static_cast<int>(m_keys[key_in_Vx]) << std::endl;
                    if(m_keys[key_in_Vx] == 1){
                        m_ProgramCounter +=2;
                    }
                    break;
                }
                case 0x0001:
                {
                    int regx = (opcode & 0x0F00) >> 8;
                    int key_in_Vx = m_Registers[regx];
                    if(m_keys[key_in_Vx] == 0){
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
                    int keypressed = get_key_pressed();

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

void key_press(int key, int state, bool& quit){
    switch (key) {
        case SDLK_ESCAPE:
            quit = true;
            break;
        default:
            auto it = keyMap.find(key);
            if (it != keyMap.end()) {
                m_keys[it->second] = state;
            }
            break;
    }
}


int main(int argc, char* argv[])
{


    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Failed to initialize the SDL2 library\n";
        return -1;
    }


    // Event loop
    bool quit = false;
    unsigned int time = SDL_GetTicks();
    CPUReset();
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) 
                quit = true;
            else if (event.type == SDL_KEYDOWN) {
                std::cout << event.key.keysym.sym << std::endl;
                key_press(event.key.keysym.sym, 1, quit);
            }
            else if (event.type == SDL_KEYUP) 
                key_press(event.key.keysym.sym, 0, quit);
        }
        
        unsigned int current = SDL_GetTicks();
        //if(current - time >= (1000/60)){
            WORD opcode = GetNextOpcode( ) ;
            handle_instruction(opcode);
            time = current;

        //}
        
        if (delayTimer > 0) {
            --delayTimer;
        }
        SDL_Delay(16.7);
    }

    // Clean up and quit SDL
    SDL_DestroyWindow(window);
    SDL_Quit();
 
    return 0;
}
