#include "sme.h"
#include "resources.h"
#include "scene_barrel.h"
#include "scene_cube.h"
#include "scene_island.h"
#include "scene_test.h"
#include "scene_bmp.h"

int main(u16 hard)
{
    sme_Init(hard);
    SND_startPlay_PCM(music, sizeof(music), SOUND_RATE_16000, SOUND_PAN_CENTER, 1);    
    
    SCENE_Bmp();
    
    sme_Exit();    
    return 0;
}
