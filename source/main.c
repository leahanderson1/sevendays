#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>

#include "ogc/tpl.h"
#include "things.h"
#include "2d.h"
#include "mud_tpl.h"
#include "mud.h"
#include "world_txt.h"
#include "sevendays_tpl.h"
#include "lighter_tpl.h"
#include "door_tpl.h"
#include "door.h"
#include "doortext_tpl.h"
#include "locked_tpl.h"
#include "noescape_tpl.h"
#include "entrance.h"
#define DEFAULT_FIFO_SIZE	(256*1024)
#define ROOM_X 1.8f
#define ROOM_Z 1.8f
/* DATA FILE FORMAT
 * Each triangle in our data file is declared as follows:
 *
 * X1 Y1 Z1 U1 V1 NX1 NY1 NZ1
 * X2 Y2 Z2 U2 V2 NX2 NY2 NZ2
 * X3 Y3 Z3 U3 V3 NX3 NY3 NZ3 */
static u64 lastTime = 0;
// delta time low and high limits
#define LOW_LIMIT 0.0167f
#define HIGH_LIMIT 0.1f
f32 deltaTime = 0.0f;
static void *frameBuffer[2] = { NULL, NULL};
GXRModeObj *rmode;

f32 xrot;   // x rotation
f32 yrot;
f32 xspeed; // x rotation speed
f32 yspeed; // y rotation speed

f32 walkbias = 0;
f32 walkbiasangle = 0;

f32 lookupdown = 0.0f;
int interactTimer = 0;
f32 xpos, zpos;
Mtx view;
f32 zdepth=0.0f; // depth into the screen

static GXColor LightColors[] = {
	{ 0xFF, 0xFF, 0xFF, 0xFF }, // Light color 1
	{ 0x80, 0x80, 0x80, 0xFF }, // Ambient 1
	{ 0x80, 0x80, 0x80, 0xFF }  // Material 1
};

	// A vertex is the basic element of our room.
f32 w, h;
SECTOR sector1;
GXTexObj texture;
TPLFile mudTPL;
TPLFile sealTPL;
TPLFile sevenTPL;
GXTexObj sealTexture;
GXTexObj sevenTexture;
TPLFile lighterTPL;
GXTexObj lighterTexture;
TPLFile doorTPL;
GXTexObj doorTexture;
TPLFile noTPL;
GXTexObj noTexture;
TPLFile lockedTPL;
GXTexObj lockedTexture;
TPLFile entranceTPL;
GXTexObj entranceTexture;
int (*level_render)();
int (*level_init)();
int (*level_collide)();
int (*level_interact)();
int (*level_free)();

void DrawScene(Mtx v, GXTexObj texture);
void SetLight(Mtx view, GXColor litcol, GXColor ambcol, GXColor matcol, f32 playerX, f32 playerZ);
int SetupWorld(void);
static inline void textDraw(GXTexObj);
bool CheckObjectCollision(f32 x, f32 z, f32, f32 x2, f32 z2);
TEXT interaction;
bool titlething = true;

//---------------------------------------------------------------------------------
int main( int argc, char **argv ){
	//---------------------------------------------------------------------------------
	f32 yscale;

	u32 xfbHeight;
	interaction = NONE;
	// various matrices for things like view
	Mtx mv,mr;
	Mtx44 perspective;
	level_render = &entrance;
	level_init = &entrance_load;
	level_collide = &entrance_collision;
	level_interact = &entrance_interact;
	level_free = &entrance_free;

	u32	fb = 0; 	// initial framebuffer index
	GXColor background = {0, 0, 0, 0xff};
	// various init stuff
	VIDEO_Init();
	PAD_Init();
	WPAD_Init();

	rmode = VIDEO_GetPreferredMode(NULL);

	// allocate 2 framebuffers for double buffering
	frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// configure video and wait for the screen to blank
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(frameBuffer[fb]);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	// setup the fifo...
	void *gp_fifo = NULL;
	gp_fifo = memalign(32,DEFAULT_FIFO_SIZE);
	memset(gp_fifo,0,DEFAULT_FIFO_SIZE);

	// ...then init the GPU
	GX_Init(gp_fifo,DEFAULT_FIFO_SIZE);

	// clears the bg to color and clears the z buffer
	GX_SetCopyClear(background, 0x00ffffff);

	// other gx setup
	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
	yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
	if (rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(frameBuffer[fb],GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	// setup the vertex attribute table
	// describes the data
	// args: vat location 0-7, type of data, data format, size, scale
	// so for ex. in the first call we are sending position data with
	// 3 values X,Y,Z of size F32. scale sets the number of fractional
	// bits for non float data.
	GX_InvVtxCache(); // by Inv it means invalidate
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	// selects the data types for each input
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetTevOp(GX_TEVSTAGE0,GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	// setup texture coordinate generation
	// args: texcoord slot 0-7, matrix type, source to generate texture coordinates from, matrix to use
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY);

	w = rmode->viWidth;
	h = rmode->viHeight;
	guLightPerspective(mv,45, (f32)w/h, 1.05F, 1.0F, 0.0F, 0.0F);
	guMtxTrans(mr, 0.0F, 0.0F, -1.0F);
	guMtxConcat(mv, mr, mv);
	void title_init(void);
	void title(Mtx44);
	GX_LoadTexMtxImm(mv, GX_TEXMTX0, GX_MTX3x4);
	GX_InvalidateTexAll();
	title_init();
	TPL_OpenTPLFromMemory(&mudTPL, (void *)mud_tpl,mud_tpl_size);
	TPL_GetTexture(&mudTPL,mud,&texture);
	TPL_CloseTPLFile(&mudTPL);
	TPL_OpenTPLFromMemory(&sevenTPL, (void *)sevendays_tpl, sevendays_tpl_size);
	TPL_GetTexture(&sevenTPL,0,&sevenTexture);
	TPL_CloseTPLFile(&sevenTPL);
	TPL_OpenTPLFromMemory(&lighterTPL, (void *)lighter_tpl, lighter_tpl_size);
	TPL_GetTexture(&lighterTPL,0,&lighterTexture);
	TPL_CloseTPLFile(&lighterTPL);
	TPL_OpenTPLFromMemory(&doorTPL, (void *)door_tpl, door_tpl_size);
	TPL_GetTexture(&doorTPL,door,&doorTexture);
	TPL_CloseTPLFile(&doorTPL);
	TPL_OpenTPLFromMemory(&noTPL,(void *)doortext_tpl, doortext_tpl_size);
	TPL_GetTexture(&noTPL,0,&noTexture);
	TPL_CloseTPLFile(&noTPL);
	TPL_OpenTPLFromMemory(&lockedTPL, (void *)locked_tpl, locked_tpl_size);
	TPL_GetTexture(&lockedTPL, 0, &lockedTexture);
	TPL_CloseTPLFile(&lockedTPL);
	TPL_OpenTPLFromMemory(&entranceTPL, (void *)noescape_tpl, noescape_tpl_size);
	TPL_GetTexture(&entranceTPL, 0, &entranceTexture);
	TPL_CloseTPLFile(&entranceTPL);
	GX_InitTexObjFilterMode(&sevenTexture, GX_NEAR, GX_NEAR);
	GX_InitTexObjFilterMode(&noTexture, GX_NEAR, GX_NEAR);
	GX_InitTexObjFilterMode(&lockedTexture, GX_NEAR, GX_NEAR);
	GX_InitTexObjFilterMode(&entranceTexture, GX_NEAR, GX_NEAR);
	// setup our camera at the origin
	// looking down the -z axis with y up
	guVector cam = {0.0F, 0.0F, 0.0F},
	up = {0.0F, 1.0F, 0.0F},
	look = {0.0F, 0.0F, -1.0F};
	guLookAt(view, &cam, &up, &look);

	// setup our projection matrix
	// this creates a perspective matrix with a view angle of 90,
	// and aspect ratio based on the display resolution
	guPerspective(perspective, 45, (f32)w/h, 0.1F, 300.0F);
	GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);
	// get the initial room ready to render
	SetupWorld();
	// call whatever function level_init is referring to
	(*level_init)();
	while(1) {
		VIDEO_WaitVSync();
		f32 moveForward = 0.0f;
		f32 moveStrafe = 0.0f;
		u64 currentTime = gettime();
		deltaTime = (f32)diff_usec(lastTime, currentTime) / 1000000.0f;
		lastTime = currentTime;
		if(deltaTime > HIGH_LIMIT) 
			deltaTime = HIGH_LIMIT;
		else if(deltaTime < LOW_LIMIT)
				deltaTime = LOW_LIMIT;
		f32 moveSpeed = 0.05f * (deltaTime * 60.0f);
		if(!titlething) {
		PAD_ScanPads();
		WPAD_ScanPads();
		if(!titlething) {
		int pad = PAD_ButtonsDown(0);
		int wpad = WPAD_ButtonsDown(0);
		if (wpad & WPAD_BUTTON_HOME) exit(0);
		if(pad & PAD_BUTTON_START) exit(0);
		if(pad & PAD_BUTTON_A) {
			// as well as set the interact timer, the interact function also sets what kind of interaction to use
			interactTimer = (*level_interact)();
		}
		s8 tpad;
		tpad = PAD_SubStickX(0);
		if ((tpad < -8) || (tpad > 8)) yrot -= (float)tpad / 50.f;
		tpad = PAD_SubStickY(0);
		if ((tpad < -8) || (tpad > 8)) xrot += (float)tpad / 50.f;
		// Get movement input
		tpad = PAD_StickY(0);
		if(tpad > 50) {
			moveForward = 1.0f; // Move on the x-plane based on player direction
		}

		// Go backward
		if(tpad < -50) {
			moveForward = -1.0f;

		}

		tpad = PAD_StickX(0);
		if(tpad > 50) {
			moveStrafe = 1.0f;
		} 
		if(tpad < -50) {
			moveStrafe = -1.0f;
		}
		}
// why the fuck is this here and what is it doing
// i removed it and it still works fine
	/*	if(moveForward != 0.0f && moveStrafe != 0.0f) {
			f32 length = sqrtf(moveForward * moveForward + moveStrafe * moveStrafe);
			moveForward /= length;
			moveStrafe /= length;
		} */
		if(moveForward != 0.0f || moveStrafe != 0.0f) {
			f32 proposedX = xpos - (float)sin(DegToRad(yrot)) * moveForward * moveSpeed;
			f32 proposedZ = zpos - (float)cos(DegToRad(yrot)) * moveForward * moveSpeed;

			proposedX += (float)cos(DegToRad(yrot)) * moveStrafe * moveSpeed;
			proposedZ -= (float)sin(DegToRad(yrot)) * moveStrafe * moveSpeed;

			if (!(proposedX >= ROOM_X) && !(proposedX <= -ROOM_X)) {
				xpos = proposedX;
			}
			if (!(proposedZ >= ROOM_Z) && !(proposedZ <= -ROOM_Z)) {
				zpos = proposedZ;
			}
			if (walkbiasangle >= 359.0f) {
				walkbiasangle = 0.0f;
			} else {
				walkbiasangle += 10;
			}
			walkbias = (float)sin(DegToRad(walkbiasangle))/25.0f;
		}
		}
		(*level_collide)();
		
		// do this before drawing
		GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
		GX_SetNumTexGens(1);

		// Draw things
		DrawScene(view,texture);
		(*level_render)();
		Draw2D();
		// 2D drawing
		if (interactTimer > 0) {
			switch (interaction) {
				default:
					break; //unimplemented
				case NONE:
					interactTimer = 1; // looks like SOMEONE forgot to clear the interaction timer so i'll just fix it for ya buddy
					break;
				case SEVENDAYS:
					textDraw(sevenTexture);
					break;
				case NOTURNINGBACK:
					textDraw(noTexture);
					break;
				case LOCKED:
					textDraw(lockedTexture);
					break;
				case ENTRANCE:
					textDraw(entranceTexture);
					break;
			}
			interactTimer--;
		}
		if (interactTimer <= 0 && interaction != NONE)
			interaction = NONE;
		DrawTex(300, 250, 330, 247, lighterTexture);
		title(perspective);
		End2D(perspective);
		GX_CopyDisp(frameBuffer[fb],GX_TRUE);
		// do this stuff after drawing
		GX_DrawDone();
		fb ^= 1; // flip framebuffer
		VIDEO_SetNextFramebuffer(frameBuffer[fb]);

		VIDEO_Flush();



	}
	return 0;
}

// Perform the actual scene drawing.
void DrawScene(Mtx v, GXTexObj texture) {
	// Draw things
	// FIXME: Need to clear first?
	// FIXME: Check datatype sizes
	f32 x_m,y_m,z_m,u_m,v_m;       // Float types for temp x, y, z, u and v vertices
	f32 xtrans = -xpos;            // Used for player translation on the x axis
	f32 ztrans = -zpos;            // Used for player translation on the z axis
	f32 ytrans = -walkbias-0.25f;  // Used for bouncing motion up and down
	f32 sceneroty = 360.0f - yrot; // 360 degree angle for player direction
	f32 scenerotx = 360.0f - xrot;
	int numtriangles;              // Integer to hold the number of triangles
	Mtx m; // Model matrix
	Mtx mt; // Model rotated matrix
	Mtx mv; // Modelview matrix
	guVector axis;                 // Vector for axis we're rotating on

	SetLight(v, LightColors[0], LightColors[1], LightColors[2], xpos, zpos);
	GX_LoadTexObj(&texture, GX_TEXMAP0);

	//glRotatef(lookupdown,1.0f,0,0);
	axis.x = 1.0f;
	axis.y = 0;
	axis.z = 0;
	guMtxIdentity(m);
	guMtxRotAxisDeg(m, &axis, scenerotx);
	guMtxConcat(m,v,mv);

	//glrotatef(sceneroty,0,1.0f,0);
	axis.x = 0;
	axis.y = 1.0f;
	axis.z = 0;
	guMtxIdentity(m);
	guMtxRotAxisDeg(m, &axis, sceneroty);
	guMtxConcat(mv,m,mv);

	// Translate the camera view
	guMtxApplyTrans(mv,mt,xtrans,ytrans,ztrans);

	//glTranslatef(xtrans,ytrans,ztrans);
	//guMtxIdentity(m);
	//guMtxTrans(m, xtrans, ytrans, ztrans);
	//guMtxConcat(v,m,v);

	// load the modelview matrix into matrix memory
	GX_LoadPosMtxImm(mt, GX_PNMTX0);

	numtriangles = sector1.numtriangles;

	// HACK: v tex coord is inverted so textures are rightside up.
	for (int loop_m = 0; loop_m < numtriangles; loop_m++) {
		GX_Begin(GX_TRIANGLES,GX_VTXFMT0,3);
		x_m = sector1.triangle[loop_m].vertex[0].x;
		y_m = sector1.triangle[loop_m].vertex[0].y;
		z_m = sector1.triangle[loop_m].vertex[0].z;
		u_m = sector1.triangle[loop_m].vertex[0].u;
		v_m = sector1.triangle[loop_m].vertex[0].v;
		GX_Position3f32(x_m,y_m,z_m);
		GX_Normal3f32((f32)sector1.triangle[loop_m].vertex[0].nx, (f32)sector1.triangle[loop_m].vertex[0].ny, (f32)sector1.triangle[loop_m].vertex[0].nz);
		//GX_Color3f32(0.7f,0.7f,0.7f);
		GX_TexCoord2f32(u_m,-v_m);

		x_m = sector1.triangle[loop_m].vertex[1].x;
		y_m = sector1.triangle[loop_m].vertex[1].y;
		z_m = sector1.triangle[loop_m].vertex[1].z;
		u_m = sector1.triangle[loop_m].vertex[1].u;
		v_m = sector1.triangle[loop_m].vertex[1].v;
		GX_Position3f32(x_m,y_m,z_m);
		GX_Normal3f32((f32)sector1.triangle[loop_m].vertex[1].nx,(f32)sector1.triangle[loop_m].vertex[1].ny,(f32)sector1.triangle[loop_m].vertex[1].nz);
		//GX_Color3f32(0.7f,0.7f,0.7f);
		GX_TexCoord2f32(u_m,-v_m);

		x_m = sector1.triangle[loop_m].vertex[2].x;
		y_m = sector1.triangle[loop_m].vertex[2].y;
		z_m = sector1.triangle[loop_m].vertex[2].z;
		u_m = sector1.triangle[loop_m].vertex[2].u;
		v_m = sector1.triangle[loop_m].vertex[2].v;
		GX_Position3f32(x_m,y_m,z_m);
		GX_Normal3f32((f32)sector1.triangle[loop_m].vertex[2].nx, (f32)sector1.triangle[loop_m].vertex[2].ny, (f32)sector1.triangle[loop_m].vertex[2].nz);
		//GX_Color3f32(0.7f,0.7f,0.7f);
		GX_TexCoord2f32(u_m,-v_m);
		GX_End();
	}
	GX_LoadTexObj(&doorTexture, GX_TEXMAP0);
	// draw doors in every doorway
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
	GX_Position3f32(-0.5f, 0.0f, 3.0f);
	GX_Normal3f32(0.0f, 0.0f, 1.0f);
	GX_TexCoord2f32(0.0f, 0.0f);//top left
	GX_Position3f32(0.5f, 0.0f, 3.0f);
	GX_Normal3f32(0.0f, 0.0f, 1.0f);
	GX_TexCoord2f32(1.0f, 0.0f);//top right
	GX_Position3f32(0.5f, 1.0f, 3.0f);
	GX_Normal3f32(0.0f, 0.0f, 1.0f);
	GX_TexCoord2f32(1.0f, 1.0f);//bottom right
	GX_Position3f32(-0.5f, 1.0f, 3.0f);
	GX_Normal3f32(0.0f, 0.0f, 1.0f);
	GX_TexCoord2f32(0.0f, 1.0f);//bottom left
	GX_End();
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
	GX_Position3f32(-0.5f, 0.0f, -3.0f);
	GX_Normal3f32(0.0f, 0.0f, -1.0f);
	GX_TexCoord2f32(0.0f, 0.0f);//top left
	GX_Position3f32(0.5f, 0.0f, -3.0f);
	GX_Normal3f32(0.0f, 0.0f, -1.0f);
	GX_TexCoord2f32(1.0f, 0.0f);//top right
	GX_Position3f32(0.5f, 1.0f, -3.0f);
	GX_Normal3f32(0.0f, 0.0f, -1.0f);
	GX_TexCoord2f32(1.0f, 1.0f);//bottom right
	GX_Position3f32(-0.5f, 1.0f, -3.0f);
	GX_Normal3f32(0.0f, 0.0f, -1.0f);
	GX_TexCoord2f32(0.0f, 1.0f);//bottom left
	GX_End();
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
	GX_Position3f32(3.0f, 0.0f, -0.5f);
	GX_Normal3f32(1.0f, 0.0f, 0.0f);
	GX_TexCoord2f32(0.0f, 0.0f);//top left
	GX_Position3f32(3.0f, 0.0f, 0.5f);
	GX_Normal3f32(1.0f, 0.0f, 0.0f);
	GX_TexCoord2f32(1.0f, 0.0f);//top right
	GX_Position3f32(3.0f, 1.0f, 0.5f);
	GX_Normal3f32(1.0f, 0.0f, 0.0f);
	GX_TexCoord2f32(1.0f, 1.0f);//bottom right
	GX_Position3f32(3.0f, 1.0f, -0.5f);
	GX_Normal3f32(1.0f, 0.0f, 0.0f);
	GX_TexCoord2f32(0.0f, 1.0f);//bottom left
	GX_End();
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
	GX_Position3f32(-3.0f, 0.0f, 0.5f);
	GX_Normal3f32(-1.0f, 0.0f, 0.5f);
	GX_TexCoord2f32(0.0f, 0.0f);//top left
	GX_Position3f32(-3.0f, 0.0f, -0.5f);
	GX_Normal3f32(-1.0f, 0.0f, 0.0f);
	GX_TexCoord2f32(1.0f, 0.0f);//top right
	GX_Position3f32(-3.0f, 1.0f, -0.5f);
	GX_Normal3f32(-1.0f, 0.0f, 0.0f);
	GX_TexCoord2f32(1.0f, 1.0f);//bottom right
	GX_Position3f32(-3.0f, 1.0f, 0.5f);
	GX_Normal3f32(-1.0f, 0.0f, 0.0f);
	GX_TexCoord2f32(0.0f, 1.0f);//bottom left
	GX_End();

	return;
}

void SetLight(Mtx view, GXColor litcol, GXColor ambcol, GXColor matcol, f32 playerX, f32 playerZ)
{
	guVector lpos;
	GXLightObj lobj;

	lpos.x = playerX; 
	lpos.y = 0.5f;
	lpos.z = playerZ;

	GX_InitLightPos(&lobj, lpos.x, lpos.y, lpos.z);
	GX_InitLightColor(&lobj, litcol);
	GX_LoadLightObj(&lobj, GX_LIGHT0);

	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GX_SetChanAmbColor(GX_COLOR0A0, ambcol);
	GX_SetChanMatColor(GX_COLOR0A0, matcol);
}

// sets up the initial generic 4-exit room. You can remove an exit by just drawing a quad over an entrance.

int SetupWorld(void) {
	FILE *filein;
	int numtriangles; // Number of triangles in sector
	char line[255];   // String to store data in
	float x = 0;      // 3D coords
	float y = 0;
	float z = 0;
	float u = 0;      // tex coords
	float v = 0;
	float nx = 0;
	float ny = 0;
	float nz = 0;

	// open file in memory
	filein = fmemopen((void *)world_txt, world_txt_size, "rb");

	// read in data
	readstr(filein, line); // Get single line of data
	sscanf(line, "NUMPOLYS %d\n", &numtriangles); // Read in number of triangles

	// allocate new triangle objects
	sector1.triangle = (TRIANGLE*)malloc(sizeof(TRIANGLE)*numtriangles);
	sector1.numtriangles = numtriangles;

	// Step through each tri in sector
	for (int triloop = 0; triloop < numtriangles; triloop++) {
		// Step through each vertex in tri
		for (int vertloop = 0; vertloop < 3; vertloop++) {
			readstr(filein,line); // Read string
			if (line[0] == '\r' || line[0] == '\n') { // Ignore blank lines.
				vertloop--;
				continue;
			}
			if (line[0] == '/') { // Ignore lines with comments.
				vertloop--;
				continue;
			}
			sscanf(line, "%f %f %f %f %f %f %f %f", &x, &y, &z, &u, &v, &nx, &ny, &nz); // Read in data from string
			// Store values into respective vertices
			sector1.triangle[triloop].vertex[vertloop].x = x;
			sector1.triangle[triloop].vertex[vertloop].y = y;
			sector1.triangle[triloop].vertex[vertloop].z = z;
			sector1.triangle[triloop].vertex[vertloop].u = u;
			sector1.triangle[triloop].vertex[vertloop].v = v;
			sector1.triangle[triloop].vertex[vertloop].nx = nx;
			sector1.triangle[triloop].vertex[vertloop].ny = ny;
			sector1.triangle[triloop].vertex[vertloop].nz = nz;
		}
	}
	fclose(filein);
	return 0;
}

void Draw2D() {
	Mtx model;
	Mtx44 ortho;
	guOrtho(ortho, 0, rmode->efbHeight, 0, rmode->fbWidth, 0, 1000);
	GX_LoadProjectionMtx(ortho, GX_ORTHOGRAPHIC);

	guMtxIdentity(model);
	GX_LoadPosMtxImm(model, GX_PNMTX0);

	// Disable Z-buffer
	GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);

	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetAlphaCompare(GX_GREATER, 8, GX_AOP_AND, GX_GREATER, 8);
	GX_SetZCompLoc(GX_FALSE);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
}

void End2D(Mtx44 perspective) {
	//all of this attempts to restore order to the rendering process after setting stuff to draw in 2D
	//i have no idea if i remembered to put all the right functions i actually need and its hard enough debugging such an obscure API but it runs on both my emulator and my real wii so its good enough for me
	GX_InvalidateTexAll();
	GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);

	GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
	GX_SetCullMode(GX_CULL_NONE);

	GX_InvVtxCache();
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetTevOp(GX_TEVSTAGE0,GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetNumTexGens(1);

	GX_SetNumChans(1);
        GX_SetColorUpdate(GX_TRUE);


}
// draw text at the top left of the screen
// TODO: use fontsheet and dynamically set size of quad
static inline void textDraw(GXTexObj texture) {
	DrawTex(0, 50, 256, 32, texture);
}

void resetPlayer() {
	yrot = 0.0f;
	xrot = 0.0f;
	xpos = 0.0f;
	zpos = 1.8f;
	guVector cam = {0.0F, 0.0F, 0.0F},
	up = {0.0F, 1.0F, 0.0F},
	look = {0.0F, 0.0F, -1.0F};
	guLookAt(view, &cam, &up, &look);

}
