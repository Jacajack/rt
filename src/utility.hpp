#pragma once

#define RT_CACHE_LINE_SIZE 64

namespace rt {

template <typename T = float>
inline constexpr T pi = static_cast<T>(3.141592653589793238462643383279502884l);

}