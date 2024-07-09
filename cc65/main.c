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

static void init(void)
{
    SV_LCD.width = 160;
    SV_LCD.height = 160;
}

void main(void)
{
    init();
    memset(SV_VIDEO, 0, 0x8000);

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

        SV_DMA.start = 0xC000 + 0x1E00;
        SV_DMA.size = 0x55;
        SV_DMA.control = 0b00001100;
        SV_DMA.on = 0x80;
    }
}
