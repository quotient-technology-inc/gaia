// Copyright 2010 Google Inc. All Rights Reserved.
// Refactored from contributions of various authors in strings/strutil.cc
//
// This file contains string processing functions related to
// numeric values.
#define __STDC_FORMAT_MACROS 1
#include "strings/numbers.h"

#include <memory>
#include <cassert>
#include <ctype.h>
#include <errno.h>
#include <float.h>          // for DBL_DIG and FLT_DIG
#include <math.h>           // for HUGE_VAL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <string>

#include "base/int128.h"
#include "base/integral_types.h"

#include "base/logging.h"
#include "strings/strtoint.h"
#include "strings/stringprintf.h"

#include "absl/strings/ascii.h"

using absl::ascii_isspace;
using absl::ascii_toupper;
using std::numeric_limits;
using std::string;

// ----------------------------------------------------------------------
// ParseLeadingInt32Value()
// ParseLeadingUInt32Value()
//    A simple parser for [u]int32 values. Returns the parsed value
//    if a valid value is found; else returns deflt
//    This cannot handle decimal numbers with leading 0s.
// --------------------------------------------------------------------

int32 ParseLeadingInt32Value(StringPiece str, int32 deflt) {
  if (str.empty()) return deflt;
  using std::numeric_limits;

  char *error = NULL;
  long value = strtol(str.data(), &error, 0);
  // Limit long values to int32 min/max.  Needed for lp64; no-op on 32 bits.
  if (value > numeric_limits<int32>::max()) {
    value = numeric_limits<int32>::max();
  } else if (value < numeric_limits<int32>::min()) {
    value = numeric_limits<int32>::min();
  }
  return (error == str.data()) ? deflt : value;
}

uint32 ParseLeadingUInt32Value(StringPiece str, uint32 deflt) {
  using std::numeric_limits;
  if (str.empty()) return deflt;
  if (numeric_limits<unsigned long>::max() == numeric_limits<uint32>::max()) {
    // When long is 32 bits, we can use strtoul.
    char *error = NULL;
    const uint32 value = strtoul(str.data(), &error, 0);
    return (error == str.data()) ? deflt : value;
  } else {
    // When long is 64 bits, we must use strto64 and handle limits
    // by hand.  The reason we cannot use a 64-bit strtoul is that
    // it would be impossible to differentiate "-2" (that should wrap
    // around to the value UINT_MAX-1) from a string with ULONG_MAX-1
    // (that should be pegged to UINT_MAX due to overflow).
    char *error = NULL;
    int64 value = strto64(str.data(), &error, 0);
    if (value > numeric_limits<uint32>::max() ||
        value < -static_cast<int64>(numeric_limits<uint32>::max())) {
      value = numeric_limits<uint32>::max();
    }
    // Within these limits, truncation to 32 bits handles negatives correctly.
    return (error == str.data()) ? deflt : value;
  }
}

// ----------------------------------------------------------------------
// ParseLeadingDec32Value
// ParseLeadingUDec32Value
//    A simple parser for [u]int32 values. Returns the parsed value
//    if a valid value is found; else returns deflt
//    The string passed in is treated as *10 based*.
//    This can handle strings with leading 0s.
// --------------------------------------------------------------------

int32 ParseLeadingDec32Value(StringPiece str, int32 deflt) {
  using std::numeric_limits;

  char *error = NULL;
  long value = strtol(str.data(), &error, 10);
  // Limit long values to int32 min/max.  Needed for lp64; no-op on 32 bits.
  if (value > numeric_limits<int32>::max()) {
    value = numeric_limits<int32>::max();
  } else if (value < numeric_limits<int32>::min()) {
    value = numeric_limits<int32>::min();
  }
  return (error == str.data()) ? deflt : value;
}

uint32 ParseLeadingUDec32Value(StringPiece str, uint32 deflt) {
  using std::numeric_limits;

  if (numeric_limits<unsigned long>::max() == numeric_limits<uint32>::max()) {
    // When long is 32 bits, we can use strtoul.
    char *error = NULL;
    const uint32 value = strtoul(str.data(), &error, 10);
    return (error == str.begin()) ? deflt : value;
  } else {
    // When long is 64 bits, we must use strto64 and handle limits
    // by hand.  The reason we cannot use a 64-bit strtoul is that
    // it would be impossible to differentiate "-2" (that should wrap
    // around to the value UINT_MAX-1) from a string with ULONG_MAX-1
    // (that should be pegged to UINT_MAX due to overflow).
    char *error = NULL;
    int64 value = strto64(str.data(), &error, 10);
    if (value > numeric_limits<uint32>::max() ||
        value < -static_cast<int64>(numeric_limits<uint32>::max())) {
      value = numeric_limits<uint32>::max();
    }
    // Within these limits, truncation to 32 bits handles negatives correctly.
    return (error == str.data()) ? deflt : value;
  }
}

// ----------------------------------------------------------------------
// ParseLeadingUInt64Value
// ParseLeadingInt64Value
// ParseLeadingHex64Value
//    A simple parser for 64-bit values. Returns the parsed value if a
//    valid integer is found; else returns deflt
//    UInt64 and Int64 cannot handle decimal numbers with leading 0s.
// --------------------------------------------------------------------
uint64 ParseLeadingUInt64Value(StringPiece str, uint64 deflt) {
  char *error = NULL;
  const uint64 value = strtou64(str.data(), &error, 0);
  return (error == str.data()) ? deflt : value;
}

int64 ParseLeadingInt64Value(StringPiece str, int64 deflt) {
  char *error = NULL;
  const int64 value = strto64(str.data(), &error, 0);
  return (error == str.data()) ? deflt : value;
}

uint64 ParseLeadingHex64Value(StringPiece str, uint64 deflt) {
  char *error = NULL;
  const uint64 value = strtou64(str.data(), &error, 16);
  return (error == str.data()) ? deflt : value;
}

// ----------------------------------------------------------------------
// ParseLeadingDec64Value
// ParseLeadingUDec64Value
//    A simple parser for [u]int64 values. Returns the parsed value
//    if a valid value is found; else returns deflt
//    The string passed in is treated as *10 based*.
//    This can handle strings with leading 0s.
// --------------------------------------------------------------------

int64 ParseLeadingDec64Value(StringPiece str, int64 deflt) {
  char *error = NULL;
  const int64 value = strto64(str.data(), &error, 10);
  return (error == str.data()) ? deflt : value;
}

uint64 ParseLeadingUDec64Value(StringPiece str, uint64 deflt) {
  char *error = NULL;
  const uint64 value = strtou64(str.data(), &error, 10);
  return (error == str.data()) ? deflt : value;
}

// ----------------------------------------------------------------------
// ParseLeadingDoubleValue()
//    A simple parser for double values. Returns the parsed value
//    if a valid value is found; else returns deflt
// --------------------------------------------------------------------

double ParseLeadingDoubleValue(const char *str, double deflt) {
  char *error = NULL;
  errno = 0;
  const double value = strtod(str, &error);
  if (errno != 0 ||  // overflow/underflow happened
      error == str) {  // no valid parse
    return deflt;
  } else {
    return value;
  }
}

namespace {

// Represents integer values of digits.
// Uses 36 to indicate an invalid character since we support
// bases up to 36.
static const int8 kAsciiToInt[256] = {
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,  // 16 36s.
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
  36, 36, 36, 36, 36, 36, 36,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
  36, 36, 36, 36, 36, 36,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
  36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
  36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36 };

// Parse the sign and optional hex or oct prefix in range
// [*start_ptr, *end_ptr).
inline bool safe_parse_sign_and_base(const char** start_ptr  /*inout*/,
                                     const char** end_ptr  /*inout*/,
                                     int* base_ptr  /*inout*/,
                                     bool* negative_ptr  /*output*/) {
  const char* start = *start_ptr;
  const char* end = *end_ptr;
  int base = *base_ptr;

  // Consume whitespace.
  while (start < end && ascii_isspace(start[0])) {
    ++start;
  }
  while (start < end && ascii_isspace(end[-1])) {
    --end;
  }
  if (start >= end) {
    return false;
  }

  // Consume sign.
  *negative_ptr = (start[0] == '-');
  if (*negative_ptr || start[0] == '+') {
    ++start;
    if (start >= end) {
      return false;
    }
  }

  // Consume base-dependent prefix.
  //  base 0: "0x" -> base 16, "0" -> base 8, default -> base 10
  //  base 16: "0x" -> base 16
  // Also validate the base.
  if (base == 0) {
    if (end - start >= 2 && start[0] == '0' &&
        (start[1] == 'x' || start[1] == 'X')) {
      base = 16;
      start += 2;
    } else if (end - start >= 1 && start[0] == '0') {
      base = 8;
      start += 1;
    } else {
      base = 10;
    }
  } else if (base == 16) {
    if (end - start >= 2 && start[0] == '0' &&
        (start[1] == 'x' || start[1] == 'X')) {
      start += 2;
    }
  } else if (base >= 2 && base <= 36) {
    // okay
  } else {
    return false;
  }
  *start_ptr = start;
  *end_ptr = end;
  *base_ptr = base;
  return true;
}

// Consume digits.
//
// The classic loop:
//
//   for each digit
//     value = value * base + digit
//   value *= sign
//
// The classic loop needs overflow checking.  It also fails on the most
// negative integer, -2147483648 in 32-bit two's complement representation.
//
// My improved loop:
//
//  if (!negative)
//    for each digit
//      value = value * base
//      value = value + digit
//  else
//    for each digit
//      value = value * base
//      value = value - digit
//
// Overflow checking becomes simple.

template<typename IntType>
inline bool safe_parse_positive_int(
    const char* start, const char* end, int base, IntType* value_p) {
  IntType value = 0;
  const IntType vmax = std::numeric_limits<IntType>::max();
  assert(vmax > 0);
  // assert(vmax >= base);
  const IntType vmax_over_base = vmax / base;
  // loop over digits
  // loop body is interleaved for perf, not readability
  for (; start < end; ++start) {
    unsigned char c = static_cast<unsigned char>(start[0]);
    int digit = kAsciiToInt[c];
    if (value > vmax_over_base) return false;
    value *= base;
    if (digit >= base) return false;
    if (value > vmax - digit) return false;
    value += digit;
  }
  *value_p = value;
  return true;
}

template<typename IntType>
inline bool safe_parse_negative_int(
    const char* start, const char* end, int base, IntType* value_p) {
  IntType value = 0;
  const IntType vmin = std::numeric_limits<IntType>::min();
  assert(vmin < 0);
  IntType vmin_over_base = vmin / base;
  // 2003 c++ standard [expr.mul]
  // "... the sign of the remainder is implementation-defined."
  // Although (vmin/base)*base + vmin%base is always vmin.
  // 2011 c++ standard tightens the spec but we cannot rely on it.
  if (vmin % base > 0) {
    vmin_over_base += 1;
  }
  // loop over digits
  // loop body is interleaved for perf, not readability
  for (; start < end; ++start) {
    unsigned char c = static_cast<unsigned char>(start[0]);
    int digit = kAsciiToInt[c];
    if (value < vmin_over_base) return false;
    value *= base;
    if (digit >= base) return false;
    if (value < vmin + digit) return false;
    value -= digit;
  }
  *value_p = value;
  return true;
}

// Input format based on POSIX.1-2008 strtol
// http://pubs.opengroup.org/onlinepubs/9699919799/functions/strtol.html
template<typename IntType>
bool safe_int_internal(const char* start, const char* end, int base,
                       IntType* value_p) {
  bool negative;
  if (!safe_parse_sign_and_base(&start, &end, &base, &negative)) {
    return false;
  }
  if (!negative) {
    return safe_parse_positive_int(start, end, base, value_p);
  } else {
    return safe_parse_negative_int(start, end, base, value_p);
  }
}

template<typename IntType>
inline bool safe_uint_internal(const char* start, const char* end, int base,
                               IntType* value_p) {
  bool negative;
  if (!safe_parse_sign_and_base(&start, &end, &base, &negative) || negative) {
    return false;
  }
  return safe_parse_positive_int(start, end, base, value_p);
}

}  // anonymous namespace

bool safe_strto32_base(StringPiece str,
                       int32* v, int base) {
  return safe_int_internal<int32>(str.begin(), str.end(), base, v);
}

bool safe_strto64_base(StringPiece str,
                       int64* v, int base) {
  return safe_int_internal<int64>(str.begin(), str.end(), base, v);
}


bool safe_strtof(StringPiece str, float* value) {
  char* endptr;
  *value = strtof(str.data(), &endptr);
  if (endptr != str.data()) {
    while (endptr < str.end() && ascii_isspace(*endptr)) ++endptr;
  }
  // Ignore range errors from strtod/strtof.
  // The values it returns on underflow and
  // overflow are the right fallback in a
  // robust setting.
  return !str.empty() && endptr == str.end();
}

bool safe_strtod(StringPiece str, double* value) {
  char* endptr;
  *value = strtod(str.data(), &endptr);
  if (endptr != str.data()) {
    while (endptr < str.end() && ascii_isspace(*endptr)) ++endptr;
  }
  // Ignore range errors from strtod.  The values it
  // returns on underflow and overflow are the right
  // fallback in a robust setting.
  return !str.empty() && endptr == str.end();
}

uint64 atoi_kmgt(const char* s) {
  char* endptr;
  uint64 n = strtou64(s, &endptr, 10);
  uint64 scale = 1;
  char c = *endptr;
  if (c != '\0') {
    c = ascii_toupper(c);
    switch (c) {
      case 'K':
        scale = GG_ULONGLONG(1) << 10;
        break;
      case 'M':
        scale = GG_ULONGLONG(1) << 20;
        break;
      case 'G':
        scale = GG_ULONGLONG(1) << 30;
        break;
      case 'T':
        scale = GG_ULONGLONG(1) << 40;
        break;
      default:
        LOG(FATAL) << "Invalid mnemonic: `" << c << "';"
                   << " should be one of `K', `M', `G', and `T'.";
    }
  }
  return n * scale;
}



const char two_ASCII_digits[100][2] = {
  {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'},
  {'0', '5'}, {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'},
  {'1', '0'}, {'1', '1'}, {'1', '2'}, {'1', '3'}, {'1', '4'},
  {'1', '5'}, {'1', '6'}, {'1', '7'}, {'1', '8'}, {'1', '9'},
  {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'}, {'2', '4'},
  {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'},
  {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'},
  {'3', '5'}, {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'},
  {'4', '0'}, {'4', '1'}, {'4', '2'}, {'4', '3'}, {'4', '4'},
  {'4', '5'}, {'4', '6'}, {'4', '7'}, {'4', '8'}, {'4', '9'},
  {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'}, {'5', '4'},
  {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'},
  {'6', '0'}, {'6', '1'}, {'6', '2'}, {'6', '3'}, {'6', '4'},
  {'6', '5'}, {'6', '6'}, {'6', '7'}, {'6', '8'}, {'6', '9'},
  {'7', '0'}, {'7', '1'}, {'7', '2'}, {'7', '3'}, {'7', '4'},
  {'7', '5'}, {'7', '6'}, {'7', '7'}, {'7', '8'}, {'7', '9'},
  {'8', '0'}, {'8', '1'}, {'8', '2'}, {'8', '3'}, {'8', '4'},
  {'8', '5'}, {'8', '6'}, {'8', '7'}, {'8', '8'}, {'8', '9'},
  {'9', '0'}, {'9', '1'}, {'9', '2'}, {'9', '3'}, {'9', '4'},
  {'9', '5'}, {'9', '6'}, {'9', '7'}, {'9', '8'}, {'9', '9'}
};


// ----------------------------------------------------------------------
// FastInt32ToBufferLeft()
// FastUInt32ToBufferLeft()
// FastInt64ToBufferLeft()
// FastUInt64ToBufferLeft()
//
// Like the Fast*ToBuffer() functions above, these are intended for speed.
// Unlike the Fast*ToBuffer() functions, however, these functions write
// their output to the beginning of the buffer (hence the name, as the
// output is left-aligned).  The caller is responsible for ensuring that
// the buffer has enough space to hold the output.
//
// Returns a pointer to the end of the string (i.e. the null character
// terminating the string).
// ----------------------------------------------------------------------

char* FastUInt32ToBufferLeft(uint32 u, char* buffer) {
  int digits;
  const char *ASCII_digits = NULL;
  // The idea of this implementation is to trim the number of divides to as few
  // as possible by using multiplication and subtraction rather than mod (%),
  // and by outputting two digits at a time rather than one.
  // The huge-number case is first, in the hopes that the compiler will output
  // that case in one branch-free block of code, and only output conditional
  // branches into it from below.
  if (u >= 1000000000) {  // >= 1,000,000,000
    digits = u / 100000000;  // 100,000,000
    ASCII_digits = two_ASCII_digits[digits];
    buffer[0] = ASCII_digits[0];
    buffer[1] = ASCII_digits[1];
    buffer += 2;
 sublt100_000_000:
    u -= digits * 100000000;  // 100,000,000
 lt100_000_000:
    digits = u / 1000000;  // 1,000,000
    ASCII_digits = two_ASCII_digits[digits];
    buffer[0] = ASCII_digits[0];
    buffer[1] = ASCII_digits[1];
    buffer += 2;
 sublt1_000_000:
    u -= digits * 1000000;  // 1,000,000
 lt1_000_000:
    digits = u / 10000;  // 10,000
    ASCII_digits = two_ASCII_digits[digits];
    buffer[0] = ASCII_digits[0];
    buffer[1] = ASCII_digits[1];
    buffer += 2;
 sublt10_000:
    u -= digits * 10000;  // 10,000
 lt10_000:
    digits = u / 100;
    ASCII_digits = two_ASCII_digits[digits];
    buffer[0] = ASCII_digits[0];
    buffer[1] = ASCII_digits[1];
    buffer += 2;
 sublt100:
    u -= digits * 100;
 lt100:
    digits = u;
    ASCII_digits = two_ASCII_digits[digits];
    buffer[0] = ASCII_digits[0];
    buffer[1] = ASCII_digits[1];
    buffer += 2;
 done:
    *buffer = 0;
    return buffer;
  }

  if (u < 100) {
    digits = u;
    if (u >= 10) goto lt100;
    *buffer++ = '0' + digits;
    goto done;
  }
  if (u  <  10000) {   // 10,000
    if (u >= 1000) goto lt10_000;
    digits = u / 100;
    *buffer++ = '0' + digits;
    goto sublt100;
  }
  if (u  <  1000000) {   // 1,000,000
    if (u >= 100000) goto lt1_000_000;
    digits = u / 10000;  //    10,000
    *buffer++ = '0' + digits;
    goto sublt10_000;
  }
  if (u  <  100000000) {   // 100,000,000
    if (u >= 10000000) goto lt100_000_000;
    digits = u / 1000000;  //   1,000,000
    *buffer++ = '0' + digits;
    goto sublt1_000_000;
  }
  // we already know that u < 1,000,000,000
  digits = u / 100000000;   // 100,000,000
  *buffer++ = '0' + digits;
  goto sublt100_000_000;
}


// ----------------------------------------------------------------------
// AutoDigitStrCmp
// AutoDigitLessThan
// StrictAutoDigitLessThan
// autodigit_less
// autodigit_greater
// strict_autodigit_less
// strict_autodigit_greater
//    These are like less<string> and greater<string>, except when a
//    run of digits is encountered at corresponding points in the two
//    arguments.  Such digit strings are compared numerically instead
//    of lexicographically.  Therefore if you sort by
//    "autodigit_less", some machine names might get sorted as:
//        exaf1
//        exaf2
//        exaf10
//    When using "strict" comparison (AutoDigitStrCmp with the strict flag
//    set to true, or the strict version of the other functions),
//    strings that represent equal numbers will not be considered equal if
//    the string representations are not identical.  That is, "01" < "1" in
//    strict mode, but "01" == "1" otherwise.
// ----------------------------------------------------------------------

int AutoDigitStrCmp(const char* a, int alen,
                    const char* b, int blen,
                    bool strict) {
  int aindex = 0;
  int bindex = 0;
  while ((aindex < alen) && (bindex < blen)) {
    if (isdigit(a[aindex]) && isdigit(b[bindex])) {
      // Compare runs of digits.  Instead of extracting numbers, we
      // just skip leading zeroes, and then get the run-lengths.  This
      // allows us to handle arbitrary precision numbers.  We remember
      // how many zeroes we found so that we can differentiate between
      // "1" and "01" in strict mode.

      // Skip leading zeroes, but remember how many we found
      int azeroes = aindex;
      int bzeroes = bindex;
      while ((aindex < alen) && (a[aindex] == '0')) aindex++;
      while ((bindex < blen) && (b[bindex] == '0')) bindex++;
      azeroes = aindex - azeroes;
      bzeroes = bindex - bzeroes;

      // Count digit lengths
      int astart = aindex;
      int bstart = bindex;
      while ((aindex < alen) && isdigit(a[aindex])) aindex++;
      while ((bindex < blen) && isdigit(b[bindex])) bindex++;
      if (aindex - astart < bindex - bstart) {
        // a has shorter run of digits: so smaller
        return -1;
      } else if (aindex - astart > bindex - bstart) {
        // a has longer run of digits: so larger
        return 1;
      } else {
        // Same lengths, so compare digit by digit
        for (int i = 0; i < aindex-astart; i++) {
          if (a[astart+i] < b[bstart+i]) {
            return -1;
          } else if (a[astart+i] > b[bstart+i]) {
            return 1;
          }
        }
        // Equal: did one have more leading zeroes?
        if (strict && azeroes != bzeroes) {
          if (azeroes > bzeroes) {
            // a has more leading zeroes: a < b
            return -1;
          } else {
            // b has more leading zeroes: a > b
            return 1;
          }
        }
        // Equal: so continue scanning
      }
    } else if (a[aindex] < b[bindex]) {
      return -1;
    } else if (a[aindex] > b[bindex]) {
      return 1;
    } else {
      aindex++;
      bindex++;
    }
  }

  if (aindex < alen) {
    // b is prefix of a
    return 1;
  } else if (bindex < blen) {
    // a is prefix of b
    return -1;
  } else {
    // a is equal to b
    return 0;
  }
}

bool AutoDigitLessThan(const char* a, int alen, const char* b, int blen) {
  return AutoDigitStrCmp(a, alen, b, blen, false) < 0;
}

bool StrictAutoDigitLessThan(const char* a, int alen,
                             const char* b, int blen) {
  return AutoDigitStrCmp(a, alen, b, blen, true) < 0;
}

// ----------------------------------------------------------------------
// SimpleDtoa()
// SimpleFtoa()
// DoubleToBuffer()
// FloatToBuffer()
//    We want to print the value without losing precision, but we also do
//    not want to print more digits than necessary.  This turns out to be
//    trickier than it sounds.  Numbers like 0.2 cannot be represented
//    exactly in binary.  If we print 0.2 with a very large precision,
//    e.g. "%.50g", we get "0.2000000000000000111022302462515654042363167".
//    On the other hand, if we set the precision too low, we lose
//    significant digits when printing numbers that actually need them.
//    It turns out there is no precision value that does the right thing
//    for all numbers.
//
//    Our strategy is to first try printing with a precision that is never
//    over-precise, then parse the result with strtod() to see if it
//    matches.  If not, we print again with a precision that will always
//    give a precise result, but may use more digits than necessary.
//
//    An arguably better strategy would be to use the algorithm described
//    in "How to Print Floating-Point Numbers Accurately" by Steele &
//    White, e.g. as implemented by David M. Gay's dtoa().  It turns out,
//    however, that the following implementation is about as fast as
//    DMG's code.  Furthermore, DMG's code locks mutexes, which means it
//    will not scale well on multi-core machines.  DMG's code is slightly
//    more accurate (in that it will never use more digits than
//    necessary), but this is probably irrelevant for most users.
//
//    Rob Pike and Ken Thompson also have an implementation of dtoa() in
//    third_party/fmt/fltfmt.cc.  Their implementation is similar to this
//    one in that it makes guesses and then uses strtod() to check them.
//    Their implementation is faster because they use their own code to
//    generate the digits in the first place rather than use snprintf(),
//    thus avoiding format string parsing overhead.  However, this makes
//    it considerably more complicated than the following implementation,
//    and it is embedded in a larger library.  If speed turns out to be
//    an issue, we could re-implement this in terms of their
//    implementation.
// ----------------------------------------------------------------------

string SimpleDtoa(double value) {
  char buffer[kDoubleToBufferSize];
  return DoubleToBuffer(value, buffer);
}

string SimpleFtoa(float value) {
  char buffer[kFloatToBufferSize];
  return FloatToBuffer(value, buffer);
}

char* DoubleToBuffer(double value, char* buffer) {
  // DBL_DIG is 15 for IEEE-754 doubles, which are used on almost all
  // platforms these days.  Just in case some system exists where DBL_DIG
  // is significantly larger -- and risks overflowing our buffer -- we have
  // this assert.
  COMPILE_ASSERT(DBL_DIG < 20, DBL_DIG_is_too_big);

  int snprintf_result =
    snprintf(buffer, kDoubleToBufferSize, "%.*g", DBL_DIG, value);

  // The snprintf should never overflow because the buffer is significantly
  // larger than the precision we asked for.
  DCHECK(snprintf_result > 0 && snprintf_result < kDoubleToBufferSize);

  if (strtod(buffer, NULL) != value) {
    snprintf_result =
      snprintf(buffer, kDoubleToBufferSize, "%.*g", DBL_DIG+2, value);

    // Should never overflow; see above.
    DCHECK(snprintf_result > 0 && snprintf_result < kDoubleToBufferSize);
  }
  return buffer;
}

char* FloatToBuffer(float value, char* buffer) {
  // FLT_DIG is 6 for IEEE-754 floats, which are used on almost all
  // platforms these days.  Just in case some system exists where FLT_DIG
  // is significantly larger -- and risks overflowing our buffer -- we have
  // this assert.
  COMPILE_ASSERT(FLT_DIG < 10, FLT_DIG_is_too_big);

  int snprintf_result =
    snprintf(buffer, kFloatToBufferSize, "%.*g", FLT_DIG, value);

  // The snprintf should never overflow because the buffer is significantly
  // larger than the precision we asked for.
  DCHECK(snprintf_result > 0 && snprintf_result < kFloatToBufferSize);

  float parsed_value;
  if (!safe_strtof(buffer, &parsed_value) || parsed_value != value) {
    snprintf_result =
      snprintf(buffer, kFloatToBufferSize, "%.*g", FLT_DIG+2, value);

    // Should never overflow; see above.
    DCHECK(snprintf_result > 0 && snprintf_result < kFloatToBufferSize);
  }
  return buffer;
}

// ----------------------------------------------------------------------
// SimpleItoaWithCommas()
//    Description: converts an integer to a string.
//    Puts commas every 3 spaces.
//    Faster than printf("%d")?
//
//    Return value: string
// ----------------------------------------------------------------------
string SimpleItoaWithCommas(int32 i) {
  // 10 digits, 3 commas, and sign are good for 32-bit or smaller ints.
  // Longest is -2,147,483,648.
  char local[14];
  char *p = local + sizeof(local);
  // Need to use uint32 instead of int32 to correctly handle
  // -2,147,483,648.
  uint32 n = i;
  if (i < 0)
    n = 0 - n;  // negate the unsigned value to avoid overflow
  *--p = '0' + n % 10;          // this case deals with the number "0"
  n /= 10;
  while (n) {
    *--p = '0' + n % 10;
    n /= 10;
    if (n == 0) break;

    *--p = '0' + n % 10;
    n /= 10;
    if (n == 0) break;

    *--p = ',';
    *--p = '0' + n % 10;
    n /= 10;
    // For this unrolling, we check if n == 0 in the main while loop
  }
  if (i < 0)
    *--p = '-';
  return string(p, local + sizeof(local));
}

// We need this overload because otherwise SimpleItoaWithCommas(5U) wouldn't
// compile.
string SimpleItoaWithCommas(uint32 i) {
  // 10 digits and 3 commas are good for 32-bit or smaller ints.
  // Longest is 4,294,967,295.
  char local[13];
  char *p = local + sizeof(local);
  *--p = '0' + i % 10;          // this case deals with the number "0"
  i /= 10;
  while (i) {
    *--p = '0' + i % 10;
    i /= 10;
    if (i == 0) break;

    *--p = '0' + i % 10;
    i /= 10;
    if (i == 0) break;

    *--p = ',';
    *--p = '0' + i % 10;
    i /= 10;
    // For this unrolling, we check if i == 0 in the main while loop
  }
  return string(p, local + sizeof(local));
}

string SimpleItoaWithCommas(int64 i) {
  // 19 digits, 6 commas, and sign are good for 64-bit or smaller ints.
  char local[26];
  char *p = local + sizeof(local);
  // Need to use uint64 instead of int64 to correctly handle
  // -9,223,372,036,854,775,808.
  uint64 n = i;
  if (i < 0)
    n = 0 - n;
  *--p = '0' + n % 10;          // this case deals with the number "0"
  n /= 10;
  while (n) {
    *--p = '0' + n % 10;
    n /= 10;
    if (n == 0) break;

    *--p = '0' + n % 10;
    n /= 10;
    if (n == 0) break;

    *--p = ',';
    *--p = '0' + n % 10;
    n /= 10;
    // For this unrolling, we check if n == 0 in the main while loop
  }
  if (i < 0)
    *--p = '-';
  return string(p, local + sizeof(local));
}

// We need this overload because otherwise SimpleItoaWithCommas(5ULL) wouldn't
// compile.
string SimpleItoaWithCommas(uint64 i) {
  // 20 digits and 6 commas are good for 64-bit or smaller ints.
  // Longest is 18,446,744,073,709,551,615.
  char local[26];
  char *p = local + sizeof(local);
  *--p = '0' + i % 10;          // this case deals with the number "0"
  i /= 10;
  while (i) {
    *--p = '0' + i % 10;
    i /= 10;
    if (i == 0) break;

    *--p = '0' + i % 10;
    i /= 10;
    if (i == 0) break;

    *--p = ',';
    *--p = '0' + i % 10;
    i /= 10;
    // For this unrolling, we check if i == 0 in the main while loop
  }
  return string(p, local + sizeof(local));
}

// ----------------------------------------------------------------------
// ItoaKMGT()
//    Description: converts an integer to a string
//    Truncates values to a readable unit: K, G, M or T
//    Opposite of atoi_kmgt()
//    e.g. 100 -> "100" 1500 -> "1500"  4000 -> "3K"   57185920 -> "45M"
//
//    Return value: string
// ----------------------------------------------------------------------
string ItoaKMGT(int64 i) {
  const char *sign = "", *suffix = "";
  if (i < 0) {
    // We lose some accuracy if the caller passes LONG_LONG_MIN, but
    // that's OK as this function is only for human readability
    if (i == std::numeric_limits<int64>::min()) i++;
    sign = "-";
    i = -i;
  }

  int64 val;

  if ((val = (i >> 40)) > 1) {
    suffix = "T";
  } else if ((val = (i >> 30)) > 1) {
    suffix = "G";
  } else if ((val = (i >> 20)) > 1) {
    suffix = "M";
  } else if ((val = (i >> 10)) > 1) {
    suffix = "K";
  } else {
    val = i;
  }

  return StringPrintf("%s%" PRId64 "d%s", sign, val, suffix);
}

// DEPRECATED(wadetregaskis).
// These are non-inline because some BUILD files turn on -Wformat-non-literal.

string FloatToString(float f, const char* format) {
  return StringPrintf(format, f);
}

string IntToString(int i, const char* format) {
  return StringPrintf(format, i);
}

string Int64ToString(int64 i64, const char* format) {
  return StringPrintf(format, i64);
}

string UInt64ToString(uint64 ui64, const char* format) {
  return StringPrintf(format, ui64);
}

// DEPRECATED(wadetregaskis).  Just call StringPrintf.
string FloatToString(float f) {
  return StringPrintf("%7f", f);
}

// DEPRECATED(wadetregaskis).  Just call StringPrintf.
string IntToString(int i) {
  return StringPrintf("%7d", i);
}

// DEPRECATED(wadetregaskis).  Just call StringPrintf.
string Int64ToString(int64 i64) {

  return StringPrintf("%7" PRId64, i64);
}

// DEPRECATED(wadetregaskis).  Just call StringPrintf.
string UInt64ToString(uint64 ui64) {
  return StringPrintf("%7" PRIu64, ui64);
}
