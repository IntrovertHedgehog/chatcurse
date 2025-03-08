#include <curses.h>
#include <linux/prctl.h>
#include <panel.h>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <time.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception>
#include <format>
#include <functional>
#include <ios>
#include <iostream>
#include <memory>
#include <ostream>
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
  std::cout << "Starting chatcurse..." << std::endl;

  try {
    logger = spdlog::basic_logger_mt("chatcurse", "tmp/debug.log");
    logger->flush_on(spdlog::level::info);
  } catch (spdlog::spdlog_ex& e) {
    std::cerr << "spdlog error: " << e.what() << std::endl;
  }

  for (int i = 1; i < argv; ++i) {
    logger->info("option " + string(argc[i]));
    if (strcmp(argc[i], "--debug-attach") == 0) {
      debug_attach = true;
      prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
    } else if (strcmp(argc[i], "--use-test-dc") == 0) {
      use_test_dc = true;
    } else if (strcmp(argc[i], "--logout") == 0) {
      logout_next = true;
    } else {
      std::cerr << "invalid options '" << argc[i] << "'\n";
      exit(1);
    }
  }

  if (debug_attach) {
    int c;
    std::cout << "waiting for attachment, press enter to proceed..."
              << std::endl;
    while ((c = std::cin.get()) != 10) {
    }
  }

  application_states["tg"] = std::make_shared<app_state>();

  // authorization
  TgClient tgcl;
  tgcl.init_auth();

  // setting up ui
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();
  timeout(100);

  mmask_t old_mm, new_mm = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;
  mousemask(new_mm, &old_mm);
  mouseinterval(0);         // do not distinguish click from press
  printf("\033[?1003h\n");  // magic for x-based terminal

  // reset magic when terminate in any case
  auto term = [](int s) {
    printf("\033[?1003l\n");
    SIG_DFL(s);
  };

  for (int s : {SIGTERM, SIGABRT, SIGILL, SIGINT, SIGSEGV, SIGFPE})
    signal(s, term);

  // required, otherwise panel library display incorrectly
  refresh();

  init_config();
  init_layout();

  logger->info("initialization finished");

  // in app

  // spawn thread to process tg input
  std::thread tgcl_thread(&TgClient::set_response_handlers, std::ref(tgcl));

  bool cont = true;
  while (cont) {
    // update UI every loop
    process_input();
    shared_ptr<event_base> to_update = event_queue.pop_and_get();
    if (!to_update) continue;
    switch (to_update->type) {
      case ET_QUIT: {
        cont = false;
        application_states["tg"]->set_terminating(true);
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
  tgcl_thread.join();

  printf("\033[?1003l\n");  // reset magic
  mousemask(old_mm, NULL);
  endwin();

  return 0;
}

void init_config() {
  side_w = min(32, COLS / 4);
  composer_h = min(6, LINES / 5);
}
