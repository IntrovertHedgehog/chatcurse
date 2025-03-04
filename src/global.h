#ifndef INCLUDE_SRC_GLOBAL_H_
#define INCLUDE_SRC_GLOBAL_H_

#include <curses.h>
#include <panel.h>
#include <sys/eventfd.h>

#include <fstream>
#include <memory>
#include <mutex>
#include <queue>

#include "event_types.h"

// debugging
extern std::ofstream log_os;

// UI
extern PANEL *composer_pan, *side_pan, *main_pan, *float_pan;
extern int side_w, composer_h;
extern int current_pan, comcurx, comcury;

// options
extern bool use_test_dc, logout_next;

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
    shared_ptr<event_base> res = event_queue.front();
    event_queue.pop();
    q_mutex.unlock();
    return res;
  }

  shared_ptr<event_base> wait_pop() {
    while (event_queue.empty()) {
    }
    return pop_and_get();
  }

  void pop() {
    q_mutex.lock();
    event_queue.pop();
    q_mutex.unlock();
  }
};

extern event_queue_struct event_queue;

#endif  // INCLUDE_SRC_GLOBAL_H_
