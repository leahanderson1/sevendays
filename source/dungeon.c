#include <gccore.h>

#include "things.h"
#include "dungeon.h"
#include "entrance.h"

Room debug[2][2] = { { (Room){ .render = entrance, .init = entrance_load, .collide = entrance_collision, .interact = entrance_interact, .free = entrance_free, (roomPtr){0, 0}, (roomPtr){0, 0}, (roomPtr){0,0}, (roomPtr){0,0} } } };
