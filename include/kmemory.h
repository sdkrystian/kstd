#pragma once
#include <memory>

namespace kstd
{
  template<typename InputIt, typename ForwardIt>
  ForwardIt uninitialized_move_if_noexcept(InputIt first, InputIt last, ForwardIt d_first)
  {
    if constexpr (std::is_nothrow_move_constructible_v<std::iterator_traits<InputIt>::value_type>)
      return std::uninitialized_move(first, last, d_first);
    else
      return std::uninitialized_copy(first, last, d_first);
  }
}