#include "layout.h"

#include <curses.h>
#include <openssl/conf.h>
#include <panel.h>

#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <mutex>
#include <string>
#include <utility>

#include "global.h"
#include "td/telegram/td_api.h"
#include "utils.h"

void init_layout(tl_app_state_struct &app_state) {
  WINDOW *side_win = newwin(LINES - composer_h - 1, side_w, 0, 0),
         *main_win =
             newwin(LINES - composer_h - 1, COLS - side_w - 1, 0, side_w + 1),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0),
         *side_main_border_win = newwin(LINES - composer_h - 1, 1, 0, side_w),
         *composer_top_border_win = newwin(1, COLS, LINES - composer_h - 1, 0);

  panels[ID_SIDE] = new_panel(side_win);
  panels[ID_MAIN] = new_panel(main_win);
  panels[ID_COMP] = new_panel(composer_win);
  panels[ID_E_SIDE_MAIN] = new_panel(side_main_border_win);
  panels[ID_C_SIDE_MAIN_COMP] = new_panel(composer_top_border_win);

  current_pan = ID_SIDE;

  draw_border();
  fill(app_state);
  update_panels();
  doupdate();
}

// always draw borders before anything else
void draw_border() {
  mvwvline(panel_window(panels[ID_E_SIDE_MAIN]), 0, 0, ACS_VLINE,
           LINES - composer_h - 1);
  mvwhline(panel_window(panels[ID_C_SIDE_MAIN_COMP]), 0, 0, ACS_HLINE, COLS);
  mvwaddch(panel_window(panels[ID_C_SIDE_MAIN_COMP]), 0, side_w, ACS_SSBS);
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

  WINDOW *side_win = newwin(LINES - composer_h - 1, side_w, 0, 0),
         *main_win =
             newwin(LINES - composer_h - 1, COLS - side_w - 1, 0, side_w + 1),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0),
         *side_main_border_win = newwin(LINES - composer_h - 1, 1, 0, side_w),
         *composer_top_border_win = newwin(1, COLS, LINES - composer_h - 1, 0),
         *old_side_win = panel_window(panels[ID_SIDE]),
         *old_main_win = panel_window(panels[ID_MAIN]),
         *old_composer_win = panel_window(panels[ID_COMP]),
         *old_side_main_border_win = panel_window(panels[ID_E_SIDE_MAIN]),
         *old_composer_top_border_win =
             panel_window(panels[ID_C_SIDE_MAIN_COMP]);

  replace_panel(panels[ID_SIDE], side_win);
  replace_panel(panels[ID_MAIN], main_win);
  replace_panel(panels[ID_COMP], composer_win);
  replace_panel(panels[ID_E_SIDE_MAIN], side_main_border_win);
  replace_panel(panels[ID_C_SIDE_MAIN_COMP], composer_top_border_win);

  delwin(old_side_win);
  delwin(old_main_win);
  delwin(old_composer_win);
  delwin(old_side_main_border_win);
  delwin(old_composer_top_border_win);

  draw_border();
  fill(app_state);
  update_panels();
}

void fill(tl_app_state_struct &app_state) {
  draw_main(app_state);
  draw_side(app_state);
  draw_composer(app_state);
}

void draw_pane(tl_app_state_struct &app_state, int pane) {
  switch (pane) {
  case ID_SIDE:
    draw_side(app_state);
    break;
  case ID_COMP:
    draw_composer(app_state);
    break;
  case ID_MAIN:
    draw_main(app_state);
    break;
  }
}

void draw_side(tl_app_state_struct &app_state) {
  std::lock_guard l(*app_state.mutexes[tl_app_state_struct::_id_chatboxes]);
  auto &position_sets = app_state.position_sets[app_state.current_chatlist];
  int maxy, maxx;
  getmaxyx(panel_window(panels[ID_SIDE]), maxy, maxx);
  size_t list_size = std::min(position_sets.size(), static_cast<size_t>(maxy));
  auto pos = position_sets.begin();
  advance(pos, app_state.chatlist_scroll_offset);
  auto &id_to_chat = app_state.id_to_chat;

  for (size_t i = 0; i < list_size; ++i) {
    if (pos != position_sets.end()) {
      auto &chat = id_to_chat[pos->second];
      std::string name(chat->title_);
      if (name.size() < static_cast<size_t>(maxx)) {
        name.insert(name.end(), maxx - name.size(), ' ');
      } else if (name.size() == 0) {
        name = "deleted account";
      }

      if (app_state.chosen_chat_id == chat->id_) {
        wattron(panel_window(panels[ID_SIDE]), A_REVERSE);
      }
      mvwaddstr(panel_window(panels[ID_SIDE]), i, 0,
                name.substr(0, maxx).c_str());
      if (app_state.chosen_chat_id == chat->id_) {
        wattroff(panel_window(panels[ID_SIDE]), A_REVERSE);
      }
      ++pos;
    } else {
      mvwaddstr(panel_window(panels[ID_SIDE]), i, 0,
                std::string(maxx, ' ').c_str());
    }
  }
}

void draw_main(tl_app_state_struct &app_state) {};
void draw_composer(tl_app_state_struct &app_state) {};

void draw_cursor() {
  WINDOW *win = panel_window(panels[current_pan]);

  auto &curpos = cursor_positions[current_pan];
  logger->debug("draw_cursor current_pan({}) curpos({}, {})", current_pan,
                curpos.first, curpos.second);
  wmove(win, curpos.first, curpos.second);
  wnoutrefresh(win);
}

void init_config() {
  side_w = std::min(32, COLS / 4);
  composer_h = std::min(3, LINES / 5);
}
