//
// Created by zachvem on 16-Sep-26.
//

#include "frame.h"
frame::frame(uint32_t length)
  : length_(length >= 0 ? length : 0), payload_{new std::byte[length_]} {
}

frame::~frame() {
  *payload_ = nullptr;
  delete* payload_;
}