#include <iostream>

#include "qrcodegen.hpp"

using qrcodegen::display;
using qrcodegen::QrCode;

int main() {
  QrCode qr = QrCode::encodeText("unbelievable", QrCode::Ecc::QUARTILE);
  display(std::cout, qr);
}
