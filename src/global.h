#ifndef INCLUDE_SRC_GLOBAL_H_
#define INCLUDE_SRC_GLOBAL_H_

#define ID_SIDE 0
#define ID_MAIN 1
#define ID_COMP 2
#define ID_FLOAT 3

#define ID_E_SIDE_MAIN 1
#define ID_E_COMP_TOP 2
#define ID_C_SIDE_MAIN_COMP 3

#include <fstream>
#include <curses.h>
#include <panel.h>

extern PANEL * composer_pan, *side_pan, *main_pan, *float_pan;
extern int side_w, composer_h;
extern std::ofstream log_os;
extern int current_pan, comcurx, comcury;

extern bool use_test_dc, logout_on_init;

#endif  // INCLUDE_SRC_GLOBAL_H_
