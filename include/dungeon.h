#ifndef DUNGEON_H
#define DUNGEON_H
typedef struct {
	unsigned int x;
	unsigned int y;
} roomPtr;
typedef struct {
	int (*render)();
	int (*init)();
	int (*collide)();
	int (*interact)();
	int (*free)();
	roomPtr top;
	roomPtr left;
	roomPtr right;
	roomPtr bottom;
} Room;
#endif
