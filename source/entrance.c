#include <gccore.h>
#include <wiiuse/wpad.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "things.h"
#include "seal_txt.h"
#include "seal_tpl.h"
#include "entrance.h"


f32 objectYRot = 0.0f;
static int dummy_render() {
    return 0;
}

SECTOR sealObj;
extern GXTexObj sealTexture;

int entrance() {
    objectYRot += 0.5f;
    if(objectYRot >= 360.0f) objectYRot -= 360.0f;
    DrawThing(view, sealTexture, &sealObj, 0.0f, 0.1f, -1.0f, objectYRot);
    return 0;
}
int entrance_load() {
    TPL_OpenTPLFromMemory(&sealTPL, (void *)seal_tpl,seal_tpl_size);
    TPL_GetTexture(&sealTPL,0,&sealTexture);
    TPL_CloseTPLFile(&sealTPL);
    SetupThing(seal_txt, seal_txt_size, &sealObj);
    return 0;
}
int entrance_collision() {
    if(CheckObjectCollision(xpos, zpos, 0.0f, -1.0f, 0.2f)) {
        f32 pushBack = 0.05f;
        f32 dx = 0.0f - xpos;
        f32 dz = -1.0f - zpos;
        f32 dist = sqrtf(dx*dx + dz*dz);
        if(dist > 0) {
            xpos -= (dx/dist) * pushBack;
            zpos -= (dz/dist) * pushBack;
        }
    }
    return 0;
}
int entrance_free() {
    FreeThing(sealObj);
    level_render = &dummy_render;
    level_interact = &dummy_render;
    level_collide = &dummy_render;
    return 0;
}
int entrance_interact() {
    if (CheckObjectCollision(xpos, zpos, 0.0f, -1.0f, 0.5f)) {
        interaction = SEVENDAYS;
        return 60;
    } else if (CheckObjectCollision(xpos, zpos, 0.0f, 1.8f, 0.2f)) {
        interaction = NOTURNINGBACK;
        entrance_free();
        resetPlayer();
        return 60;
    } else if (CheckObjectCollision(xpos, zpos, 0.0f, -1.8f, 0.2f)) {
        interaction = LOCKED;
        return 60;
    }
    return 0;
}
