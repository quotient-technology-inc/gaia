// Copyright 2013, Beeri 15.  All rights reserved.
// Author: Roman Gershman (romange@gmail.com)
//
#ifndef _STRINGS_HASH_H
#define _STRINGS_HASH_H

#include <functional>
#include "base/hash.h"
#include "strings/stringpiece.h"

namespace std {

#if __cplusplus <  201703L
template<> struct hash<StringPiece> {
  size_t operator()(StringPiece slice) const {
    return base::MurmurHash3_x86_32(
          reinterpret_cast<const uint8_t*>(slice.data()),
          slice.size() * sizeof(typename StringPiece::value_type), 16785407UL);
  }
};
#endif

template<typename Iter> struct hash<strings::Range<Iter>> {
  size_t operator()(strings::Range<Iter> slice) const {
    return base::MurmurHash3_x86_32(
          reinterpret_cast<const uint8_t*>(slice.data()),
          slice.size() * sizeof(typename strings::Range<Iter>::value_type), 16785407UL);
  }
};

}  // namespace std


#endif  // _STRINGS_HASH_H

