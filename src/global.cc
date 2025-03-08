#include "global.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>
#include <mutex>

std::shared_ptr<spdlog::logger> logger;
PANEL *composer_pan, *side_pan, *main_pan, *float_pan;
int side_w, composer_h;
int current_pan, comcurx, comcury;
bool use_test_dc = false, logout_next = false, debug_attach = false;
event_queue_struct event_queue;

int tl_app_state_struct::current_pane() {
  std::lock_guard<std::mutex> g(*mutexes[_id_current_pane]);
  return _current_pane;
}

void tl_app_state_struct::set_current_pane(int e) {
  std::lock_guard<std::mutex> g(*mutexes[_id_current_pane]);
  _current_pane = e;
}

bool tl_app_state_struct::terminating() {
  std::lock_guard<std::mutex> g(*mutexes[_id_terminating]);
  return _terminating;
}

void tl_app_state_struct::set_terminating(bool e) {
  std::lock_guard<std::mutex> g(*mutexes[_id_terminating]);
  _terminating = e;
}

// tl_app_state_struct tl_app_state;

// std::unordered_map<std::string, shared_ptr<app_state>> application_states;
