// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include <stddef.h>  // for nullptr

#include "JsonBufferAllocated.hpp"

namespace Variants {
namespace Internals {

// A node for a singly-linked list.
// Used by List<T> and its iterators.
template <typename T>
struct ListNode : public Internals::JsonBufferAllocated {
  ListNode() throw() : next(nullptr) {}

  ListNode<T> *next;
  T content;
};
}
}
