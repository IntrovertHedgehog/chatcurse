#include <curses.h>
#include <linux/prctl.h>
#include <panel.h>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include <cctype>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <functional>
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

using std::string;

int main(int argv, char **argc) {
  std::cout << "Starting chatcurse..." << std::endl;

  try {
    logger = spdlog::basic_logger_mt("chatcurse", "tmp/debug.log", true);
    logger->flush_on(spdlog::level::info);
  } catch (spdlog::spdlog_ex &e) {
    std::cerr << "spdlog error: " << e.what() << std::endl;
  }

  for (int i = 1; i < argv; ++i) {
    logger->info("option " + string(argc[i]));
    if (strcmp(argc[i], "--debug") == 0) {
      logger->set_level(spdlog::level::debug);
      logger->flush_on(spdlog::level::debug);
    } else if (strcmp(argc[i], "--attach") == 0) {
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
    char c;
    std::cout << "waiting for attachment, press enter to proceed..."
              << std::endl;
    while ((c = std::cin.get()) != 10) {
      std::cout << static_cast<int>(c) << std::endl;
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
  mouseinterval(0);        // do not distinguish click from press
  printf("\033[?1003h\n"); // magic for x-based terminal

  // reset magic when terminate in any case
  auto term = [](int s) {
    logger->error("signal {}, terminating...", s);
    printf("\033[?1003l\n");
    fflush(stdout);
    endwin();
    SIG_DFL(s);
  };

  for (int s : {SIGTERM, SIGABRT, SIGILL, SIGINT, SIGSEGV, SIGFPE})
    signal(s, term);

  // required, otherwise panel library display incorrectly
  refresh();

  init_config();
  init_layout(*tgcl.app_state);

  // initial data display (chatlist, messages, etc.)
  {
    int chat_list_size = getmaxy(panel_window(panels[ID_SIDE]));
    tgcl.init_data(chat_list_size);
  }

  logger->info("initialization finished");

  // spawn thread to process tg input
  // ref(tgcl) -> reference_wrapper, otherwise thread constructor will copy
  // construct the TgClient object
  std::thread tgcl_thread(&TgClient::set_response_handlers, std::ref(tgcl));

  bool cont = true;
  while (cont) {
    // update UI every loop
    process_input(*tgcl.app_state, cont);
    // TODO(hedgehog): do all update before moving on getting inputs
    shared_ptr<event_base> to_update = event_queue.pop_and_get();
    if (!to_update) {
      // logger->debug("nothing to update");
      continue;
    }
    logger->debug("updating: {}", to_update->type);
    switch (to_update->type) {
    case ET_QUIT: {
      logger->debug("ET_QUIT");
      cont = false;
      tgcl.app_state->set_terminating(true);
      break;
    }
    case ET_RESIZE: {
      logger->debug("ET_RESIZE");
      shared_ptr<event_resize> ev =
          std::dynamic_pointer_cast<event_resize>(to_update);
      resize(*tgcl.app_state, ev->side_w, ev->comp_h);
      update_panels();
      draw_cursor();
      doupdate();
      break;
    }
    case ET_CHATLIST: {
      logger->debug("ET_CHATLIST");
      draw_side(*tgcl.app_state);
      update_panels();
      draw_cursor();
      doupdate();
      break;
    }
    case ET_CYCLE_PANEL: {
      logger->debug("ET_CYCLE_PANEL");
      current_pan <<= 1;
      if (current_pan > ID_COMP) {
        current_pan = 1;
      }
      draw_cursor();
      doupdate();
      break;
    }
    }
  }

  logger->info("quit main loop");
  tgcl_thread.join();

  printf("\033[?1003l\n"); // reset magic
  mousemask(old_mm, NULL);
  endwin();

  return 0;
}
