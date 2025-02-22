#ifndef INCLUDE_SRC_LAYOUT_H_
#define INCLUDE_SRC_LAYOUT_H_

#include <panel.h>

void fill();
void fill(PANEL *pan, char c, int offsetx = 0, int cutoffx = 0, int offsety = 0,
          int cutoffy = 0);
void init_config();
void init_layout();
void draw_border();
void resize(int new_side_w, int new_composer_h);
void draw_cur();
#endif  // INCLUDE_SRC_LAYOUT_H_
