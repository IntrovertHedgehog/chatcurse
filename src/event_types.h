#ifndef INCLUDE_SRC_EVENT_TYPES_H_
#define INCLUDE_SRC_EVENT_TYPES_H_

#include "td/telegram/td_api.h"
#define ET_QUIT 0
#define ET_RESIZE 1
#define ET_CHATLIST 2
#define ET_CYCLE_PANEL 3
#define ET_MESSAGES 4

struct event_base {
  int type;
  explicit event_base(int t) : type(t) {}
  event_base(const event_base &) = default;
  event_base(event_base &&) noexcept = default;
  event_base &operator=(const event_base &) = default;
  event_base &operator=(event_base &&) noexcept = default;
  virtual ~event_base() = default;
};

struct event_quit : public event_base {
  event_quit() : event_base(ET_QUIT) {}
};

struct event_cycle_panel : public event_base {
  event_cycle_panel() : event_base(ET_CYCLE_PANEL) {}
};

struct event_resize : public event_base {
  int side_w, comp_h;
  event_resize(int w, int h) : event_base(ET_RESIZE), side_w{w}, comp_h{h} {}
};

struct event_chatlist : public event_base {
  event_chatlist() : event_base(ET_CHATLIST) {}
};

struct event_messages : public event_base {
  td::td_api::int53 chat_id_;
  event_messages(td::td_api::int53 chat_id)
      : event_base(ET_MESSAGES), chat_id_(chat_id) {}
};

#endif // INCLUDE_SRC_EVENT_TYPES_H_
