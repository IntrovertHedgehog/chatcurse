#include "process_input.h"

#include <curses.h>

#include <format>
#include <memory>

#include "event_types.h"
#include "global.h"
#include "layout.h"
#include "utils.h"

struct input_state_str {
  int state = S_NONE;  // mouse_dragging, key prefix, none
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

void process_mouse(MEVENT *mevent) {
  debug_log(std::format("mouse event received ({}, {}, {}, {})", mevent->bstate,
                        mevent->y, mevent->x, mevent->z));

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
      event_queue.push(
          std::make_shared<event_resize>(new_side_w, new_composer_h));
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
    debug_log(std::format("dragging edge {}", where));
  } else {
    input_state.reset();
    debug_log(std::format("choosing pane {}", where));
  }
}

void process_input() {
  MEVENT mevent;
  int c;
  do {
    c = getch();
    switch (c) {
      case KEY_MOUSE: {
        if (getmouse(&mevent) == OK) {
          process_mouse(&mevent);
        }
        break;
      }
      case KEY_RESIZE: {
        event_queue.push(std::make_shared<event_resize>(side_w, composer_h));
        break;
      }
    }
  } while (c != CTRL('q'));
  event_queue.push(std::make_shared<event_quit>());
}
