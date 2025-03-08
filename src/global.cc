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

int app_state::current_pane() {
  std::lock_guard<std::mutex> g(*mutexes[_id_current_pane]);
  return _current_pane;
}

void app_state::set_current_pane(int e) {
  std::lock_guard<std::mutex> g(*mutexes[_id_current_pane]);
  _current_pane = e;
}

bool app_state::terminating() {
  std::lock_guard<std::mutex> g(*mutexes[_id_terminating]);
  return _terminating;
}

void app_state::set_terminating(bool e) {
  std::lock_guard<std::mutex> g(*mutexes[_id_terminating]);
  _terminating = e;
}

std::unordered_map<std::string, shared_ptr<app_state>> application_states;
