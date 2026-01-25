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

int entrance() {
    // HACK: we use 8 vertices so i can split the texture in half and then switch the position of the two halves
    // also we use 0'd out normals because world.txt doesn't have normals
    GX_LoadTexObj(&texture, GX_TEXMAP0);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    GX_Position3f32(2.0f, 0.0f, -0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);
    GX_Position3f32(2.0f, 0.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 1.0f);
    GX_Position3f32(2.0f, 1.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 0.0f);
    GX_Position3f32(2.0f, 1.0f, -0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);
    GX_Position3f32(2.0f, 0.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 1.0f);
    GX_Position3f32(2.0f, 0.0f, 0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);
    GX_Position3f32(2.0f, 1.0f, 0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);
    GX_Position3f32(2.0f, 1.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 0.0f);
    GX_End();
    // quick explanation of deltatime: essentially when i multiply it by deltatime thats how much i want it to move in one frame
    objectYRot += 20.0f * deltaTime;
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
    if(CheckObjectCollision(xpos, zpos, 0.0f, -1.0f, 0.3f)) {
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
        // seal
        interaction = SEVENDAYS;
        return 60;
    } else if (CheckObjectCollision(xpos, zpos, 0.0f, 1.8f, 0.2f)) {
        // back door
        interaction = ENTRANCE;
        return 60;
    } else if (CheckObjectCollision(xpos, zpos, 0.0f, -1.8f, 0.2f)) {
        // upper door
        interaction = LOCKED;
        return 60;
   /* } else if (CheckObjectCollision(xpos, zpos, 1.8f, 0.0f, 0.2f)) {
        // right door
        interaction = LOCKED;
        return 60; */
    } else if (CheckObjectCollision(xpos, zpos, -1.8f, 0.0f, 0.2f)) {
        // left door
        interaction = SEVENDAYS;
        return 60;
    }
    return 0;
}
