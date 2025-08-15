#include <supervision.h>
#include <string.h>
#include <stdio.h>

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

void main(void)
{
    char reset;
    char control;

    init();
    memset(SV_VIDEO, 0, 0x8000);

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
