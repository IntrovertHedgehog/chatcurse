#include "global.h"

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>

std::shared_ptr<spdlog::logger> logger;
PANEL *composer_pan, *side_pan, *main_pan, *float_pan;
int side_w, composer_h;
int current_pan, comcurx, comcury;
bool use_test_dc = false, logout_next = false;
event_queue_struct event_queue;
