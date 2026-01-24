#ifndef ROOM_H
#define ROOM_H
#include "dungeon.h"
#include "things.h"
// credits to MatzeBraun on stack overflow for this bit that makes it portable to all C compilers
// url: https://stackoverflow.com/questions/11983231/is-it-safe-to-use-an-enum-in-a-bit-field
#ifdef __GNUC__ 
#define ENUMBF(type) __extension__ type
#else 
#define ENUMBF(type) unsigned
#endif
void changeRoom(Room dungeon[][9], roomPtr target);
#ifdef __GNUC__
typedef enum : unsigned {
#else
typedef enum {
#endif
//  there is a max of 16 possible room types you can use
	EMPTY,
	ONEEXIT,
	TWOEXITS_STRAIGHT,
	TWOEXITS_CORNER,
	THREEEXITS,
	FOUREXITS
} ROOMTYPE;
struct ROOMTYPE_PAIR {
	ENUMBF(ROOMTYPE) one:4;
	ENUMBF(ROOMTYPE) two:4;
};
extern int room_3exits();
extern int room_2exits_straight();
extern int room_2exits_corner();
extern int room_1exit();
#endif
