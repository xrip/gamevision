#include <supervision.h>
#include <string.h>
#include <stdio.h>
#include "font.h"

#define WORDS_PER_LINE (160 / 8 + 4)

#define CHAR_ON_LINE 18
#define ROM_INFO_SIZE 22
#define GAMES_PER_PAGE 18

#define GAMECOUNT *((unsigned char*)0x9100)
#define GAMEINFO ((unsigned char*)0x9101)
#define GAMEINFO_SIZE 56

struct sv_vram
{
  unsigned int v[160 / 8][8][WORDS_PER_LINE];
};

#define SV_VRAM ((*(struct sv_vram *)0x4000).v)

struct __sv_dma_buffer
{
    unsigned short source;
    unsigned short destination;
    unsigned char lenght; //*16
    unsigned char control;
};
#define SV_DMA_BUFFER (*(struct __sv_dma_buffer *)0x2008)
#define SV_IRQ_STATUS (*(unsigned char *)0x2027)
#define SV_RESET_DMA_IRQ (*(unsigned char *)0x2025)

#define DMA_AUDIO_SYSTEM_MASK 0x2

static void init(void)
{
    SV_LCD.width = 160;
    SV_LCD.height = 160;
}
#define ZP_PTR ((volatile unsigned char*)0x02)

#define CARTRIDGE_CONTROL (*(unsigned char*)0x9080)

static void clear_screen(void)
{
  memset(SV_VIDEO, 0, 0x4000);
}

static void __fastcall__ beep()
{
  SV_LEFT.delay = 300;
  SV_LEFT.timer = 2;
  SV_LEFT.control = 0b00111111;
  SV_RIGHT.delay = 254;
  SV_RIGHT.timer = 2;
  SV_RIGHT.control = 0b00111111;
}

static void display_char(const unsigned char x, const unsigned char y, const unsigned int *ch, const char invert)
{
  unsigned char i;
  unsigned int word;

  for (i = 0; i < 8; ++i) {
    word = ch[i];
    if (invert) word = ~word;
    SV_VRAM[y][i][x] = word;
  }
}

static void print(char *str, int len, unsigned char x, unsigned char y, char invert)
{
  unsigned char i;

  for (i = 0; i < len; i++) {
    char character = str[i];
    display_char(x + i, y, &font[(character - 32) * 8], invert);
  }
}

void print_game_item(unsigned char page, unsigned char line, unsigned char selected) 
{
    print(
        (char*)&GAMEINFO[4 + GAMEINFO_SIZE * (page * GAMES_PER_PAGE + line - 1)], 
        CHAR_ON_LINE,
        1,
        line,
        selected
    );
}

void draw_games_list(unsigned char page, unsigned char cursor) 
{
    unsigned char start_game = page * GAMES_PER_PAGE;
    unsigned char end_game = start_game + GAMES_PER_PAGE;
    unsigned char i = start_game;
    clear_screen();
    if (end_game > GAMECOUNT) 
    {
        end_game = GAMECOUNT;
    }

    for (i; i < end_game; i++) 
    {
        unsigned char line = i - start_game + 1;
        print_game_item(page, line, line == cursor);
    }
}

void handle_gb(void)
{
    char reset;
    char control;
    clear_screen();
    
    SV_DMA.start = 0xDE00;
    SV_DMA.size = 23;
    SV_DMA.control = 0b00001100;
    SV_DMA.on = 0x80;

    while (1)
    {
        SV_LCD.xpos = 0;
        SV_LCD.ypos = 0;
        SV_DMA_BUFFER.source = 0xC000;
        SV_DMA_BUFFER.destination = (int)SV_VIDEO + 0x180;
        SV_DMA_BUFFER.lenght = 0xD8;
        SV_DMA_BUFFER.control = 0x80;

        SV_DMA_BUFFER.source = 0xCD80;
        SV_DMA_BUFFER.destination = (int)SV_VIDEO + 0xF00;
        SV_DMA_BUFFER.lenght = 0xD8;
        SV_DMA_BUFFER.control = 0x80;

        if(SV_IRQ_STATUS & DMA_AUDIO_SYSTEM_MASK) {
            control = (*(unsigned char *)(0xEF00 + SV_CONTROL));
            reset = SV_RESET_DMA_IRQ;
            SV_DMA.start = 0xDE00;
            SV_DMA.size = 23;
            SV_DMA.on = 0x80;

        }
    }
}

void main(void)
{
    unsigned char cursor = 1;
    unsigned char page = 0;
    unsigned char total_pages = (GAMECOUNT + GAMES_PER_PAGE - 1) / GAMES_PER_PAGE;
    unsigned char lastPageGameCount = GAMECOUNT % GAMES_PER_PAGE;
    char control;
    
    init();
    clear_screen();
    draw_games_list(page, cursor);

    while (1) 
    {
        if ((~SV_CONTROL & JOY_DOWN_MASK) && (cursor < GAMES_PER_PAGE)) 
        {
            if(page == total_pages - 1 && cursor == lastPageGameCount) continue;
            print_game_item(page, cursor, 0);
            cursor++;
            print_game_item(page, cursor, 1);
            beep();
            while(~SV_CONTROL & JOY_DOWN_MASK){};
        }
        else if ((~SV_CONTROL & JOY_UP_MASK) && (cursor > 1)) 
        {
            print_game_item(page, cursor, 0);
            cursor--;
            print_game_item(page, cursor, 1);
            beep();
            while(~SV_CONTROL & JOY_UP_MASK){};
        }
        else if ((~SV_CONTROL & JOY_LEFT_MASK) && (page > 0)) 
        {
            page--;
            cursor = 1;
            draw_games_list(page, cursor);
            beep();
            while(~SV_CONTROL & JOY_LEFT_MASK){};
        }
        else if ((~SV_CONTROL & JOY_RIGHT_MASK) && (page < total_pages - 1)) 
        {  
            page++;
            cursor = 1;
            draw_games_list(page, cursor);
            beep();
            while(~SV_CONTROL & JOY_RIGHT_MASK){};
        }
        else if (~SV_CONTROL & JOY_START_MASK) 
        {
            unsigned char game_index = page * GAMES_PER_PAGE + cursor;
            control = (*(unsigned char*)(0xEF00 + game_index));
            handle_gb();
        }
    }
}
