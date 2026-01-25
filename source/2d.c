#include "2d.h"

void DrawTex(f32 x, f32 y, unsigned int width, unsigned int height, GXTexObj texture) {
	GX_LoadTexObj(&texture, GX_TEXMAP0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position2s16(x, y);
	GX_TexCoord2f32(0.0f, 0.0f);
	GX_Position2s16(x + width, y);
	GX_TexCoord2f32(1.0f, 0.0f);
	GX_Position2s16(x + width, y + height);
	GX_TexCoord2f32(1.0f, 1.0f);
	GX_Position2s16(x, y + height);
	GX_TexCoord2f32(0.0f, 1.0f);

}
