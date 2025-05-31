#include "process_input.h"

#include <curses.h>
#include <panel.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <algorithm>
#include <csignal>
#include <cstddef>
#include <format>
#include <iterator>
#include <utility>

#include "global.h"
#include "layout.h"
#include "tg.h"
#include "utils.h"

struct input_state_str {
  int state = S_NONE; // mouse_dragging, key prefix, none
  struct {
    bool edge;
    int where;
  } mbuf;
  int pref_key;
  void reset() { state = S_NONE; }
};

input_state_str input_state;

void process_mouse(TgClient &tgcl, MEVENT *mevent) {
  if ((mevent->bstate & BUTTON1_PRESSED)) {
    process_B1_pressed(mevent);
  } else if (mevent->bstate & BUTTON1_RELEASED) {
    input_state.reset();
  } else if (mevent->bstate & REPORT_MOUSE_POSITION) {
    if (input_state.state == S_MOUSE_DRAG) {
      int new_side_w{side_w}, new_composer_h{composer_h};
      if (input_state.mbuf.where & ID_E_SIDE_MAIN) {
        new_side_w = mevent->x;
      }
      if (input_state.mbuf.where & ID_E_COMP_TOP) {
        new_composer_h = LINES - mevent->y - 1;
      }
      curs_set(0);
      resize(*tgcl.app_state, new_side_w, new_composer_h);
      update_panels();
      draw_cursor();
      doupdate();
      curs_set(1);
    }
  }
}

void process_B1_pressed(MEVENT *mevent) {
  bool &edge = input_state.mbuf.edge;
  int &where = input_state.mbuf.where;

  if (mevent->y < LINES - composer_h - 1) {
    if (mevent->x < side_w) {
      edge = false;
      where = ID_SIDE;
    } else if (mevent->x > side_w) {
      edge = false;
      where = ID_MAIN;
    } else {
      edge = true;
      where = ID_E_SIDE_MAIN;
    }
  } else if (mevent->y > LINES - composer_h - 1) {
    edge = false;
    where = ID_COMP;
  } else {
    edge = true;
    if (mevent->x == side_w) {
      where = ID_C_SIDE_MAIN_COMP;
    } else {
      where = ID_E_COMP_TOP;
    }
  }

  if (edge) {
    input_state.state = S_MOUSE_DRAG;
    logger->debug("dragging edge {}", where);
  } else {
    input_state.reset();
    logger->debug("choosing pane {}", where);
  }
}

void process_input(TgClient &tgcl, bool &cont) {
  MEVENT mevent;
  int c = getch();
  switch (c) {
  case KEY_MOUSE: {
    if (getmouse(&mevent) == OK) {
      process_mouse(tgcl, &mevent);
    }
    break;
  }
  case KEY_RESIZE: {
    logger->debug("key: resize");
    curs_set(0);
    resize(*tgcl.app_state, side_w, composer_h);
    update_panels();
    draw_cursor();
    doupdate();
    curs_set(1);
    break;
  }
  case CTRL('q'): {
    logger->debug("key: quit");
    cont = false;
    tgcl.app_state->set_terminating(true);
    break;
  }
  case '\t': {
    logger->debug("key: tab");
    current_pan <<= 1;
    if (current_pan > ID_COMP) {
      current_pan = 1;
    }
    draw_cursor();
    doupdate();
    break;
  }
  case ERR: {
    break;
  }
  default: {
    logger->debug(std::format("key: unrecognized key event {}", c));
    break;
  }
  }

  if (current_pan == ID_SIDE) {
    switch (c) {
    case 'h': {
      logger->debug("key: h");
      cursor_positions[current_pan].second =
          std::max(cursor_positions[current_pan].second - 1, 0);
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'j': {
      logger->debug("key: j");
      std::pair<int, int> &pos = cursor_positions[current_pan];
      int maxy = getmaxy(panel_window(panels[current_pan]));
      if (pos.first >= maxy - 1) {
        pos.first = maxy - 1;
        size_t max_offset =
            std::max(
                static_cast<size_t>(maxy),
                tgcl.app_state->position_sets[tgcl.app_state->current_chatlist]
                    .size()) -
            maxy;
        tgcl.app_state->scroll_offset[current_pan] = std::min(
            max_offset, tgcl.app_state->scroll_offset[current_pan] + 1);
        draw_side(*tgcl.app_state);
      } else {
        ++pos.first;
      }
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'k': {
      logger->debug("key: k");
      std::pair<int, int> &pos = cursor_positions[current_pan];
      if (pos.first <= 0) {
        pos.first = 0;
        tgcl.app_state->scroll_offset[current_pan] =
            std::max(static_cast<size_t>(1),
                     tgcl.app_state->scroll_offset[current_pan]) -
            1;
        draw_side(*tgcl.app_state);
      } else {
        --pos.first;
      }
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'l': {
      logger->debug("key: l");
      int maxx = getmaxx(panel_window(panels[current_pan]));
      cursor_positions[current_pan].second =
          std::min(cursor_positions[current_pan].second + 1, maxx - 1);
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'G': {
      logger->debug("key: G");
      int maxy = getmaxy(panel_window(panels[current_pan]));
      cursor_positions[current_pan].first = maxy - 1;
      tgcl.app_state->scroll_offset[current_pan] =
          std::max(
              static_cast<size_t>(maxy),
              tgcl.app_state->position_sets[tgcl.app_state->current_chatlist]
                  .size()) -
          maxy;
      draw_side(*tgcl.app_state);
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'g': {
      logger->debug("key: g");
      if (input_state.state == S_KEY_PREF) {
        if (input_state.pref_key == 'g') {
          tgcl.app_state->scroll_offset[current_pan] = 0;
          cursor_positions[current_pan].first = 0;
          draw_side(*tgcl.app_state);
          draw_cursor();
          wnoutrefresh(panel_window(panels[current_pan]));
          doupdate();
        }
        input_state.reset();
      } else {
        input_state.state = S_KEY_PREF;
        input_state.pref_key = 'g';
      }
      break;
    }
    case 'o': {
      logger->debug("key: o");
      auto &pos = cursor_positions[current_pan];
      if (tgcl.app_state->scroll_offset[current_pan] + pos.first <
          tgcl.app_state->position_sets[tgcl.app_state->current_chatlist]
              .size()) {
        auto it =
            tgcl.app_state->position_sets[tgcl.app_state->current_chatlist]
                .begin();
        advance(it, tgcl.app_state->scroll_offset[current_pan] + pos.first);
        tgcl.app_state->chosen_chat_id = it->second;
        draw_side(*tgcl.app_state);
        tgcl.get_chat_history(tgcl.app_state->chosen_chat_id);
        draw_cursor();
        wnoutrefresh(panel_window(panels[current_pan]));
        doupdate();
      }
    }
    case ERR: {
      break;
    }
    default: {
      logger->debug(std::format("key: unrecognized key event {}", c));
      break;
    }
    }
  } else if (current_pan == ID_MAIN) {
    switch (c) {
    case 'h': {
      logger->debug("key: h");
      cursor_positions[current_pan].second =
          std::max(cursor_positions[current_pan].second - 1, 0);
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'j': {
      logger->debug("key: j");
      std::pair<int, int> &pos = cursor_positions[current_pan];
      int maxy = getmaxy(panel_window(panels[current_pan]));
      if (pos.first >= maxy - 1) {
        pos.first = maxy - 1;
        tgcl.app_state->scroll_offset[current_pan] =
            std::max(size_t(1), tgcl.app_state->scroll_offset[current_pan]) - 1;
        draw_main(*tgcl.app_state);
      } else {
        ++pos.first;
      }
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'k': {
      logger->debug("key: k");
      std::pair<int, int> &pos = cursor_positions[current_pan];
      if (pos.first <= 0) {
        pos.first = 0;
        size_t maxy(getmaxy(panel_window(panels[current_pan])));
        size_t max_offset =
            std::max(
                tgcl.app_state->messages[tgcl.app_state->chosen_chat_id].size(),
                maxy) -
            maxy;
        tgcl.app_state->scroll_offset[current_pan] = std::min(
            max_offset, tgcl.app_state->scroll_offset[current_pan] + 1);
        draw_main(*tgcl.app_state);
      } else {
        --pos.first;
      }
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'l': {
      logger->debug("key: l");
      int maxx = getmaxx(panel_window(panels[current_pan]));
      cursor_positions[current_pan].second =
          std::min(cursor_positions[current_pan].second + 1, maxx - 1);
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'G': {
      logger->debug("key: G");
      int maxy = getmaxy(panel_window(panels[current_pan]));
      cursor_positions[current_pan].first = maxy - 1;
      tgcl.app_state->scroll_offset[current_pan] =
          std::max(
              static_cast<size_t>(maxy),
              tgcl.app_state->position_sets[tgcl.app_state->current_chatlist]
                  .size()) -
          maxy;
      draw_side(*tgcl.app_state);
      draw_cursor();
      wnoutrefresh(panel_window(panels[current_pan]));
      doupdate();
      break;
    }
    case 'g': {
      logger->debug("key: g");
      if (input_state.state == S_KEY_PREF) {
        if (input_state.pref_key == 'g') {
          tgcl.app_state->scroll_offset[current_pan] = 0;
          cursor_positions[current_pan].first = 0;
          draw_side(*tgcl.app_state);
          draw_cursor();
          wnoutrefresh(panel_window(panels[current_pan]));
          doupdate();
        }
        input_state.reset();
      } else {
        input_state.state = S_KEY_PREF;
        input_state.pref_key = 'g';
      }
      break;
    }
    case 'o': {
      logger->debug("key: o");
      auto &pos = cursor_positions[current_pan];
      if (tgcl.app_state->scroll_offset[current_pan] + pos.first <
          tgcl.app_state->position_sets[tgcl.app_state->current_chatlist]
              .size()) {
        auto it =
            tgcl.app_state->position_sets[tgcl.app_state->current_chatlist]
                .begin();
        advance(it, tgcl.app_state->scroll_offset[current_pan] + pos.first);
        tgcl.app_state->chosen_chat_id = it->second;
        draw_side(*tgcl.app_state);
        tgcl.get_chat_history(tgcl.app_state->chosen_chat_id);
        draw_cursor();
        wnoutrefresh(panel_window(panels[current_pan]));
        doupdate();
      }
    }
    case ERR: {
      break;
    }
    default: {
      logger->debug(std::format("key: unrecognized key event {}", c));
      break;
    }
    }
  }
}
