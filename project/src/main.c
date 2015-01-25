#include "sme.h"
#include "resources.h"
#include "scene_barrel.h"
#include "scene_cube.h"
#include "scene_island.h"
#include "scene_test.h"
#include "scene_bmp.h"

#include "data.h"

#define MOD(a, b)  ((a) % (b) + (b)) % (b)

int VIEWPORT_WIDTH;
int VIEWPORT_HEIGHT;
int VIEWPORT_BORDER;
int PLAN_WIDTH;
int PLAN_HEIGHT;
int MAP_WIDTH;
int MAP_HEIGHT;

int ANIM_ANGLES = 12;

Map Plan_a;
Map Plan_b;

Sprite sprites[2];

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
    
    ms[0] = mx;
    ps[0] = px;
    ms[1] = breaks[0];
    ps[1] = breaks[0];
    
    for (i=1 ; i<breaks_count ; ++i)
    {
        ms[i*2] = MOD(mx+breaks[i-1], m_max);
        ps[i*2] = MOD(px+breaks[i-1], p_max);
        ms[i*2+1] = breaks[i]-breaks[i-1];
        ps[i*2+1] = breaks[i]-breaks[i-1];
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
            VDP_setMapEx(VDP_PLAN_A, &Plan_a, 0, pxs[i], pys[j], mxs[i], mys[j], ws, hs); 
            VDP_setMapEx(VDP_PLAN_B, &Plan_b, 0, pxs[i], pys[j], mxs[i], mys[j], ws, hs); 
        }
    }
}

float position_x = 0.0f;
float position_y = 0.0f;
float roll = 0.0f;
float camera_position_x = 0.0f;
float camera_position_y = 0.0f;

void vblank()
{
    VDP_setHorizontalScroll(PLAN_A, -(int)(camera_position_x-screenWidth/2));
    VDP_setHorizontalScroll(PLAN_B, -(int)(camera_position_x-screenWidth/2));
    VDP_setVerticalScroll(PLAN_A, (int)(camera_position_y-screenHeight/2));
    VDP_setVerticalScroll(PLAN_B, (int)(camera_position_y-screenHeight/2));      
}

float cos(float a)
{
    int v = (int)(a*512.0f/PI);
    if (v<0) v += 1024;
    return ((int)cosFix32(v))/1024.0f;
}

float sin(float a)
{
    int v = (int)(a*512.0f/PI);
    if (v<0) v += 1024;
    return ((int)sinFix32(v))/1024.0f;
}

int main(u16 hard)
{
    sme_Init(hard);
    SND_startPlay_PCM(music, sizeof(music), SOUND_RATE_16000, SOUND_PAN_CENTER, 1);    
    
    VIEWPORT_BORDER = 1;
    VIEWPORT_WIDTH = screenWidth/8+VIEWPORT_BORDER*2;
    VIEWPORT_HEIGHT = screenHeight/8+VIEWPORT_BORDER*2;
    PLAN_WIDTH = VDP_getPlanWidth();
    PLAN_HEIGHT = VDP_getPlanHeight();
    MAP_WIDTH = 252;
    MAP_HEIGHT = 252;
    
    SPR_init(256);
    SPR_initSprite(&sprites[0], &camion, 0, 0, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    VDP_setPalette(1, camion.palette->data);
    
    VDP_setPalette(0, palettepict.palette->data);
    VDP_loadTileSet(&test_tiles, TILE_USERINDEX, 0);
    
    Plan_a.compression = 0;
    Plan_a.w = MAP_WIDTH;
    Plan_a.h = MAP_HEIGHT;
    Plan_a.tilemap = map_plan_a;
    
    Plan_b.compression = 0;
    Plan_b.w = MAP_WIDTH;
    Plan_b.h = MAP_HEIGHT;
    Plan_b.tilemap = map_plan_b;
    
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    
    camera_position_x = position_x;
    camera_position_y = position_y;
    
    update((int)(camera_position_x/8-screenWidth/16-VIEWPORT_BORDER), (int)(camera_position_y/8-screenHeight/16-VIEWPORT_BORDER), VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    SYS_setVIntCallback(vblank);
    
    while (1)
    {
        u16 value = JOY_readJoypad(0);
        if (value&BUTTON_LEFT) roll -= 0.02f;
        if (value&BUTTON_RIGHT) roll += 0.02f;        
        if (roll<-PI) roll += 2.0f*PI;
        if (roll>PI) roll -= 2.0f*PI;
        float speed = 0.0f;
        if (value&BUTTON_UP) speed += 1.5f;
        if (value&BUTTON_DOWN) speed -= 1.5f;     
        
        float vec_x = sin(roll)*speed;
        float vec_y = -cos(roll)*speed;
        
        
        int censor = map_plan_a[MOD((int)((position_y+vec_y)/8.0f), MAP_HEIGHT)*MAP_WIDTH+MOD((int)((position_x+vec_x)/8.0f), MAP_WIDTH)];
        if (censor==0)
        {
            vec_x = 0.0f;
            vec_y = 0.0f;
        }
        
        position_x += vec_x;
        position_y += vec_y;
        
        int sp = (int)(roll*ANIM_ANGLES/PI);
        if (sp<0) { SPR_setAttribut(&sprites[0], TILE_ATTR(PAL1, TRUE, FALSE, TRUE)); sp = -sp; }
        else SPR_setAttribut(&sprites[0], TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
        SPR_setAnim(&sprites[0], sp);
        
        
        int old_place_x = (int)(camera_position_x/8);
        int old_place_y = (int)(camera_position_y/8);
        
        if (speed<0.0f) speed = -speed;
        float smooth = 10.0f/(speed/2.0f+0.5f);
        camera_position_x = (camera_position_x*smooth+(position_x+vec_x*20.0f))/(smooth+1.0f);
        camera_position_y = (camera_position_y*smooth+(position_y+vec_y*20.0f))/(smooth+1.0f);
        int place_x = (int)(camera_position_x/8);
        int place_y = (int)(camera_position_y/8);
                
        SPR_setPosition(&sprites[0], position_x-camera_position_x-16+screenWidth/2, position_y-camera_position_y-16+screenHeight/2);
        SPR_update(sprites, 1);
        
        int px = place_x-screenWidth/16-VIEWPORT_BORDER;
        int py = place_y-screenHeight/16-VIEWPORT_BORDER;
        
        if (place_x>old_place_x)
            update(px+VIEWPORT_WIDTH-1, py, 1, VIEWPORT_HEIGHT);
        else if (place_x<old_place_x)
            update(px, py, 1, VIEWPORT_HEIGHT);
        
        if (place_y>old_place_y)
            update(px, py+VIEWPORT_HEIGHT-1, VIEWPORT_WIDTH, 1);
        else if (place_y<old_place_y)
            update(px, py, VIEWPORT_WIDTH, 1);
    }
    
    sme_Exit();    
    return 0;
}
