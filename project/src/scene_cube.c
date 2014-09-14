#include "scene_cube.h"

void SCENE_Cube()
{
    smeFRAMEBUFFER_Begin();
    
    smeMesh* cube = smeMESH_CreateCube();
    cube->Material = smeMESH_Material_TexMap; //Flat; //Gouraud;
    //cube->TexMap = cube_texmap.image;
    u16 cube_id = smeSCENE_Add(cube);
    
    float roll = 0.0f;
    
    int c;
    for (c=0 ; c<16 ; ++c)
    {
        VDP_setPaletteColor(c, c);
    }
    
    //VDP_setPaletteColors(1, (u16*)cube_texmap.palette->data, 16);
    //VDP_setPaletteColors(16, (u16*)cube_texmap.palette->data, 16);
    
    int ys;
    u16 w = 0; //cube_texmap.w;
    u16 h = 0; //cube_texmap.h;
    //VDP_loadBMPTileData((u32*)cube_texmap.image, BMP_FB1ENDTILEINDEX, w/8, h/8, w/8);
    for (ys=0 ; ys<VDP_getPlanHeight()/(h/8); ++ys)
        VDP_fillTileMapRectInc(BPLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, BMP_FB1ENDTILEINDEX), 8, ys*(h/8), w / 8, h / 8);
    
    while (1)
    {        
        smeMESH_SetRotation(cube, FIX16(roll*4.0f), FIX16(roll), FIX16(roll*2.0f));
        roll+=0.1f;
        
        smeMESH_SetTranslation(cube, FIX16(0.0f), FIX16(0.0f), FIX16(150));
        smeSCENE_Update();
        smeFRAMEBUFFER_Update();
    }
    
    smeSCENE_Remove(cube_id);
    smeMESH_Destroy(cube);
    
    smeFRAMEBUFFER_End();
}
