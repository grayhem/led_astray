#pragma once


// fucking, are you serious.

template <class A, class B>
uint16_t force_min (A left, B right) {
  return min (uint16_t (left), uint16_t (right));
}
