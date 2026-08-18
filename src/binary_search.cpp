//
// Created by zachvem on 18-Aug-26.
//
#include "binary_search.h"

std::optional<int> binary_search(std::span<int> &list, int target) {
  int left = 0;
  int right = static_cast<int>(list.size()) - 1;
  //invariant:
  //if the target exists, it's index is inside [left, right]
  while (left <= right) {
    int mid = left + (right - left) / 2;
    if (list[mid] == target) {
      return mid;
    }
    if (list[mid] < target) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return {};
}