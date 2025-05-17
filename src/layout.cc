#include "layout.h"

#include <curses.h>
#include <mutex>
#include <panel.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "global.h"
#include "td/telegram/td_api.h"
#include "utils.h"

void init_layout(tl_app_state_struct &app_state) {
  WINDOW *side_win = newwin(LINES - composer_h, side_w, 0, 0),
         *main_win = newwin(LINES - composer_h, COLS - side_w, 0, side_w),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0);

  side_pan = new_panel(side_win);
  main_pan = new_panel(main_win);
  composer_pan = new_panel(composer_win);
  comcurx = 0, comcury = 1;
  current_pan = ID_COMP;

  draw_border();
  fill(app_state);
  update_panels();
  doupdate();
}

// always draw borders before anything else
void draw_border() {
  wborder(panel_window(side_pan), ' ', 0, ' ', ' ', ' ', ACS_VLINE, ' ',
          ACS_VLINE);
  wborder(panel_window(composer_pan), ' ', ' ', ACS_HLINE, ' ', ACS_HLINE,
          ACS_HLINE, ' ', ' ');
  mvwaddch(panel_window(composer_pan), 0, side_w - 1, ACS_SSBS);
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
         *old_side_win = panel_window(side_pan),
         *old_main_win = panel_window(main_pan),
         *old_composer_win = panel_window(composer_pan);

  replace_panel(side_pan, side_win);
  replace_panel(main_pan, main_win);
  replace_panel(composer_pan, composer_win);

  delwin(old_side_win);
  delwin(old_main_win);
  delwin(old_composer_win);

  draw_border();
  fill(app_state);
  update_panels();
}

void draw_cur() {
  if (current_pan == ID_COMP) {
    wmove(panel_window(composer_pan), comcury, comcurx);
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
    fill(main_pan, 'b', 0, 0, 0, 0);
  } else if (p == ID_SIDE) {
    draw_side(app_state);
  } else if (p == ID_COMP) {
    fill(composer_pan, 'c', 0, 0, 1, 0);
  }
}

void fill(PANEL *pan, char c, int offsetx, int cutoffx, int offsety,
          int cutoffy) {
  int maxx, maxy;
  getmaxyx(panel_window(pan), maxy, maxx);
  // logger->info(std::format("fill {}, {}, {}, {}, {}, {}, {}", c, offsetx,
  // cutoffx,
  //                       offsety, cutoffy, maxy, maxx));
  for (int y = offsety; y < maxy - cutoffy; ++y) {
    // logger->info(std::format("pos {}, {} size {}", y, offsetx,
    //                       maxx - offsetx - cutoffx));
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
  std::vector<std::string> chatlist_name(list_size);

  auto pos = position_sets.begin();
  for (size_t i = 0; i < list_size; ++i) {
    auto &id_to_chat = app_state.id_to_chat;
    logger->debug("chat item {}({}, {})", id_to_chat[pos->second]->title_,
                  pos->first, pos->second);
    chatlist_name[i] = id_to_chat[pos->second]->title_;
    ++pos;
  }

  logger->debug(
      "printing chatlist {}",
      _from_container<std::vector<std::string>>(
          chatlist_name.begin(), chatlist_name.end(), chatlist_name.size()));

  for (size_t i = 0; i < list_size; ++i) {
    mvwaddstr(panel_window(side_pan), i, 0,
              chatlist_name[i].substr(0, side_w - 1).c_str());
  }
}

void draw_main(tl_app_state_struct &app_state);
void draw_composer(tl_app_state_struct &app_state);

void init_config() {
  side_w = std::min(32, COLS / 4);
  composer_h = std::min(6, LINES / 5);
}
