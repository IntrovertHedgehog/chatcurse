#ifndef INCLUDE_SRC_PROCESS_INPUT_H_
#define INCLUDE_SRC_PROCESS_INPUT_H_

#include <memory>

#include "global.h"
#include "tg.h"
#define S_NONE 0
#define S_MOUSE_DRAG 1
#define S_KEY_PREF 2

#include <curses.h>

void process_input(TgClient &, bool &);
void process_mouse(TgClient &, MEVENT *);
void process_B1_pressed(MEVENT *);

#endif
