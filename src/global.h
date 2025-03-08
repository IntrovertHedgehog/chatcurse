#ifndef INCLUDE_SRC_GLOBAL_H_
#define INCLUDE_SRC_GLOBAL_H_

#include <curses.h>
#include <fmt/base.h>
#include <panel.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <sys/eventfd.h>

#include <initializer_list>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

#include "event_types.h"

// logging
extern std::shared_ptr<spdlog::logger> logger;

// UI
extern PANEL *composer_pan, *side_pan, *main_pan, *float_pan;
extern int side_w, composer_h;
extern int current_pan, comcurx, comcury;

// options
extern bool use_test_dc, logout_next, debug_attach;

using std::shared_ptr;
// event queue
class event_queue_struct {
  std::queue<shared_ptr<event_base>> event_queue;
  std::mutex q_mutex;

 public:
  void push(shared_ptr<event_base> e) {
    q_mutex.lock();
    event_queue.push(e);
    q_mutex.unlock();
  }

  shared_ptr<event_base> front() { return event_queue.front(); }

  shared_ptr<event_base> pop_and_get() {
    q_mutex.lock();
    shared_ptr<event_base> res;
    if (!event_queue.empty()) {
      res = std::move(event_queue.front());
      event_queue.pop();
    }
    q_mutex.unlock();
    return res;
  }

  // shared_ptr<event_base> wait_pop() {
  //   while (event_queue.empty()) {
  //   }
  //   return pop_and_get();
  // }

  void pop() {
    q_mutex.lock();
    event_queue.pop();
    q_mutex.unlock();
  }
};

extern event_queue_struct event_queue;

class app_state {
  static int constexpr _id_current_pane = 1, _id_chatboxes = 2, _id_terminating = 3;

  int _current_pane;
  std::set<std::string> _chatboxes;
  bool _terminating;
  std::unordered_map<int, shared_ptr<std::mutex>> mutexes;

 public:
  app_state()
      : mutexes{{_id_current_pane, std::make_shared<std::mutex>()},
                {_id_chatboxes, std::make_shared<std::mutex>()},
                {_id_terminating, std::make_shared<std::mutex>()}} {}
  int current_pane();
  void set_current_pane(int);
  bool terminating();
  void set_terminating(bool);
};

extern std::unordered_map<std::string, shared_ptr<app_state>>
    application_states;

#endif  // INCLUDE_SRC_GLOBAL_H_
