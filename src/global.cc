#include "global.h"

#include <fstream>

PANEL *composer_pan, *side_pan, *main_pan, *float_pan;
int side_w, composer_h;
std::ofstream log_os;
int current_pan, comcurx, comcury;
