#include "things.h"
#include "2d.h"
#include <wiiuse/wpad.h>
#include <stdlib.h>
#include "logo_tpl.h"
#include "subtitle_tpl.h"

static TPLFile titleTPL;
static GXTexObj titleTexture;
static TPLFile subtitleTPL;
static GXTexObj subtitleTexture;
void title_init() {
	TPL_OpenTPLFromMemory(&titleTPL, (void *)logo_tpl, logo_tpl_size);
	TPL_GetTexture(&titleTPL, 0, &titleTexture);
	TPL_CloseTPLFile(&titleTPL);
	TPL_OpenTPLFromMemory(&subtitleTPL, (void *)subtitle_tpl, subtitle_tpl_size);
	TPL_GetTexture(&subtitleTPL, 0, &subtitleTexture);
	TPL_CloseTPLFile(&subtitleTPL);
	GX_InitTexObjFilterMode(&titleTexture, GX_NEAR, GX_NEAR);
	GX_InitTexObjFilterMode(&subtitleTexture, GX_NEAR, GX_NEAR);
}
void title(Mtx44 perspective) {
	if(titlething) {
	VIDEO_WaitVSync();
	PAD_ScanPads();
	WPAD_ScanPads();
	int pad = PAD_ButtonsDown(0);
	int wpad = WPAD_ButtonsDown(0);
	if(pad & PAD_BUTTON_A) titlething = false;
	if(wpad & WPAD_BUTTON_A) titlething = false;
	if (wpad & WPAD_BUTTON_HOME) exit(0);
	if(pad & PAD_BUTTON_START) exit(0);
	Draw2D();
	const int width = w/2;
	const int height = h/2;
	DrawTex(width, height, 256, 128, titleTexture);
	DrawTex(width + 10, height+128, 256, 64, subtitleTexture);
	End2D(perspective);
	}
}
