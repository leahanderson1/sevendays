#include "dungeon.h"
#include "things.h"

#include "room.h"

void changeRoom(Room dungeon[][9], roomPtr target) {
	Room new = dungeon[target.x][target.y];
	(*level_free)();
	level_collide = new.collide;
	level_render = new.render;
	level_init = new.init;
	level_free = new.free;
	level_interact = new.interact;
	level = new;
}

// room prototypes to use

int room_3exits() {
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
    return 0;
}

int room_2exits_straight() {			
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
    //left wall
    GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    GX_Position3f32(-2.0f, 0.0f, -0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);
    GX_Position3f32(-2.0f, 0.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 1.0f);
    GX_Position3f32(-2.0f, 1.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 0.0f);
    GX_Position3f32(-2.0f, 1.0f, -0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);

    GX_Position3f32(-2.0f, 0.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 1.0f);
    GX_Position3f32(-2.0f, 0.0f, 0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);
    GX_Position3f32(-2.0f, 1.0f, 0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);
    GX_Position3f32(-2.0f, 1.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 0.0f);
    GX_End();
    return 0;
}
int room_2exits_corner() {
    room_3exits();
    GX_Begin(GX_QUADS, GX_VTXFMT0,8);
    GX_Position3f32(-0.5f, 0.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);//top left
    GX_Position3f32(0.0f, 0.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 1.0f);//top right
    GX_Position3f32(0.0f, 1.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 0.0f);//bottom right
    GX_Position3f32(-0.5f, 1.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);//bottom left
    
    GX_Position3f32(0.0f, 0.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 1.0f);
    GX_Position3f32(0.5f, 0.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);
    GX_Position3f32(0.5f, 1.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);
    GX_Position3f32(0.0f, 1.0f, -2.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 0.0f);
    GX_End();
    return 0;
}
int room_1exit() {
    room_2exits_corner();
    GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    GX_Position3f32(-2.0f, 0.0f, -0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);
    GX_Position3f32(-2.0f, 0.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 1.0f);
    GX_Position3f32(-2.0f, 1.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(1.0f, 0.0f);
    GX_Position3f32(-2.0f, 1.0f, -0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);

    GX_Position3f32(-2.0f, 0.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 1.0f);
    GX_Position3f32(-2.0f, 0.0f, 0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 1.0f);
    GX_Position3f32(-2.0f, 1.0f, 0.5f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.5f, 0.0f);
    GX_Position3f32(-2.0f, 1.0f, 0.0f);
    GX_Normal3f32(0.0f, 0.0f, 0.0f);
    GX_TexCoord2f32(0.0f, 0.0f);
    GX_End();
    return 0;
}
