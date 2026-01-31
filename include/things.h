#ifndef THINGS_H
#define THINGS_H
#include <gccore.h>
#include <stdio.h>
#include "dungeon.h"
#include "ogc/gx.h"

typedef struct tagVERTEX // vertex coords - 3d and texture
{
    f32 x, y, z; // 3d coords
    f32 u, v;    // tex coords
    f32 nx, ny, nz; // normals
} VERTEX;
typedef struct tagMATRIXVERTEX {
	f32 x, y, z, u, v, nx, ny, nz;
	u8 index; // matrix index (this has a max of 10)
} MATRIXVERTEX;
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
typedef struct tagBONE {
    char name[32];
    int parent; // parent bone index (-1 for root)
    guVector pos;
    guVector rot;
    guVector scale;
    Mtx localTransform;
    Mtx worldTransform;
} Bone;
typedef struct tagSKINNED_VERTEX {
    f32 x, y, z;           // 3d coords
    f32 u, v;              // tex coords
    f32 nx, ny, nz;        // normals
    u8 indices[4];    // Up to 4 (fake) bones per vertex
    f32 weights[4]; 
} SKINNED_VERTEX;
typedef struct tagKEYFRAME {
    f32 time;              // in seconds
    guVector pos;
    guVector rot;
    guVector scale;
} Keyframe;
typedef struct tagANIMTRACK {
	int boneIndex;
	unsigned int numkeyframes;
	Keyframe* keyframes;
} AnimTrack;
typedef struct tagANIMATION {
	char name[32];
	f32 length;
	unsigned int numtracks;
	AnimTrack* tracks;
} Animation;
typedef struct tagARMATURE {
    int numBones;
    Bone* bones;
    Mtx* bindPoseInverse;
} Armature;
typedef struct tagTHING {
    SECTOR obj;
    guVector pos;
    GXTexObj *tex;
} Thing;
typedef struct tagTEXTURE {
	unsigned char* data; // raw RGBA8 pixels
	u16 width;
	u16 height;
	u8 format;
	bool dyanmic;
} Texture;
typedef struct tagANIMATEDTHING {
	int numvertices;
	MATRIXVERTEX* vertices;
	int numindices;
	unsigned short* indices;
	SKINNED_VERTEX* blendedvertices;
	Armature armature;
	unsigned int numanims;
	Animation* animations;
	unsigned int numtexs;
	Texture* texs;
	GXTexObj* objs; // converted textures
} AnimatedThing;
typedef struct tagANIMSTATE {
	unsigned int current;
	f32 time;
	bool playing;
	bool looping;
	f32 speed;
} AnimState;
typedef enum {
    NONE,
    NOTURNINGBACK,
    SEVENDAYS,
    LOCKED,
    ENTRANCE
} TEXT;
int SetupThing(const unsigned char* mdata, size_t msize, SECTOR* output);
void FreeThing(SECTOR thing);
bool CheckObjectCollision(f32 x, f32 z, f32, f32 x2, f32 z2);
void readstr(FILE *f, char *string);
void resetPlayer();
void DrawThing(Mtx v, GXTexObj texture, SECTOR* object, f32 posX, f32 posY, f32 posZ, f32 rotY);
void Draw2D();
void End2D(Mtx44);
void LoadAnimatedThing(const unsigned char* gltfdata, size_t size, AnimatedThing* thing);
void UpdateAnimation(AnimatedThing* model, AnimState* state);
void UpdateBoneTransforms(AnimatedThing* model);
void DrawAnimatedThing(Mtx v, AnimatedThing* model, f32 posX, f32 posY, f32 posZ);
void FreeAnimatedThing(AnimatedThing* model);
void PreprocessVerts(AnimatedThing* model);
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
extern f32 deltaTime;
extern f32 w, h;
extern bool titlething;
// this is so levels can access the wall texture in the case they need to block off exits from the base room
extern GXTexObj texture;
extern Room level;
#endif
