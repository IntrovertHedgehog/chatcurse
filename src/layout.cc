#include "layout.h"

#include <curses.h>
#include <panel.h>

#include <algorithm>
#include <cstddef>
#include <format>
#include <mutex>
#include <string>
#include <utility>

#include "global.h"
#include "td/telegram/td_api.h"
#include "utils.h"

void init_layout(tl_app_state_struct &app_state) {
  WINDOW *side_win = newwin(LINES - composer_h, side_w, 0, 0),
         *main_win = newwin(LINES - composer_h, COLS - side_w, 0, side_w),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0);

  panels[ID_SIDE] = new_panel(side_win);
  panels[ID_MAIN] = new_panel(main_win);
  panels[ID_COMP] = new_panel(composer_win);

  cursor_positions[ID_COMP] = {1, 0};
  cursor_positions[ID_SIDE] = {0, 0};
  cursor_positions[ID_MAIN] = {0, 0};
  cursor_positions[ID_FLOAT] = {0, 0};

  current_pan = ID_COMP;

  draw_border();
  fill(app_state);
  update_panels();
  doupdate();
}

// always draw borders before anything else
void draw_border() {
  wborder(panel_window(panels[ID_SIDE]), ' ', 0, ' ', ' ', ' ', ACS_VLINE, ' ',
          ACS_VLINE);
  wborder(panel_window(panels[ID_COMP]), ' ', ' ', ACS_HLINE, ' ', ACS_HLINE,
          ACS_HLINE, ' ', ' ');
  mvwaddch(panel_window(panels[ID_COMP]), 0, side_w - 1, ACS_SSBS);
}

void resize(tl_app_state_struct &app_state, int new_side_w,
            int new_composer_h) {
  logger->info(std::format("new size ({}, {})", new_side_w, new_composer_h));

  if (new_side_w >= COLS)
    return;
  if (new_composer_h >= LINES)
    return;

  side_w = new_side_w;
  composer_h = new_composer_h;

  WINDOW *side_win = newwin(LINES - composer_h, side_w, 0, 0),
         *main_win = newwin(LINES - composer_h, COLS - side_w, 0, side_w),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0),
         *old_side_win = panel_window(panels[ID_SIDE]),
         *old_main_win = panel_window(panels[ID_MAIN]),
         *old_composer_win = panel_window(panels[ID_COMP]);

  replace_panel(panels[ID_SIDE], side_win);
  replace_panel(panels[ID_MAIN], main_win);
  replace_panel(panels[ID_COMP], composer_win);

  delwin(old_side_win);
  delwin(old_main_win);
  delwin(old_composer_win);

  draw_border();
  fill(app_state);
  update_panels();
}

void draw_cur() {
  if (current_pan == ID_COMP) {
    auto &com_cur = cursor_positions[ID_COMP];
    wmove(panel_window(panels[ID_COMP]), com_cur.first, com_cur.second);
  } else {
    curs_set(0);
  }
}

void fill(tl_app_state_struct &app_state) {
  fill(app_state, ID_MAIN);
  fill(app_state, ID_SIDE);
  fill(app_state, ID_COMP);
}

void fill(tl_app_state_struct &app_state, int p) {
  if (p == ID_MAIN) {
    fill(panels[ID_MAIN], 'b', 0, 0, 0, 0);
  } else if (p == ID_SIDE) {
    draw_side(app_state);
  } else if (p == ID_COMP) {
    fill(panels[ID_COMP], 'c', 0, 0, 1, 0);
  }
}

void fill(PANEL *pan, char c, int offsetx, int cutoffx, int offsety,
          int cutoffy) {
  int maxx, maxy;
  getmaxyx(panel_window(pan), maxy, maxx);
  for (int y = offsety; y < maxy - cutoffy; ++y) {
    mvwaddstr(panel_window(pan), y, offsetx,
              std::string(maxx - offsetx - cutoffx, c).c_str());
  }
}

void draw_side(tl_app_state_struct &app_state) {
  std::lock_guard l(*app_state.mutexes[tl_app_state_struct::_id_chatboxes]);
  auto &position_sets = app_state.position_sets[td::td_api::chatListMain::ID];
  logger->debug("position set size {}", position_sets.size());
  size_t list_size =
      std::min(position_sets.size(), static_cast<size_t>(LINES - composer_h));

  auto pos = position_sets.begin();
  auto &id_to_chat = app_state.id_to_chat;

  for (size_t i = 0; i < list_size; ++i) {
    auto &chat = id_to_chat[pos->second];
    if (i == 0) {
      app_state.chosen_chat_id = chat->id_;
    }
    std::string name(chat->title_);
    if (name.size() < static_cast<size_t>(side_w - 1)) {
      name.insert(name.end(), side_w - 1 - name.size(), ' ');
    }

    if (app_state.chosen_chat_id == chat->id_) {
      logger->debug("reversing {}", name);
      wattron(panel_window(panels[ID_SIDE]), A_REVERSE);
    }
    mvwaddstr(panel_window(panels[ID_SIDE]), i, 0, name.substr(0, side_w - 1).c_str());
    if (app_state.chosen_chat_id == chat->id_) {
      wattroff(panel_window(panels[ID_SIDE]), A_REVERSE);
    }
    ++pos;
  }
}

void draw_main(tl_app_state_struct &app_state);
void draw_composer(tl_app_state_struct &app_state);

void draw_cursor() {
  WINDOW *win;
  switch (current_pan) {
  case ID_SIDE:
    win = panel_window(panels[ID_SIDE]);
    break;
  case ID_MAIN:
    win = panel_window(panels[ID_MAIN]);
    break;
  case ID_COMP:
    win = panel_window(panels[ID_COMP]);
    break;
  }

  auto &curpos = cursor_positions[current_pan];
  logger->debug("draw_cursor current_pan({}) curpos({}, {})", current_pan,
                curpos.first, curpos.second);
  wmove(win, curpos.first, curpos.second);
  wnoutrefresh(win);
};

void init_config() {
  side_w = std::min(32, COLS / 4);
  composer_h = std::min(6, LINES / 5);
}
