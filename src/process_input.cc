#include "process_input.h"

#include <curses.h>

#include <format>

#include "utils.h"
#include "layout.h"
#include "global.h"

void process_mouse(MEVENT *mevent) {
  debug_log(std::format("mouse event received ({}, {}, {}, {})", mevent->bstate,
                        mevent->y, mevent->x, mevent->z));

  if ((mevent->bstate & BUTTON1_PRESSED)) {
    process_B1_pressed(mevent);
  }
}

void process_B1_pressed(MEVENT *mevent) {
  bool edge;
  int where{};

  if (mevent->y < LINES - composer_h) {
    if (mevent->x < side_w - 1) {
      edge = false;
      where = ID_SIDE;
    } else if (mevent->x > side_w - 1) {
      edge = false;
      where = ID_MAIN;
    } else {
      edge = true;
      where = ID_E_SIDE_MAIN;
    }
  } else if (mevent->y > LINES - composer_h) {
    edge = false;
    where = ID_COMP;
  } else {
    edge = true;
    if (mevent->x == side_w - 1) {
      where = ID_C_SIDE_MAIN_COMP;
    } else {
      where = ID_E_COMP_TOP;
    }
  }

  if (edge) {
    debug_log(std::format("dragging edge {}", where));
    int c;
    MEVENT mevent2;
    while (true) {
      c = getch();
      if (c != KEY_MOUSE) continue;
      if (getmouse(&mevent2) != OK) continue;
      debug_log(std::format("mevent2 {} {} {}", mevent2.bstate, mevent2.y,
                            mevent2.x));
      if (mevent2.bstate & BUTTON1_RELEASED) break;
      if (mevent2.bstate & REPORT_MOUSE_POSITION) {
        int new_side_w{side_w}, new_composer_h{composer_h};
        if (where & ID_E_SIDE_MAIN) {
          new_side_w = mevent2.x + 1;
        }
        if (where & ID_E_COMP_TOP) {
          new_composer_h = LINES - mevent2.y;
        }
        resize(new_side_w, new_composer_h);
      }
    }
  } else {
    debug_log(std::format("choosing pane {}", where));
  }
}
