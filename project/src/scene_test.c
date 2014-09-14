#include "scene_test.h"

void SCENE_Test()
{
    int cell_size = 16;
    int width = 16;
    int height = screenHeight/cell_size;
    s16* offsets = (s16*)MEM_alloc(height*width*sizeof(s16));
    
    s16* scanline = offsets;
    int ty;
    
    vu16 *pw = (u16*)GFX_DATA_PORT;
    vu32 *pl = (u32*)GFX_CTRL_PORT;
    vu32 addr = GFX_WRITE_VSRAM_ADDR(0);
    
    void vblank()
    {
        scanline = offsets;
    }
    
    void hblank()
    {
        *pl = addr;
        
        int i=width;
        while(i--) *pw = *scanline++;
    }

    int x;
    int y;
    for (y=0 ; y<height ; ++y)
    for (x=0 ; x<width ; ++x)
    {
        int tx = 0;
        int ty = (y*cell_size)%32;
        
        offsets[y*width+x] = -y*cell_size+ty;
    }
    
    smeSCREEN_SetScrollMode(smeSCREEN_HSCROLL_Plane, smeSCREEN_VSCROLL_Column);
    
    
    //VDP_setPaletteColors(1, (u16*)texmap_test.palette->data, 16);
    u16 w = 0; //texmap_test.w;
    u16 h = 0; //texmap_test.h;
    //VDP_loadBMPTileData((u32*)texmap_test.image, 1, w/8, h/8, w/8);
    for (y=0 ; y<VDP_getPlanHeight()/(h/8); ++y)
    for (x=0 ; x<VDP_getPlanWidth()/(w/8); ++x)
        VDP_fillTileMapRectInc(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 1), x*(w/8), y*(h/8), w/8, h/8);
    
    SYS_setVIntCallback(vblank);
    SYS_pushHIntCallback(hblank);
    VDP_setHIntCounter(cell_size-1);
    VDP_setHInterrupt(1); 
    VDP_setAutoInc(4);
        
    do
    {
    }
    while(1);
    
    SYS_popHIntCallback();
    SYS_setVIntCallback(0);
    SYS_setHIntCallback(0);
    
    MEM_free(offsets);
}
 

/*
    int linePos[16][32];
    
    int* dTable;
    int* sizeTable;
    int* sizeTableD;
    
    u16 curY;
    
    void initTable()
    {
        int h, k;
     
        h = 15;
        int w = 128;
        k = 0;
        while(w>=100)
        {
            int height = w*32/128;
            int y;
            for (y=0 ; y<32 ; ++y)
            {
                linePos[h][y] = k+y*height/32;
            }
            k += height;
            w -= 2;
            --h;
        }
     
        dTable = (int*)MEM_alloc(screenHeight*sizeof(int));
        sizeTable = (int*)MEM_alloc(screenHeight*sizeof(int));
        sizeTableD = (int*)MEM_alloc(screenHeight*sizeof(int));
        
        
        VDP_setPaletteColors(16, (u16*)cubetext.palette->data, 16);
    }

    void updateTable()
    {
        ++ttt;
        
        smeMESH_SetTranslation(mesh, FIX16(0.0f), sinFix16(FIX16(ttt/4.0f))*40, FIX16(160.0f));
        smeMESH_SetRotation(mesh, FIX16(ttt/2.0f), FIX16(0.0f), FIX16(0.0f));
        mesh->Transform.rebuildMat = 1;
        M3D_transform(&mesh->Transform, mesh->Vertexes, smeSCENE.VertexesWS, mesh->VertexCount);
        M3D_project_s16(smeSCENE.VertexesWS, smeSCENE.VertexesVS, mesh->VertexCount);
        
        int k;
        
        minZ = 9999;
        maxZ = -9999;
        for (k=0 ; k<mesh->VertexCount ; ++k)
        {
            if (fix16ToInt(smeSCENE.VertexesWS[k].z)<minZ) minZ = fix16ToInt(smeSCENE.VertexesWS[k].z);
            if (fix16ToInt(smeSCENE.VertexesWS[k].z)>maxZ) maxZ = fix16ToInt(smeSCENE.VertexesWS[k].z);
        }
        if (maxZ==minZ)
            return;
        
        for (k=0 ; k<screenHeight ; ++k)
        {
            dTable[k] = 0;
            sizeTable[k] = -5-k;
        }
        
        int n;
        for (n=0 ; n<mesh->VertexCount ; ++n)
        {
            if ((n+1)%4==0) drawPoly(n, n-3);
            else drawPoly(n, n+1);
        }
        
        memcpy(sizeTableD, sizeTable, screenHeight*sizeof(int));
    }
    
    void vblank()
    {
        curY = 0;
    } 

    void hblank()
    {
        int p = sizeTableD[curY];
        
        vu16 *pw = (u16*)GFX_DATA_PORT;
        vu32 *pl = (u32*)GFX_CTRL_PORT;    
        u16 addr = (0&0x1F)*4;
        *pl = GFX_WRITE_VSRAM_ADDR(addr);
        *pw = p;
        curY++;
    }
    
    smeSCREEN_SetScrollMode(smeSCREEN_HSCROLL_Plane, smeSCREEN_VSCROLL_Column);
    
    initTable();

    int xs=0, ys=0;
    
    u16 w = cubetext.w;
    u16 h = cubetext.h;
    VDP_loadBMPTileData((u32*)cubetext.image, 1, w/8, h/8, w/8);
    for (ys=0 ; ys<VDP_getPlanHeight()/(h/8); ++ys)
        VDP_fillTileMapRectInc(APLAN, TILE_ATTR_FULL(PAL1, 0, 0, 0, 1), 8, ys*(h/8), w / 8, h / 8);
    
    SYS_setVIntCallback(vblank);
    SYS_pushHIntCallback(hblank);
    VDP_setHIntCounter(0);
    VDP_setHInterrupt(1); 
    
    curY = 0;
    
    do
    {
        updateTable();
    }
    while(1);
    
    SYS_popHIntCallback();
    SYS_setVIntCallback(0);
    SYS_setHIntCallback(0);
    
    smeSCENE_Remove(mesh_id);
    smeMESH_Destroy(mesh);
}
*/