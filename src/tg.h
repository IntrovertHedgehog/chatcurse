#ifndef INCLUDE_SRC_TG_H_
#define INCLUDE_SRC_TG_H_

#include <curses.h>
#include <memory.h>

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <thread>
#include <utility>

#include "global.h"
#include "qrcodegen.hpp"
#include "td/telegram/Client.h"
#include "td/telegram/td_api.h"
#include "td/telegram/td_api.hpp"
#include "utils.h"

namespace detail {
template <class... Fs>
struct overload;

template <class F>
struct overload<F> : public F {
  explicit overload(F f) : F(f) {}
};
template <class F, class... Fs>
struct overload<F, Fs...> : public overload<F>, public overload<Fs...> {
  overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {}
  using overload<F>::operator();
  using overload<Fs...>::operator();
};
}  // namespace detail

template <class... F>
auto overloaded(F... f) {
  return detail::overload<F...>(f...);
}

namespace td_api = td::td_api;

class TgData {};

class TgClient {
  std::unique_ptr<td::ClientManager> client_manager_;
  std::int32_t client_id_{0};
  td::td_api::object_ptr<td::td_api::AuthorizationState> auth_state_;
  bool is_auth_{false};
  bool need_restart_{false};
  std::uint64_t current_query_id_{0};
  std::uint64_t auth_query_id_{0};
  std::map<std::uint64_t,
           std::function<void(td::td_api::object_ptr<td::td_api::Object>)>>
      handlers_;

  void _update_position(td::td_api::int53, td::td_api::chatPosition &);

 public:
  shared_ptr<tl_app_state_struct> app_state;
  friend void call_init_event_handler(TgClient &cl);
  TgClient();
  // initialize authentication and setup db
  // at then end of this function it's safe to show the ui
  void init_auth();
  // set up handler and listener
  void set_response_handlers();
  td::td_api::int32 next_query_id() { return ++current_query_id_; }
  void send_query(
      td_api::object_ptr<td_api::Function> f,
      std::function<void(td::td_api::object_ptr<td::td_api::Object>)> handler);
  void process_response(td::ClientManager::Response response);
  void process_update(td::td_api::object_ptr<td::td_api::Object> object);
  auto auth_query_handler() {
    return [this,
            id = auth_query_id_](td::td_api::object_ptr<td::td_api::Object> o) {
      if (id == auth_query_id_) {
        if (o->get_id() == td::td_api::error::ID) {
          process_auth();
        }
      }
    };
  }
  void process_auth();
  void process_update_user(td::td_api::updateUser &);
  void process_update_user_full_info(td::td_api::updateUserFullInfo &);
  void process_update_new_chat(td::td_api::updateNewChat &);
  void process_update_basicgroup(td::td_api::updateBasicGroup &);
  void process_update_basicgroup_full_info(
      td::td_api::updateBasicGroupFullInfo &);
  void process_update_supergroup(td::td_api::updateSupergroup &);
  void process_update_supergroup_full_info(
      td::td_api::updateSupergroupFullInfo &);
  void process_update_chat_added_to_list(td::td_api::updateChatAddedToList &);
  void process_update_chat_position(td::td_api::updateChatPosition &);
  void process_update_chat_last_message(td::td_api::updateChatLastMessage &);
};

#endif  // INCLUDE_SRC_TG_H_
