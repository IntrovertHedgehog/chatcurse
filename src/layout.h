#ifndef INCLUDE_SRC_LAYOUT_H_
#define INCLUDE_SRC_LAYOUT_H_

#include "global.h"
#define ID_SIDE 1
#define ID_MAIN 2
#define ID_COMP 4
#define ID_FLOAT 8
#define ID_ALL (ID_SIDE | ID_MAIN | ID_COMP | ID_FLOAT)
#define ID_QUIT 16

#define ID_E_SIDE_MAIN 1
#define ID_E_COMP_TOP 2
#define ID_C_SIDE_MAIN_COMP 3

#include <panel.h>

void fill(tl_app_state_struct &);
void fill(tl_app_state_struct &, int);
void fill(PANEL *pan, char c, int offsetx = 0, int cutoffx = 0, int offsety = 0,
          int cutoffy = 0);
void draw_side(tl_app_state_struct &);
void draw_main(tl_app_state_struct &);
void draw_composer(tl_app_state_struct &);
void draw_cursor();
void init_config();
void init_layout(tl_app_state_struct &);
void draw_border();
void resize(tl_app_state_struct &, int new_side_w, int new_composer_h);
void draw_cur();
#endif  // INCLUDE_SRC_LAYOUT_H_
