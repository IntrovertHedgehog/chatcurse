#include "layout.h"

#include <string>

#include "global.h"
#include "utils.h"

void init_layout() {
  WINDOW *side_win = newwin(LINES - composer_h, side_w, 0, 0),
         *main_win = newwin(LINES - composer_h, COLS - side_w, 0, side_w),
         *composer_win = newwin(composer_h, COLS, LINES - composer_h, 0);

  side_pan = new_panel(side_win);
  main_pan = new_panel(main_win);
  composer_pan = new_panel(composer_win);
  comcurx = 0, comcury = 1;
  current_pan = ID_COMP;

  draw_border();
  fill();
  draw_cur();
  update_panels();
  doupdate();
}

void draw_border() {
  wborder(panel_window(side_pan), ' ', 0, ' ', ' ', ' ', ACS_VLINE, ' ',
          ACS_VLINE);
  wborder(panel_window(composer_pan), ' ', ' ', ACS_HLINE, ' ', ACS_HLINE,
          ACS_HLINE, ' ', ' ');
  mvwaddch(panel_window(composer_pan), 0, side_w - 1, ACS_SSBS);
}

void resize(int new_side_w, int new_composer_h) {
  debug_log(std::format("new size ({}, {})", new_side_w, new_composer_h));

  if (new_side_w >= COLS) return;
  if (new_composer_h >= LINES) return;

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
  fill();
  draw_cur();
  update_panels();
  doupdate();
}

void draw_cur() {
  if (current_pan == ID_COMP) {
    wmove(panel_window(composer_pan), comcury, comcurx);
  } else {
    curs_set(0);
  }
}

void fill() {
  fill(side_pan, 'a', 0, 1, 0, 0);
  fill(main_pan, 'b', 0, 0, 0, 0);
  fill(composer_pan, 'c', 0, 0, 1, 0);
}

void fill(PANEL *pan, char c, int offsetx, int cutoffx, int offsety,
          int cutoffy) {
  int maxx, maxy;
  getmaxyx(panel_window(pan), maxy, maxx);
  for (int y = offsety; y < maxy - cutoffy; ++y)
    mvwaddstr(panel_window(pan), y, offsetx,
              std::string(maxx - offsetx - cutoffx, c).c_str());
}
