#include <gccore.h>

#include "dungeon.h"
#include "entrance.h"

Room debug[][5] = { { (Room) { .render = entrance, .init = entrance_load, .collide = entrance_collision, .interact = entrance_interact, .free = entrance_free, }, (Room){ .render = entrance, .init = entrance_load, .collide = entrance_collision, .interact = entrance_interact, .free = entrance_free } } };
