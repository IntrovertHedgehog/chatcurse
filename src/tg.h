#ifndef INCLUDE_SRC_TG_H_
#define INCLUDE_SRC_TG_H_

#include <curses.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <ostream>

#include "td/telegram/Client.h"
#include "td/telegram/td_api.h"
#include "td/telegram/td_api.hpp"

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

class TgClient {
  std::unique_ptr<td::ClientManager> client_manager_;
  std::int32_t client_id_{0};
  td::td_api::object_ptr<td::td_api::AuthorizationState> authorization_state_;
  bool is_auth_{false};
  bool need_restart_{false};
  std::uint64_t current_query_id_{0};
  std::uint64_t auth_query_id{0};
  std::map<std::uint64_t,
           std::function<void(td::td_api::object_ptr<td::td_api::Object>)>>
      handlers_;

 public:
  TgClient() {
    td::ClientManager::execute(
        td::td_api::make_object<td::td_api::setLogVerbosityLevel>(1));
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
    send_query(td_api::make_object<td_api::getOption>("version"),
               [](td::td_api::object_ptr<td::td_api::Object> res) {
                 td::td_api::downcast_call(
                     *res, overloaded(
                               [](td_api::optionValueString &str) {
                                 std::cout << str.value_ << std::endl;
                               },
                               [](auto &) {}));
               });
    process_response(client_manager_->receive(3));
  }

  void send_query(
      td_api::object_ptr<td_api::Function> f,
      std::function<void(td::td_api::object_ptr<td::td_api::Object>)> handler) {
    auto query_id = next_query_id();
    if (handler) {
      handlers_.emplace(query_id, std::move(handler));
    }
    client_manager_->send(client_id_, query_id, std::move(f));
  }

  void process_response(td::ClientManager::Response response) {
    if (!response.object) {
      return;
    }
    std::cout << response.request_id << ":"
              << td::td_api::to_string(response.object) << std::endl;
    if (response.request_id == 0) {
      return process_update(std::move(response.object));
    }
    auto handler = handlers_.find(response.request_id);
    if (handler != handlers_.end()) {
      handler->second(std::move(response.object));
      handlers_.erase(handler);
    }
  }

  void process_update(td::td_api::object_ptr<td::td_api::Object> object) {
    std::cout << "update season" << std::endl;
  }

  td::td_api::int32 next_query_id() { return ++current_query_id_; }
};

#endif  // INCLUDE_SRC_TG_H_
