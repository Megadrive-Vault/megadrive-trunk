#include "scene_barrel_test.h"

void SCENE_BarrelTest()
{
    int linePos[16][32];
    
    int* dTable;
    int* sizeTable;
    int* sizeTableD;
    
    u16 curY;
    u16 ttt=0;
    
    smeMesh* mesh;
    u16 mesh_id;
    
    u16 paletteFade[16][32];
    
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
        
        M3D_reset();
        M3D_setViewport(screenWidth, screenHeight);
    
        mesh = smeMESH_Create();
        mesh->VertexCount = 8;
        mesh->Vertexes = (Vect3D_f16*)MEM_alloc(mesh->VertexCount*sizeof(Vect3D_f16));
        mesh->Vertexes[0].x = intToFix16(0); mesh->Vertexes[0].y = intToFix16(-14); mesh->Vertexes[0].z = intToFix16(-14);
        mesh->Vertexes[1].x = intToFix16(0); mesh->Vertexes[1].y = intToFix16(-14); mesh->Vertexes[1].z = intToFix16(14);
        mesh->Vertexes[2].x = intToFix16(0); mesh->Vertexes[2].y = intToFix16(14); mesh->Vertexes[2].z = intToFix16(14);
        mesh->Vertexes[3].x = intToFix16(0); mesh->Vertexes[3].y = intToFix16(14); mesh->Vertexes[3].z = intToFix16(-14);
        
        mesh->Vertexes[4].x = intToFix16(0); mesh->Vertexes[4].y = intToFix16(20); mesh->Vertexes[4].z = intToFix16(20);
        mesh->Vertexes[5].x = intToFix16(0); mesh->Vertexes[5].y = intToFix16(20); mesh->Vertexes[5].z = intToFix16(38);
        mesh->Vertexes[6].x = intToFix16(0); mesh->Vertexes[6].y = intToFix16(48); mesh->Vertexes[6].z = intToFix16(38);
        mesh->Vertexes[7].x = intToFix16(0); mesh->Vertexes[7].y = intToFix16(48); mesh->Vertexes[7].z = intToFix16(20);
        
        mesh->Vertexes[8].x = intToFix16(0); mesh->Vertexes[8].y = intToFix16(-46); mesh->Vertexes[8].z = intToFix16(-24);
        mesh->Vertexes[9].x = intToFix16(0); mesh->Vertexes[9].y = intToFix16(-46); mesh->Vertexes[9].z = intToFix16(4);
        mesh->Vertexes[10].x = intToFix16(0); mesh->Vertexes[10].y = intToFix16(-18); mesh->Vertexes[10].z = intToFix16(4);
        mesh->Vertexes[11].x = intToFix16(0); mesh->Vertexes[11].y = intToFix16(-18); mesh->Vertexes[11].z = intToFix16(-24);
        
        mesh->Vertexes[12].x = intToFix16(0); mesh->Vertexes[12].y = intToFix16(-18); mesh->Vertexes[12].z = intToFix16(18);
        mesh->Vertexes[13].x = intToFix16(0); mesh->Vertexes[13].y = intToFix16(-18); mesh->Vertexes[13].z = intToFix16(36);
        mesh->Vertexes[14].x = intToFix16(0); mesh->Vertexes[14].y = intToFix16(10); mesh->Vertexes[14].z = intToFix16(36);
        mesh->Vertexes[15].x = intToFix16(0); mesh->Vertexes[15].y = intToFix16(10); mesh->Vertexes[15].z = intToFix16(18);
       
       
        mesh_id = smeSCENE_Add(mesh);
    
        for (h=0 ; h<16 ; ++h)
        {
            for (k=0 ; k<16 ; ++k)
            {
                int r = 0; //((u16*)cubetext.palette->data)[k]&15;
                int g = 0; //(((u16*)cubetext.palette->data)[k]>>4)&15;
                int b = 0; //(((u16*)cubetext.palette->data)[k]>>8)&15;
                r -= (15-h); if (r<0) r=0;
                g -= (15-h); if (g<0) g=0;
                b -= (15-h); if (b<0) b=0;
                paletteFade[h][k] = r|(g<<4)|(b<<8);
            }
        }
        
        //VDP_setPaletteColors(16, (u16*)cubetext.palette->data, 16);
    }

    int minZ;
    int maxZ;
        
    inline void drawPoly(int a, int b)
    {
        int da = fix16ToInt(smeSCENE.VertexesWS[a].z);
        int db = fix16ToInt(smeSCENE.VertexesWS[b].z);
        int ay = smeSCENE.VertexesVS[a].y;
        int by = smeSCENE.VertexesVS[b].y;
        int y;
        if (by-ay!=0)
        for (y=ay ; y<=by ; ++y)
        {
            int d = da+((db-da)*(y-ay))/(by-ay);
            d = 15-((d-minZ)*15)/(maxZ-minZ);
            
            if (y>=0 && y<screenHeight && d>dTable[y])
            {
                dTable[y] = d;
                sizeTable[y] = linePos[d][((y-ay)*31)/(by-ay)]-y;
            }
        }
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
        /*
        vu16 *pw = (u16 *) GFX_DATA_PORT;
        vu32 *pl = (u32 *) GFX_CTRL_PORT;    
        *pl = GFX_WRITE_CRAM_ADDR((PAL1 * 32) + (15 * 2));
        *pw = paletteFade[15][15];        */
    } 

    void hblank()
    {
        int p = sizeTableD[curY];
        
        vu16 *pw = (u16*)GFX_DATA_PORT;
        vu32 *pl = (u32*)GFX_CTRL_PORT;    

        //*pl = GFX_WRITE_CRAM_ADDR((PAL1 * 32) + (15 * 2));
        //*pw = paletteFade[f][15];        

        u16 addr = (0&0x1F)*4;
        *pl = GFX_WRITE_VSRAM_ADDR(addr);
        *pw = p;
        curY++;
    }
    
    smeSCREEN_SetScrollMode(smeSCREEN_HSCROLL_Plane, smeSCREEN_VSCROLL_Plane);
    
    initTable();

    int xs=0, ys=0;
    
    u16 w = 0; //cubetext.w;
    u16 h = 0; //cubetext.h;
    //VDP_loadBMPTileData((u32*)cubetext.image, 1, w/8, h/8, w/8);
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
