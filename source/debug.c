#include <gccore.h>
// 2nd room in debug dungeon
#include "things.h"
#include "debug.h"
#include "inforabbit_txt.h"

static SECTOR rabbit;
int debug_init() {
	SetupThing(inforabbit_txt, inforabbit_txt_size, &rabbit);
	return 0;
}
int debug_collision() {
	return 0;
}
int debug_render() {
	DrawThing(view, rabbitTex, &rabbit, 0.0f, 0.2f, 0.0f, 0.0f);
	return 0;
}
int debug_free() {
	FreeThing(rabbit);
	return 0;
}
