#ifndef ROOM_H
#define ROOM_H
#include "dungeon.h"
#include "things.h"
void changeRoom(Room dungeon[][9], roomPtr target);
#ifdef __GNUC__
typedef enum : unsigned {
#else
typedef enum {
#endif
	EMPTY,
	ONEEXIT,
	TWOEXITS_STRAIGHT,
	TWOEXITS_CORNER,
	THREEEXITS,
	FOUREXITS
} ROOMTYPE;
extern int room_3exits();
extern int room_2exits_straight();
extern int room_2exits_corner();
extern int room_1exit();
#endif
