#include "stream.h"

Stream::Stream(const char *str)
  : str_(str)
  , pos_(0) {
}

Stream::~Stream() {
}

int Stream::Read() {
  int c = str_[pos_];
  ++pos_;

  return c;
}

int Stream::Next() {
  return str_[pos_];
}

void Stream::Back() {
  --pos_;
}
