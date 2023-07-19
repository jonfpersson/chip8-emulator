#ifndef CHIP8_KEYBOARD
#define CHIP8_KEYBOARD

typedef unsigned char BYTE;
typedef unsigned short int WORD;

class chip8_keyboard {
    private:
    BYTE m_keys[16];
    // Initialize the mapping of SDL keys to CHIP-8 keycodes
    std::unordered_map<SDL_Keycode, uint8_t> keyMap = {
        {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
        {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
        {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
        {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF}
    };

    public:
    
    int get_key_pressed(){
        int res = -1;

        for (int i = 0 ; i < 16; i++)
        {
            if (m_keys[i] > 0)
                return i;
        }
        return res;
    }

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

    int get_key_value(int index){
        return m_keys[index];
    }

};
#endif
