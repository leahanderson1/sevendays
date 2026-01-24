#include <gccore.h>
// 2nd room in debug dungeon
#include "things.h"

#include "inforabbit_txt.h"

SECTOR rabbit;
int debug_init() {
	SetupThing(inforabbit_txt, inforabbit_txt_size, &rabbit);
	return 0;
}
