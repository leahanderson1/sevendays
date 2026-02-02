#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "things.h"
#include "dungeon.h"

Room level;

#define PLAYER_RADIUS 0.15f

void readstr(FILE *f, char *string) {
    do {
        fgets(string, 255, f);
    } while ((string[0] == '/') || (string[0] == '\n'));
    return;
}
int SetupThing(const unsigned char* mdata, size_t msize, SECTOR* output) {
    FILE *filein;
    int numtriangles; // Number of triangles in sector
    char line[255];   // String to store data in
    float x = 0;      // 3D coords
    float y = 0;
    float z = 0;
    float u = 0;      // tex coords
    float v = 0;
    float nx = 0;     // normal coords
    float ny = 0;
    float nz = 0;

    // open file in memory
    filein = fmemopen((void *)mdata, msize, "rb");

    // read in data
    readstr(filein, line); // Get single line of data
    sscanf(line, "NUMPOLYS %d\n", &numtriangles); // Read in number of triangles

    // allocate new triangle objects
    output->triangle = (TRIANGLE*)malloc(sizeof(TRIANGLE)*numtriangles);
    output->numtriangles = numtriangles;

    // Step through each tri in sector
    for (int triloop = 0; triloop < numtriangles; triloop++) {
        // Step through each vertex in tri
        for (int vertloop = 0; vertloop < 3; vertloop++) {
            readstr(filein,line); // Read string
            if (line[0] == '\r' || line[0] == '\n') { // Ignore blank lines.
                vertloop--;
                continue;
            }
            if (line[0] == '/') { // Ignore lines that start with /, we don't need to check if it has a second / because even if it doesn't then it would be an invalid line so might as well just ignore it always
                vertloop--;
                continue;
            }
            sscanf(line, "%f %f %f %f %f %f %f %f", &x, &y, &z, &u, &v, &nx, &ny, &nz); // Read in data from string
            // Store values into respective vertices
            output->triangle[triloop].vertex[vertloop].x = x;
            output->triangle[triloop].vertex[vertloop].y = y;
            output->triangle[triloop].vertex[vertloop].z = z;
            output->triangle[triloop].vertex[vertloop].u = u;
            output->triangle[triloop].vertex[vertloop].v = v;
            output->triangle[triloop].vertex[vertloop].nx = nx;
            output->triangle[triloop].vertex[vertloop].ny = ny;
            output->triangle[triloop].vertex[vertloop].nz = nz;
        }
    }
    fclose(filein);
    return 0;
}
void DrawThing(Mtx v, GXTexObj texture, SECTOR* object, f32 posX, f32 posY, f32 posZ, f32 rotY) {
    Mtx m, mr, mt, mv;
    guVector Xaxis, Yaxis;
    f32 xtrans = -xpos;
    f32 ztrans = -zpos;
    f32 ytrans = -walkbias - 0.25f;
    f32 scenerotx = 360.0f - xrot;
    f32 sceneroty = 360.0f - yrot;
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_LoadTexObj(&texture, GX_TEXMAP0);
    Xaxis.x = 1.0f;
    Xaxis.y = 0;
    Xaxis.z = 0;
    guMtxIdentity(m);
    guMtxRotAxisDeg(m, &Xaxis, scenerotx);
    guMtxConcat(m, v, mv);
    Yaxis.x = 0;
    Yaxis.y = 1.0f;
    Yaxis.z = 0;
    guMtxIdentity(m);
    guMtxRotAxisDeg(m, &Yaxis, sceneroty);
    guMtxConcat(mv, m, mv);
    guMtxApplyTrans(mv, mt, xtrans, ytrans, ztrans);
    guMtxApplyTrans(mt, m, posX, posY, posZ);
    guMtxRotAxisDeg(mr, &Yaxis, rotY);
    guMtxConcat(m, mr, mv);
    GX_LoadPosMtxImm(mv, GX_PNMTX0);

    // Draw triangles
    for (int loop_m = 0; loop_m < object->numtriangles; loop_m++) {
        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
        for(int vtx = 0; vtx < 3; vtx++) {
            GX_Position3f32(
                object->triangle[loop_m].vertex[vtx].x,
                object->triangle[loop_m].vertex[vtx].y,
                object->triangle[loop_m].vertex[vtx].z
            );
            GX_Normal3f32(
                object->triangle[loop_m].vertex[vtx].nx,
                object->triangle[loop_m].vertex[vtx].ny,
                object->triangle[loop_m].vertex[vtx].nz
            );
            GX_TexCoord2f32(
                object->triangle[loop_m].vertex[vtx].u,
                object->triangle[loop_m].vertex[vtx].v
            );
        }
        GX_End();
    }
}
void FreeThing(SECTOR thing) {
    free(thing.triangle);
    thing.triangle = NULL;
}
// TODO: get rid of this fucked up radius collision system and replace it with proper AABB
bool CheckObjectCollision(f32 x, f32 z, f32 x2, f32 z2, f32 radius) {
    f32 dx = x2 - x;
    f32 dz = z2 - z;
    f32 distance = sqrtf(dx*dx + dz*dz);
    return distance < (radius + PLAYER_RADIUS);
}
