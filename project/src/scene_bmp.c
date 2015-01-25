#include "scene_bmp.h"
//#include "data.h"

#define WIDTH 42
#define HEIGHT 94

void SCENE_Bmp()
{
/*
    int ys=0;
    u16 w = WIDTH*4;
    u16 h = HEIGHT;
    int i=0;
    
    for (ys=0 ; ys<1; ++ys)
        VDP_fillTileMapRectInc(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 1), 9, ys*(h/8)+8, w/8, h/8);
    
    u8* buffer = (u8*)MEM_alloc(16*2+WIDTH*HEIGHT*2);
    
    int p=0;
    
    do
    {
        if (Frames[i]==NULL)
        {
        }
        else
        {
            DecompressSlz(buffer, Frames[i]);
            u16* palette = (u16*)buffer;
            u32* data = (u32*)(buffer+16*2);            
            VDP_waitVSync();
            VDP_loadBMPTileData(data, 1, w/8, h/8, w/8);
            VDP_setPaletteColors(1, palette, 8);
        }
        i = (i+1)%FrameCount;
    }
    while(1); */   
}
