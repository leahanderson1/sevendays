#include "things.h"
#include <wiiuse/wpad.h>
#include <stdlib.h>

void title(Mtx44 perspective) {
	int pad = PAD_ButtonsDown(0);
	int wpad = WPAD_ButtonsDown(0);
	if(wpad & WPAD_BUTTON_HOME) exit(0);
	if(pad & PAD_BUTTON_START) exit(0);
	if(wpad & WPAD_BUTTON_A) return;
	if(pad & PAD_BUTTON_A) return;
	Draw2D();
	End2D(perspective);
}
