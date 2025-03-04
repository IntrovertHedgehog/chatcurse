#ifndef INCLUDE_SRC_EVENT_TYPES_H_
#define INCLUDE_SRC_EVENT_TYPES_H_

#define ET_QUIT 0
#define ET_RESIZE 1

struct event_base {
  int type;
  explicit event_base(int t) : type(t) {}
  event_base(const event_base&) = default;
  event_base(event_base&&) noexcept = default;
  event_base& operator=(const event_base&) = default;
  event_base& operator=(event_base&&) noexcept = default;
  virtual ~event_base() = default;
};

struct event_quit : public event_base {
  event_quit() : event_base(ET_QUIT) {}
};

struct event_resize : public event_base {
  int side_w, comp_h;
  event_resize(int w, int h) : event_base(ET_RESIZE), side_w{w}, comp_h{h} {}
};

#endif  // INCLUDE_SRC_EVENT_TYPES_H_
