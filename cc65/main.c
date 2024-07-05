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
    while (1)
    {
        SV_LCD.xpos = 0;
        SV_LCD.ypos = 0;
        SV_DMA_BUFFER.source = 0xC000;
        SV_DMA_BUFFER.destination = (int)SV_VIDEO;
        SV_DMA_BUFFER.lenght = 0xD8;
        SV_DMA_BUFFER.control = 0x80;

        SV_DMA_BUFFER.source = 0xCD80;
        SV_DMA_BUFFER.destination = (int)SV_VIDEO + 0xD80;
        SV_DMA_BUFFER.lenght = 0xD8;
        SV_DMA_BUFFER.control = 0x80;

        if (~SV_CONTROL)
        {
            SV_DMA_BUFFER.source = 0xA000 | (SV_CONTROL << 4);
            SV_DMA_BUFFER.destination = (int)SV_VIDEO + 0x1C00;
            SV_DMA_BUFFER.lenght = 0x1;
            SV_DMA_BUFFER.control = 0x80;
        }
    }
}
