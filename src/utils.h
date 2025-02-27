#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

#ifndef INCLUDE_SRC_UTILS_H_
#define INCLUDE_SRC_UTILS_H_
#include <string>
void debug_log(std::string const &);
// https://www.thonky.com/qr-code-tutorial/introduction
// version 9-Q byte latin-1 mode
std::string qrcode_from_string(std::string const &);
#endif  // INCLUDE_SRC_UTILS_H_
