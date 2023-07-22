#ifndef CHIP8_KEYBOARD
#define CHIP8_KEYBOARD

#define CHIP8_KEYS 18
typedef unsigned char BYTE;
typedef unsigned short int WORD;

class chip8_keyboard {
    private:
        BYTE m_keys[CHIP8_KEYS] = {0};
        // Initialize the mapping of SDL keys to CHIP-8 keycodes
        std::unordered_map<SDL_Keycode, uint8_t> keyMap = {
            {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
            {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
            {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
            {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF},
            {SDLK_j, 0x10}, {SDLK_k, 0x11}
        };

        void handle_keyboard_input(int key, int state, bool& quit){
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
    public:
        void poll_status(bool &quit){
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) 
                    quit = true;
                else if (event.type == SDL_KEYDOWN) {
                    handle_keyboard_input(event.key.keysym.sym, 1, quit);
                }
                else if (event.type == SDL_KEYUP) {
                    handle_keyboard_input(event.key.keysym.sym, 0, quit);
                }
            }
        }

        int get_key_pressed(){
            int res = 0;

            for (int i = 0; i < CHIP8_KEYS; i++)
            {
                if (m_keys[i] == 1)
                    return i;
            }
            return res;
        }

        int get_key_value(int index){
            return m_keys[index];
        }

};
#endif
