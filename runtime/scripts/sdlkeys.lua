local SDL = {
    Key = {
        A = 4,
        B = 5,
        C = 6,
        D = 7,
        E = 8,
        F = 9,
        G = 10,
        H = 11,
        I = 12,
        J = 13,
        K = 14,
        L = 15,
        M = 16,
        N = 17,
        O = 18,
        P = 19,
        Q = 20,
        R = 21,
        S = 22,
        T = 23,
        U = 24,
        V = 25,
        W = 26,
        X = 27,
        Y = 28,
        Z = 29,

        Key1 = 30,
        Key2 = 31,
        Key3 = 32,
        Key4 = 33,
        Key5 = 34,
        Key6 = 35,
        Key7 = 36,
        Key8 = 37,
        Key9 = 38,
        Key0 = 39,

        RETURN = 40,
        ESCAPE = 41,
        BACKSPACE = 42,
        TAB = 43,
        SPACE = 44,

        MINUS = 45,
        EQUALS = 46,
        LEFTBRACKET = 47,
        RIGHTBRACKET = 48,
        BACKSLASH = 49,  --   Located at the lower left of the return
                                      --   on ISO oards and at the right end
                                      --   of the QWERTY row on ANSI oards.
                                      --   Produces REVERSE SOLIDUS (backslash) and
                                      --   VERTICAL LINE in a US layout, REVERSE
                                      --   SOLIDUS and VERTICAL LINE in a UK Mac
                                      --   layout, NUMBER SIGN and TILDE in a UK
                                      --   Windows layout, DOLLAR SIGN and POUND SIGN
                                      --   in a Swiss German layout, NUMBER SIGN and
                                      --   APOSTROPHE in a German layout, GRAVE
                                      --   ACCENT and POUND SIGN in a French Mac
                                      --   layout, and ASTERISK and MICRO SIGN in a
                                      --   French Windows layout.
                                      --
        NONUSHASH = 50,  --   ISO USB oards actually use this code
                                      --   instead of 49 for the same  but all
                                      --   OSes I've seen treat the two codes
                                      --   identically. So, as an implementor, unless
                                      --   your oard generates both of those
                                      --   codes and your OS treats them differently,
                                      --   you should generate BACKSLASH
                                      --   instead of this code. As a user, you
                                      --   should not rely on this code because SDL
                                      --   will never generate it with most (all?)
                                      --   oards.
                                      --
        SEMICOLON = 51,
        APOSTROPHE = 52,
        GRAVE = 53,  --   Located in the top left corner (on both ANSI
                                  --   and ISO oards). Produces GRAVE ACCENT and
                                  --   TILDE in a US Windows layout and in US and UK
                                  --   Mac layouts on ANSI oards, GRAVE ACCENT
                                  --   and NOT SIGN in a UK Windows layout, SECTION
                                  --   SIGN and PLUS-MINUS SIGN in US and UK Mac
                                  --   layouts on ISO oards, SECTION SIGN and
                                  --   DEGREE SIGN in a Swiss German layout (Mac:
                                  --   only on ISO oards), CIRCUMFLEX ACCENT and
                                  --   DEGREE SIGN in a German layout (Mac: only on
                                  --   ISO oards), SUPERSCRIPT TWO and TILDE in a
                                  --   French Windows layout, COMMERCIAL AT and
                                  --   NUMBER SIGN in a French Mac layout on ISO
                                  --   oards, and LESS-THAN SIGN and GREATER-THAN
                                  --   SIGN in a Swiss German, German, or French Mac
                                  --   layout on ANSI oards.
                                  
        COMMA = 54,
        PERIOD = 55,
        SLASH = 56,

        CAPSLOCK = 57,

        F1 = 58,
        F2 = 59,
        F3 = 60,
        F4 = 61,
        F5 = 62,
        F6 = 63,
        F7 = 64,
        F8 = 65,
        F9 = 66,
        F10 = 67,
        F11 = 68,
        F12 = 69,

        PRINTSCREEN = 70,
        SCROLLLOCK = 71,
        PAUSE = 72,
        INSERT = 73, -- insert on PC, help on some Mac oards (but
                                  -- does send code 73, not 117) 
        HOME = 74,
        PAGEUP = 75,
        DELETE = 76,
        END = 77,
        PAGEDOWN = 78,
        RIGHT = 79,
        LEFT = 80,
        DOWN = 81,
        UP = 82,

        NUMLOCKCLEAR = 83, -- num lock on PC, clear on Mac oards
                                         
        KP_DIVIDE = 84,
        KP_MULTIPLY = 85,
        KP_MINUS = 86,
        KP_PLUS = 87,
        KP_ENTER = 88,
        KP_1 = 89,
        KP_2 = 90,
        KP_3 = 91,
        KP_4 = 92,
        KP_5 = 93,
        KP_6 = 94,
        KP_7 = 95,
        KP_8 = 96,
        KP_9 = 97,
        KP_0 = 98,
        KP_PERIOD = 99,

        NONUSBACKSLASH = 100,  --   This is the additional that ISO
                                            --   oards have over ANSI ones,
                                            --   located between left shift and Y.
                                            --   Produces GRAVE ACCENT and TILDE in a
                                            --   US or UK Mac layout, REVERSE SOLIDUS
                                            --   (backslash) and VERTICAL LINE in a
                                            --   US or UK Windows layout, and
                                            --   LESS-THAN SIGN and GREATER-THAN SIGN
                                            --   in a Swiss German, German, or French
                                            --   layout.
        APPLICATION = 101, --- < windows contextual menu, compose */
        POWER = 102, ---- The USB document says this is a status flag,
                                   --   not a physical - but some Mac oards
                                   --   do have a power  --/
        KP_EQUALS = 103,
        F13 = 104,
        F14 = 105,
        F15 = 106,
        F16 = 107,
        F17 = 108,
        F18 = 109,
        F19 = 110,
        F20 = 111,
        F21 = 112,
        F22 = 113,
        F23 = 114,
        F24 = 115,
        EXECUTE = 116,
        HELP = 117,
        MENU = 118,
        SELECT = 119,
        STOP = 120,
        AGAIN = 121,
        UNDO = 122,
        CUT = 123,
        COPY = 124,
        PASTE = 125,
        FIND = 126,
        MUTE = 127,
        VOLUMEUP = 128,
        VOLUMEDOWN = 129,
        KP_COMMA = 133,
        KP_EQUALSAS400 = 134,

        INTERNATIONAL1 = 135, -- used on Asian oards, see
                                           -- footnotes in USB doc */
        INTERNATIONAL2 = 136,
        INTERNATIONAL3 = 137, -- Yen
        INTERNATIONAL4 = 138,
        INTERNATIONAL5 = 139,
        INTERNATIONAL6 = 140,
        INTERNATIONAL7 = 141,
        INTERNATIONAL8 = 142,
        INTERNATIONAL9 = 143,
        LANG1 = 144, -- Hangul/English toggle */
        LANG2 = 145, -- Hanja conversion */
        LANG3 = 146, -- Katakana */
        LANG4 = 147, -- Hiragana */
        LANG5 = 148, -- Zenkaku/Hankaku */

        SYSREQ = 154,
        CANCEL = 155,
        CLEAR = 156,
        PRIOR = 157,
        RETURN2 = 158,
        SEPARATOR = 159,
        OUT = 160,
        OPER = 161,
        CLEARAGAIN = 162,
        CRSEL = 163,
        EXSEL = 164,

        KP_00 = 176,
        KP_000 = 177,
        THOUSANDSSEPARATOR = 178,
        DECIMALSEPARATOR = 179,
        CURRENCYUNIT = 180,
        CURRENCYSUBUNIT = 181,
        KP_LEFTPAREN = 182,
        KP_RIGHTPAREN = 183,
        KP_LEFTBRACE = 184,
        KP_RIGHTBRACE = 185,
        KP_TAB = 186,
        KP_BACKSPACE = 187,
        KP_A = 188,
        KP_B = 189,
        KP_C = 190,
        KP_D = 191,
        KP_E = 192,
        KP_F = 193,
        KP_XOR = 194,
        KP_POWER = 195,
        KP_PERCENT = 196,
        KP_LESS = 197,
        KP_GREATER = 198,
        KP_AMPERSAND = 199,
        KP_DBLAMPERSAND = 200,
        KP_VERTICALBAR = 201,
        KP_DBLVERTICALBAR = 202,
        KP_COLON = 203,
        KP_HASH = 204,
        KP_SPACE = 205,
        KP_AT = 206,
        KP_EXCLAM = 207,
        KP_MEMSTORE = 208,
        KP_MEMRECALL = 209,
        KP_MEMCLEAR = 210,
        KP_MEMADD = 211,
        KP_MEMSUBTRACT = 212,
        KP_MEMMULTIPLY = 213,
        KP_MEMDIVIDE = 214,
        KP_PLUSMINUS = 215,
        KP_CLEAR = 216,
        KP_CLEARENTRY = 217,
        KP_BINARY = 218,
        KP_OCTAL = 219,
        KP_DECIMAL = 220,
        KP_HEXADECIMAL = 221,

        LCTRL = 224,
        LSHIFT = 225,
        LALT = 226, --/**< alt, option */
        LGUI = 227, --/**< windows, command (apple), meta */
        RCTRL = 228,
        RSHIFT = 229,
        RALT = 230, --/**< alt gr, option */
        RGUI = 231, --/**< windows, command (apple), meta */

        MODE = 257,    
        AUDIONEXT = 258,
        AUDIOPREV = 259,
        AUDIOSTOP = 260,
        AUDIOPLAY = 261,
        AUDIOMUTE = 262,
        MEDIASELECT = 263,
        WWW = 264,
        MAIL = 265,
        CALCULATOR = 266,
        COMPUTER = 267,
        AC_SEARCH = 268,
        AC_HOME = 269,
        AC_BACK = 270,
        AC_FORWARD = 271,
        AC_STOP = 272,
        AC_REFRESH = 273,
        AC_BOOKMARKS = 274,

        BRIGHTNESSDOWN = 275,
        BRIGHTNESSUP = 276,
        DISPLAYSWITCH = 277, -- display mirroring/dual display
                                          -- switch, video mode switch
        KBDILLUMTOGGLE = 278,
        KBDILLUMDOWN = 279,
        KBDILLUMUP = 280,
        EJECT = 281,
        SLEEP = 282,

        APP1 = 283,
        APP2 = 284,

        NUM_SCANCODES = 512
    }
}

SDL.Key['1'] = 30
SDL.Key['2'] = 31
SDL.Key['3'] = 32
SDL.Key['4'] = 33
SDL.Key['5'] = 34
SDL.Key['6'] = 35
SDL.Key['7'] = 36
SDL.Key['8'] = 37
SDL.Key['9'] = 38
SDL.Key['0'] = 39

KeyEvent = {}
KeyEvent.Nothing = 0
KeyEvent.Press = 1
KeyEvent.Release = 2
KeyEvent.Hold = 3

KeyEventEnum = { "Nothing", "Press", "Release", "Hold" }

return SDL
