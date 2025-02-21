#include <curses.h>

#include <cmath>
#include <ctime>
#include <format>
#include <ios>
#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

#include <ncurses.h>
#include <panel.h>
#include <time.h>

#include <algorithm>
#include <cstdio>
#include <string>

#include "global.h"

using std::min;
using std::string;

void init_config();
void init_layout();
void draw_border();
void resize(int new_side_w, int new_composer_h);
void debug_log(string const &);
void process_mouse(MEVENT *);
void process_B1_pressed(MEVENT *);

int main() {
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  mmask_t old_mm, new_mm = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;
  mousemask(new_mm, &old_mm);
  mouseinterval(0);

  refresh();

  init_config();
  init_layout();

  debug_log("initialization finished");
  // in app

  int c = getch();
  MEVENT mevent;

  while (c != CTRL('q')) {
    switch (c) {
      case KEY_MOUSE: {
        if (getmouse(&mevent) == OK) {
          process_mouse(&mevent);
        }
      }
    }
    doupdate();
    c = getch();
  }

  mousemask(old_mm, NULL);
  endwin();
}

void init_config() {
  log_os.open("tmp/debug.log", std::ios_base::out);
  side_w = min(32, COLS / 4);
  composer_h = min(6, LINES / 6);
}

void init_layout() {
  WINDOW *side_win = newwin(LINES - composer_h, side_w, 0, 0),
         *main_win = newwin(LINES - composer_h, COLS - side_w, 0, side_w),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0);

  side_pan = new_panel(side_win);
  main_pan = new_panel(main_win);
  composer_pan = new_panel(composer_win);
  draw_border();

  update_panels();
  doupdate();
}

void draw_border() {
  wborder(panel_window(side_pan), ' ', 0, ' ', ' ', ' ', ACS_VLINE, ' ',
          ACS_VLINE);
  wborder(panel_window(composer_pan), ' ', ' ', ACS_HLINE, ' ', ACS_HLINE,
          ACS_HLINE, ' ', ' ');
  mvwaddch(panel_window(composer_pan), 0, side_w - 1, ACS_SSBS);
}

void resize(int new_side_w, int new_composer_h) {
  debug_log(std::format("new size ({}, {})", new_side_w, new_composer_h));

  if (new_side_w >= COLS) return;
  if (new_composer_h >= LINES) return;

  side_w = new_side_w;
  composer_h = new_composer_h;

  WINDOW *side_win = newwin(LINES - composer_h, side_w, 0, 0),
         *main_win = newwin(LINES - composer_h, COLS - side_w, 0, side_w),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0),
         *old_side_win = panel_window(side_pan),
         *old_main_win = panel_window(main_pan),
         *old_composer_win = panel_window(composer_pan);

  replace_panel(side_pan, side_win);
  replace_panel(main_pan, main_win);
  replace_panel(composer_pan, composer_win);

  delwin(old_side_win);
  delwin(old_main_win);
  delwin(old_composer_win);

  draw_border();
  update_panels();
  doupdate();
}

void debug_log(string const &msg) {
  time_t t = time(NULL);
  log_os << std::asctime(std::localtime(&t)) << msg << std::endl;
}

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
    printf("\033[?1003h\n");  // magic for x-based terminal
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
    printf("\033[?1003l\n");  // reset magic
  } else {
    debug_log(std::format("choosing pane {}", where));
  }
}

void fill() {}
