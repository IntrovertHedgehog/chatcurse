#ifndef INCLUDE_SRC_GLOBAL_H_
#define INCLUDE_SRC_GLOBAL_H_

#include <curses.h>
#include <fmt/base.h>
#include <panel.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <sys/eventfd.h>

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <unordered_map>
#include <utility>

#include "event_types.h"
#include "td/telegram/td_api.h"

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
  std::queue<int> queue;
  std::map<int, shared_ptr<event_base>> event_map;
  std::mutex q_mutex;

 public:
  void push(shared_ptr<event_base> e);
  shared_ptr<event_base> pop_and_get();
};

extern event_queue_struct event_queue;
using td::td_api::object_ptr;

class tl_app_state_struct {
  static int constexpr _id_current_pane = 1, _id_chatboxes = 2,
                       _id_terminating = 3;

  int _current_pane;
  bool _terminating;

  std::unordered_map<int, shared_ptr<std::mutex>> mutexes;

  // TODO(hedgehog): switch to shared_ptr and thread protect these
 public:
  std::unordered_map<td::td_api::int53, object_ptr<td::td_api::user>>
      id_to_user;
  std::unordered_map<td::td_api::int53, object_ptr<td::td_api::userFullInfo>>
      id_to_user_full_info;
  std::unordered_map<td::td_api::int53, object_ptr<td::td_api::chat>>
      id_to_chat;
  std::unordered_map<td::td_api::int53, object_ptr<td::td_api::basicGroup>>
      id_to_basicgroup;
  std::unordered_map<td::td_api::int53,
                     object_ptr<td::td_api::basicGroupFullInfo>>
      id_to_basicgroup_full_info;
  std::unordered_map<td::td_api::int53, object_ptr<td::td_api::supergroup>>
      id_to_supergroup;
  std::unordered_map<td::td_api::int53,
                     object_ptr<td::td_api::supergroupFullInfo>>
      id_to_supergroup_full_info;
  
  // class ID (for list type) -> chat id -> chat position
  typedef std::pair<int64_t, td::td_api::int53> p64_53;
  std::unordered_map<int32_t, std::set<p64_53, std::greater<p64_53>>>
      position_sets;
  std::unordered_map<int32_t, std::unordered_map<td::td_api::int53, int64_t>>
      id_to_position;

  tl_app_state_struct()
      : mutexes{{_id_current_pane, std::make_shared<std::mutex>()},
                {_id_chatboxes, std::make_shared<std::mutex>()},
                {_id_terminating, std::make_shared<std::mutex>()}} {}
  int current_pane();
  void set_current_pane(int);
  bool terminating();
  void set_terminating(bool);
};

// extern tl_app_state_struct tl_app_state;

// extern std::unordered_map<std::string, shared_ptr<app_state>>
//     application_states;

#endif  // INCLUDE_SRC_GLOBAL_H_
