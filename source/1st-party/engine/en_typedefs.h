/* en_typedefs.h */
// ===================================80 chars=================================|

#ifndef EN_TYPEDEFS_H
#define EN_TYPEDEFS_H

// Required for fixed width integer types
#include <cstdint>
// NOT IN USE. For now, using unsigned long long int for frameID)
// #include "BigIntegerLibrary.hh"

typedef unsigned char byte;
typedef std::int8_t int8;
typedef std::uint8_t uint8;
typedef std::int16_t int16;
typedef std::uint16_t uint16;
typedef unsigned int uint;
typedef std::int32_t int32;
typedef std::uint32_t uint32;
typedef std::int64_t int64;
typedef std::uint64_t uint64;
typedef wchar_t wchar;
typedef std::string t_objectName;

// TODO rename this to sysTimePoint
typedef std::chrono::time_point <std::chrono::system_clock> stdTimePoint;
typedef std::chrono::steady_clock::time_point stdyTimePoint;

// EN_TYPEDEFS_H
#endif

// ===================================80 chars=================================|
/* EOF */

