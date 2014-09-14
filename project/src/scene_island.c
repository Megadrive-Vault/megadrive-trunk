#include "scene_island.h"

void SCENE_Island()
{
    smeFRAMEBUFFER_Begin();
    
    smeMesh* island = smeMESH_Create();
    const u16 size = 6;
    island->VertexCount = size*size;
    island->Vertexes = (Vect3D_f16*)MEM_alloc(island->VertexCount*sizeof(Vect3D_f16));
    island->PolygonCount = (size-1)*(size-1)*2;
    island->Polygons = (u16*)MEM_alloc(island->PolygonCount*3*sizeof(u16));
    
    u16 i,j;
    u16 fi=0;
    for (j=0 ; j<size ; ++j)
    {
        for (i=0 ; i<size ; ++i)
        {
            island->Vertexes[j*size+i].x = intToFix16(i-(size/2))*3;
            island->Vertexes[j*size+i].y = 0;
            island->Vertexes[j*size+i].z = intToFix16(j-(size/2))*3;
            
            if (i<size-1 && j<size-1)
            {
                island->Polygons[fi++] = j*size+i;
                island->Polygons[fi++] = j*size+i+1;
                island->Polygons[fi++] = j*size+i+size+1;
                island->Polygons[fi++] = j*size+i;
                island->Polygons[fi++] = j*size+i+size+1;
                island->Polygons[fi++] = j*size+i+size;                
            }
        }
    }
        
    island->Material = smeMESH_Material_Flat; //Flat; //Gouraud;
    
    u16 island_id = smeSCENE_Add(island);
    
    float roll_y = 0.0f;
    
    int c;
    for (c=0 ; c<8 ; ++c)
    {
        VDP_setPaletteColor(c, c*2);
        VDP_setPaletteColor(c+8, (c*2)<<4);
    }
    
    while (1)
    {
        
        for (j=0 ; j<size ; ++j)
        {
            for (i=0 ; i<size ; ++i)
            {
                int h;
                if (j<size-1) h = fix16ToInt(island->Vertexes[(j+1)*size+i].y)*20;
                else
                {
                    if (i>0 && i<size-1) h = (random()%200+fix16ToInt(island->Vertexes[(j-1)*size+i].y)*16)/2;
                    else h = 0;
                }
                
                island->Vertexes[j*size+i].y = intToFix16(h/20);
            }
        }
        
        smeMESH_SetRotation(island, FIX16(-0.4f), FIX16(roll_y), FIX16(-0.4f));
        roll_y+=0.2f;
        
        smeMESH_SetTranslation(island, FIX16(0.0f), FIX16(0.0f), FIX16(22));
        smeSCENE_Update();
        smeFRAMEBUFFER_Update();
    }
    
    smeSCENE_Remove(island_id);
    smeMESH_Destroy(island);
    
    smeFRAMEBUFFER_End();
}
