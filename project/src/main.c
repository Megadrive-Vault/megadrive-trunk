#include "sme.h"
#include "resources.h"
#include "scene_barrel.h"
#include "scene_cube.h"
#include "scene_island.h"
#include "scene_test.h"
#include "scene_bmp.h"

#include "data.h"

#define MOD(a, b)  (a % b + b) % b

int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int BORDER;
int PLAN_WIDTH;
int PLAN_HEIGHT;
int MAP_WIDTH;
int MAP_HEIGHT;

Map Plan_a;
Map Plan_b;

void slice(int* ms, int* ps, int* count, int mx, int px, int s, int m_max, int p_max)
{
    // check breaks with each maximum
    int breaks[3];
    int breaks_count = 0;
    
    {
        // check map maximum
        int out = mx+s-m_max;
        if (out>0)
        {
            breaks[0] = s-out;
            ++breaks_count;
        }
    
        // check plan maximum
        out = px+s-p_max;
        if (out>0)
        {
            int b = s-out;
            if (breaks_count==0 || breaks[0]<b)
            {
                breaks[breaks_count] = b;
                ++breaks_count;
            }
            else if (breaks[0]!=b)
            {
                int t = breaks[0];
                breaks[0] = b;
                breaks[1] = t;
                ++breaks_count;
            }        
        }
    
        // add final break if required
        if (breaks_count==0 || breaks[breaks_count-1]!=s)
        {
            breaks[breaks_count] = s;
            ++breaks_count;
        }
    }
    
    // create drawing points
    int i;
    *count = breaks_count*2;
    
    for (i=0 ; i<breaks_count ; ++i)
    {
        if (i==0)
        {
            ms[0] = mx;
            ps[0] = px;
            ms[1] = breaks[0];
            ps[1] = breaks[0];
        }
        else
        {
            ms[i*2] = MOD(mx+breaks[i-1], m_max);
            ps[i*2] = MOD(px+breaks[i-1], p_max);
            ms[i*2+1] = breaks[i]-breaks[i-1];
            ps[i*2+1] = breaks[i]-breaks[i-1];
        }
    }
}

void update(int x, int y, int w, int h)
{
    int mx = MOD(x, MAP_WIDTH);
    int my = MOD(y, MAP_HEIGHT);
    int px = MOD(x, PLAN_WIDTH);
    int py = MOD(y, PLAN_HEIGHT);
    
    int mxs[6];
    int pxs[6];    
    int nx = 0;
    slice(mxs, pxs, &nx, mx, px, w, MAP_WIDTH, PLAN_WIDTH);
    
    int mys[6];
    int pys[6];    
    int ny = 0;
    slice(mys, pys, &ny, my, py, h, MAP_HEIGHT, PLAN_HEIGHT);
    
    int i, j;
    for (j=0 ; j<ny ; j+=2)
    {
        int hs = pys[j+1];
        for (i=0 ; i<nx ; i+=2)
        {
            int ws = pxs[i+1];
            /*
            if (pxs[i]<0 || pxs[i]+ws>PLAN_WIDTH
                || pys[j]<0 || pys[j]+hs>PLAN_HEIGHT
                || mxs[i]<0 || mxs[i]+ws>MAP_WIDTH
                || mys[j]<0 || mys[j]+hs>MAP_HEIGHT
                || ws<0 || hs<0)
            {
                while(1){};
            }
            */
            VDP_setMapEx(VDP_PLAN_A, &Plan_a, 0, pxs[i], pys[j], mxs[i], mys[j], ws, hs); 
            VDP_setMapEx(VDP_PLAN_B, &Plan_b, 0, pxs[i], pys[j], mxs[i], mys[j], ws, hs); 
        }
    }
}

int main(u16 hard)
{
    sme_Init(hard);
    SND_startPlay_PCM(music, sizeof(music), SOUND_RATE_16000, SOUND_PAN_CENTER, 1);    
    
    //SCENE_Bmp();
    
    BORDER = 2;
    
    SCREEN_WIDTH = screenWidth/8+BORDER*2;
    SCREEN_HEIGHT = screenHeight/8+BORDER*2;
    PLAN_WIDTH = VDP_getPlanWidth();
    PLAN_HEIGHT = VDP_getPlanHeight();
    MAP_WIDTH = 100;
    MAP_HEIGHT = 100;
    
    VDP_setPalette(0, palettepict.palette->data);
    VDP_loadTileSet(&test_tiles, TILE_USERINDEX, 0);
    
    Plan_a.compression = 0;
    Plan_a.w = MAP_WIDTH;
    Plan_a.h = MAP_HEIGHT;
    Plan_a.tilemap = (u16*)MEM_alloc(MAP_WIDTH*MAP_HEIGHT*sizeof(u16));
    
    Plan_b.compression = 0;
    Plan_b.w = MAP_WIDTH;
    Plan_b.h = MAP_HEIGHT;
    Plan_b.tilemap = (u16*)MEM_alloc(MAP_WIDTH*MAP_HEIGHT*sizeof(u16));
    
    int i;
    for (i=0 ; i<MAP_WIDTH*MAP_HEIGHT ; ++i)
    {
        Plan_a.tilemap[i] = map_plan_a[i]==-1?0:TILE_USERINDEX+map_plan_a[i];
        Plan_b.tilemap[i] = map_plan_b[i]==-1?0:TILE_USERINDEX+map_plan_b[i];
    }
    
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    
    int position_x = 0;
    int position_y = 0;
    update(position_x/8, position_y/8, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    while (1)
    {
        int old_place_x = position_x/8;
        int old_place_y = position_y/8;
    
        u16 value = JOY_readJoypad(0);
        int move_x = 0;
        int move_y = 0;
        if (value&BUTTON_UP) --move_y;
        if (value&BUTTON_DOWN) ++move_y;
        if (value&BUTTON_LEFT) --move_x;
        if (value&BUTTON_RIGHT) ++move_x;
        position_x += move_x*2;
        position_y += move_y*2;
        
        int place_x = position_x/8;
        int place_y = position_y/8;
    
        VDP_waitVSync();
        
        if (place_x>old_place_x)
            update(place_x+SCREEN_WIDTH-1, place_y, 1, SCREEN_HEIGHT);
        else if (place_x<old_place_x)
            update(place_x, place_y, 1, SCREEN_HEIGHT);
        
        if (place_y>old_place_y)
            update(place_x, place_y+SCREEN_HEIGHT-1, SCREEN_WIDTH, 1);
        else if (place_y<old_place_y)
            update(place_x, place_y, SCREEN_WIDTH, 1);
        
        VDP_setHorizontalScroll(PLAN_A, -position_x-BORDER*8);
        VDP_setHorizontalScroll(PLAN_B, -position_x-BORDER*8);
        VDP_setVerticalScroll(PLAN_A, position_y+BORDER*8);
        VDP_setVerticalScroll(PLAN_B, position_y+BORDER*8);        
    }
    
    sme_Exit();    
    return 0;
}
