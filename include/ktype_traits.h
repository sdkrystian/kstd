#pragma once
#include <iterator>

namespace kstd
{
  namespace detail
  {
    template<class, class = void>
    struct is_iterator : std::false_type { };

    template<class T>
    struct is_iterator<T, decltype(std::iterator_traits<T>::iterator_category(), void())> : std::true_type { };

    template<typename T>
    constexpr bool is_iterator_v = is_iterator<T>::value;
  }
}