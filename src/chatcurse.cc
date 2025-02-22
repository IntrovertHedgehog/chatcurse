#include <curses.h>
#include <panel.h>
#include <time.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <ios>
#include <iostream>
#include <string>

#include "global.h"
#include "layout.h"
#include "process_input.h"
#include "tg.h"
#include "utils.h"

using std::min;
using std::string;

int main() {
  TgClient tgcl;

  int ch;
  std::cin >> ch;

  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  mmask_t old_mm, new_mm = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;
  mousemask(new_mm, &old_mm);
  printf("\033[?1003h\n");  // magic for x-based terminal
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
        break;
      }
      case KEY_RESIZE: {
        resize(side_w, composer_h);
        break;
      }
    }
    doupdate();
    c = getch();
  }

  printf("\033[?1003l\n");  // reset magic
  mousemask(old_mm, NULL);
  endwin();
}

void init_config() {
  log_os.open("tmp/debug.log", std::ios_base::out);
  side_w = min(32, COLS / 4);
  composer_h = min(6, LINES / 5);
}
