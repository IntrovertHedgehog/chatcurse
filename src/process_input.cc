#include "process_input.h"

#include <curses.h>

#include <format>
#include <memory>

#include "event_types.h"
#include "global.h"
#include "layout.h"
#include "utils.h"

struct input_state_str {
  int state = S_NONE; // mouse_dragging, key prefix, none
  std::vector<int> kbuf;
  struct {
    bool edge;
    int where;
  } mbuf;
  void reset() {
    state = S_NONE;
    kbuf.clear();
  }
};

input_state_str input_state;

void process_mouse(tl_app_state_struct &app_state, MEVENT *mevent) {
  if ((mevent->bstate & BUTTON1_PRESSED)) {
    process_B1_pressed(mevent);
  } else if (mevent->bstate & BUTTON1_RELEASED) {
    input_state.reset();
  } else if (mevent->bstate & REPORT_MOUSE_POSITION) {
    if (input_state.state == S_MOUSE_DRAG) {
      int new_side_w{side_w}, new_composer_h{composer_h};
      if (input_state.mbuf.where & ID_E_SIDE_MAIN) {
        new_side_w = mevent->x + 1;
      }
      if (input_state.mbuf.where & ID_E_COMP_TOP) {
        new_composer_h = LINES - mevent->y;
      }
      resize(app_state, new_side_w, new_composer_h);
      update_panels();
      draw_cursor();
      doupdate();
    }
  }
}

void process_B1_pressed(MEVENT *mevent) {
  bool &edge = input_state.mbuf.edge;
  int &where = input_state.mbuf.where;

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
    input_state.state = S_MOUSE_DRAG;
    logger->debug("dragging edge {}", where);
  } else {
    input_state.reset();
    logger->debug("choosing pane {}", where);
  }
}

void process_input(tl_app_state_struct &app_state, bool &cont) {
  MEVENT mevent;
  int c = getch();
  switch (c) {
  case KEY_MOUSE: {
    if (getmouse(&mevent) == OK) {
      process_mouse(app_state, &mevent);
    }
    break;
  }
  case KEY_RESIZE: {
    logger->debug("key: resize");
    resize(app_state, side_w, composer_h);
    update_panels();
    draw_cursor();
    doupdate();
    break;
  }
  case CTRL('q'): {
    logger->debug("key: quit");
    cont = false;
    app_state.set_terminating(true);
    break;
  }
  case '\t': {
    logger->debug("key: tab");
    current_pan <<= 1;
    if (current_pan > ID_COMP) {
      current_pan = 1;
    }
    draw_cursor();
    doupdate();
    break;
  }
  case ERR: {
    break;
  }
  default: {
    logger->debug(std::format("key: unrecognized key event {}", c));
    break;
  }
  }
}
