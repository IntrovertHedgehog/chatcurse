#include "tg.h"

#include <iostream>

#include "global.h"

TgClient::TgClient() {
  td::ClientManager::execute(
      td::td_api::make_object<td::td_api::setLogVerbosityLevel>(1));
  client_manager_ = std::make_unique<td::ClientManager>();
  client_id_ = client_manager_->create_client_id();
  send_query(td_api::make_object<td_api::getOption>("version"), {});
}

void TgClient::init_auth() {
  while (true) {
    if (need_restart_) {
      client_manager_.reset();
      *this = TgClient();
      std::cout << "restarted successfully";
    } else if (!is_auth_) {
      process_response(client_manager_->receive(10));
    } else {
      break;
    }
  }
}

void TgClient::send_query(
    td_api::object_ptr<td_api::Function> f,
    std::function<void(td::td_api::object_ptr<td::td_api::Object>)> handler) {
  auto query_id = next_query_id();
  debug_log("send " + std::to_string(query_id) + ":" +
            td::td_api::to_string(f));
  if (handler) {
    handlers_.emplace(query_id, std::move(handler));
  }
  client_manager_->send(client_id_, query_id, std::move(f));
}

void TgClient::process_response(td::ClientManager::Response response) {
  if (!response.object) {
    return;
  }
  debug_log("reiv " + std::to_string(response.request_id) + ":" +
            td::td_api::to_string(response.object));
  if (response.request_id == 0) {
    return process_update(std::move(response.object));
  }
  auto handler = handlers_.find(response.request_id);
  if (handler != handlers_.end()) {
    handler->second(std::move(response.object));
    handlers_.erase(handler);
  }
}

void TgClient::process_update(
    td::td_api::object_ptr<td::td_api::Object> object) {
  td::td_api::downcast_call(
      *object, overloaded(
                   [this](td::td_api::updateAuthorizationState &u) {
                     auth_state_ = std::move(u.authorization_state_);
                     process_auth();
                   },
                   [](auto &) {}));
}

void TgClient::process_auth() {
  auth_query_id_++;
  td::td_api::downcast_call(
      *auth_state_,
      overloaded(
          [this](td::td_api::authorizationStateWaitTdlibParameters &) {
            td::td_api::object_ptr<td::td_api::setTdlibParameters> req =
                td::td_api::make_object<td::td_api::setTdlibParameters>();
            req->use_test_dc_ = use_test_dc;  // TODO(hedgehog): make option
            req->database_directory_ = "tmp/db";
            req->use_message_database_ = true;
            req->use_secret_chats_ = true;
            req->api_id_ = std::stoi(getenv("TG_API_ID"));
            req->api_hash_ = getenv("TG_API_HASH");
            req->system_language_code_ = "en";
            req->device_model_ = "Desktop";
            req->application_version_ = "1.0";
            send_query(std::move(req), auth_query_handler());
          },
          [this](td::td_api::authorizationStateWaitPhoneNumber &) {
            logout_next = false;
            std::string phone_no;
            std::cout << "phone number pls (put 0 for QR): ";
            std::cin >> phone_no;
            std::cout << "got the phone number '" + phone_no + "'" << std::endl;
            if (phone_no != "0") {
              send_query(
                  td::td_api::make_object<
                      td::td_api::setAuthenticationPhoneNumber>(
                      phone_no,
                      td::td_api::object_ptr<
                          td::td_api::phoneNumberAuthenticationSettings>()),
                  auth_query_handler());
            } else {
              send_query(td::td_api::make_object<
                             td::td_api::requestQrCodeAuthentication>(),
                         auth_query_handler());
            }
          },
          [this](td::td_api::authorizationStateWaitCode &) {
            logout_next = false;
            std::string code;
            std::cout << "code pls: ";
            std::cin >> code;
            send_query(
                td::td_api::make_object<td::td_api::checkAuthenticationCode>(
                    code),
                auth_query_handler());
          },
          [this](td::td_api::authorizationStateReady &) {
            if (logout_next) {
              logout_next = false;
              send_query(td::td_api::make_object<td::td_api::logOut>(), {});
            } else {
              is_auth_ = true;
            }
          },
          [this](td::td_api::authorizationStateWaitEmailAddress &) {
            logout_next = false;
            std::string email;
            std::cout << "Email pls: ";
            std::cin >> email;
            send_query(td::td_api::make_object<
                           td::td_api::setAuthenticationEmailAddress>(email),
                       auth_query_handler());
          },
          [this](td::td_api::authorizationStateWaitEmailCode &) {
            logout_next = false;
            std::string code;
            std::cout << "code pls: ";
            std::cin >> code;
            send_query(
                td::td_api::make_object<
                    td::td_api::checkAuthenticationEmailCode>(
                    td::td_api::make_object<
                        td::td_api::emailAddressAuthenticationCode>(code)),
                auth_query_handler());
          },
          [this](td::td_api::authorizationStateWaitRegistration &) {
            logout_next = false;
            std::string first_name, last_name;
            std::cout << "First name pls: ";
            std::cin >> first_name;
            std::cout << "Last name pls:";
            std::cin >> last_name;
            send_query(td::td_api::make_object<td::td_api::registerUser>(
                           first_name, last_name, false),
                       auth_query_handler());
          },
          [](td::td_api::authorizationStateWaitOtherDeviceConfirmation &u) {
            logout_next = false;
            std::cout << "Scan QR with an active Telegram session \n";
            qrcodegen::display(
                std::cout,
                qrcodegen::QrCode::encodeText(
                    u.link_.c_str(), qrcodegen::QrCode::Ecc ::QUARTILE));
            std::cout << "login link: " << u.link_ << std::endl;
          },
          [this](td::td_api::authorizationStateLoggingOut &) {
            is_auth_ = false;
          },
          [](td::td_api::authorizationStateClosing &) {},
          [this](td::td_api::authorizationStateClosed &) {
            is_auth_ = false;
            need_restart_ = true;
          },
          [](auto &) {}));
}

void TgClient::init_event_handlers() {

}
