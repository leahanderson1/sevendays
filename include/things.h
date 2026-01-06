#ifndef THINGS_H
#define THINGS_H
#include <gccore.h>
typedef struct tagVERTEX // vertex coords - 3d and texture
{
    f32 x, y, z; // 3d coords
    f32 u, v;    // tex coords
    f32 nx, ny, nz; // normals
} VERTEX;

// Triangle is a set of three vertices.
typedef struct tagTRIANGLE // triangle
{
    VERTEX vertex[3]; // 3 vertices
} TRIANGLE;

// Sector represents a room, i.e. series of tris.
typedef struct tagSECTOR
{
    int numtriangles;   // Number of tris in this sector
    TRIANGLE* triangle; // Ptr to array of tris
} SECTOR;
typedef struct {
    SECTOR obj;
    guVector pos;
    GXTexObj tex;
} Thing;
typedef enum {
    NONE,
    NOTURNINGBACK,
    SEVENDAYS,
    LOCKED
} TEXT;
int SetupThing(const unsigned char* mdata, size_t msize, SECTOR* output);
void FreeThing(SECTOR thing);
bool CheckObjectCollision(f32 x, f32 z, f32, f32 x2, f32 z2);
void readstr(FILE *f, char *string);
void resetPlayer();
void DrawThing(Mtx v, GXTexObj texture, SECTOR* object, f32 posX, f32 posY, f32 posZ, f32 rotY);
extern int (*level_render)();
extern int (*level_init)();
extern int (*level_collide)();
extern int (*level_interact)();
extern int (*level_free)();
extern TPLFile sealTPL;
extern SECTOR sealObj;
extern GXTexObj sealTexture;
extern Mtx view;
extern f32 xrot;
extern f32 yrot;
extern f32 lookupdown;
extern f32 walkbias;
extern f32 xpos, zpos;
extern TEXT interaction;
#endif
