#include <curses.h>
#include <linux/prctl.h>
#include <panel.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <time.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <format>
#include <ios>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "event_types.h"
#include "global.h"
#include "layout.h"
#include "process_input.h"
#include "tg.h"
#include "utils.h"

using std::min;
using std::string;

int main(int argv, char** argc) {
  log_os.open("tmp/debug.log", std::ios_base::out);
  std::cout << "Starting chatcurse..." << std::endl;

  for (int i = 1; i < argv; ++i) {
    debug_log("option " + string(argc[i]));
    if (strcmp(argc[i], "--debug") == 0) {
      prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
    } else if (strcmp(argc[i], "--test") == 0) {
      use_test_dc = true;
    } else if (strcmp(argc[i], "--logout") == 0) {
      logout_next = true;
    } else {
      std::cerr << "invalid options '" << argc[i] << "'\n";
      exit(1);
    }
  }

  // authorization
  TgClient tgcl;
  tgcl.init_auth();

  // setting up ui
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();
  timeout(0);

  mmask_t old_mm, new_mm = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;
  mousemask(new_mm, &old_mm);
  printf("\033[?1003h\n");  // magic for x-based terminal
  mouseinterval(0);

  // required, otherwise panel library display incorrectly
  refresh();

  init_config();
  init_layout();

  debug_log("initialization finished");

  // in app

  // spawn thread to process user input -> become single thread
  // std::thread input_thread = std::thread(process_input);
  // spawn thread to process tg input

  bool cont = true;
  while (cont) {
    // update UI every loop
    process_input();
    shared_ptr<event_base> to_update = event_queue.pop_and_get();
    if (!to_update) continue;
    switch (to_update->type) {
      case ET_QUIT: {
        cont = false;
        break;
      }
      case ET_RESIZE: {
        shared_ptr<event_resize> ev =
            std::dynamic_pointer_cast<event_resize>(to_update);
        resize(ev->side_w, ev->comp_h);
        break;
      }
    }
  }

  // input_thread.join();

  printf("\033[?1003l\n");  // reset magic
  mousemask(old_mm, NULL);
  endwin();

  return 0;
}

void init_config() {
  side_w = min(32, COLS / 4);
  composer_h = min(6, LINES / 5);
}
