#include <gccore.h>
// 2nd room in debug dungeon
#include "things.h"
#include "debug.h"
#include "inforabbit_txt.h"

SECTOR rabbit;
int debug_init() {
	SetupThing(inforabbit_txt, inforabbit_txt_size, &rabbit);
	return 0;
}
int debug_render() {
//	DrawThing(f32 (*v)[4], GXTexObj texture, SECTOR *object, f32 posX, f32 posY, f32 posZ, f32 rotY);
	return 0;
}
