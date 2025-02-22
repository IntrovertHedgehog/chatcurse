#include "utils.h"

#include <time.h>

#include <string>

#include "global.h"

void debug_log(std::string const &msg) {
  time_t t = time(NULL);
  log_os << std::asctime(std::localtime(&t)) << msg << std::endl;
}
