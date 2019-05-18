#pragma once
#include <memory>
#include <utility>
#include <type_traits>

#define ALLOW_UB

namespace kstd
{
  namespace detail
  {
    template<typename InputIterator, typename OutputIterator>
    OutputIterator uninitialized_move_range_optimal(InputIterator first, InputIterator last, OutputIterator d_first)
    {
      using T = typename std::iterator_traits<InputIterator>::value_type;
#ifdef ALLOW_UB
      if constexpr (std::is_trivial_v<T>)
        return OutputIterator(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
#else
      if (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
#endif
        return std::uninitialized_move(first, last, d_first);
      else
        return std::uninitialized_copy(first, last, d_first);
    }

    template<typename InputIterator, typename OutputIterator>
    OutputIterator move_range_optimal(InputIterator first, InputIterator last, OutputIterator d_first)
    {
      using T = typename std::iterator_traits<InputIterator>::value_type;
#ifdef ALLOW_UB
      if constexpr (std::is_trivial_v<T>)
        return OutputIterator(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else if constexpr (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
#else
      if (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
#endif
        return std::move(first, last, d_first);
      else
        return std::copy(first, last, d_first);
    }

    template<typename InputIterator, typename OutputIterator>
    OutputIterator move_range_optimal_backward(InputIterator first, InputIterator last, OutputIterator d_first)
    {
      using T = typename std::iterator_traits<InputIterator>::value_type;
#ifdef ALLOW_UB
      if constexpr (std::is_trivial_v<T>)
        return OutputIterator(std::memmove(d_first, first, (last - first) * sizeof(T)));
      else if constexpr (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
#else
      if (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
#endif
        return std::move_backward(first, last, d_first);
      else
        return std::copy_backward(first, last, d_first);
    }

    template<typename InputIterator, typename OutputIterator>
    OutputIterator copy_range_optimal(InputIterator first, InputIterator last, OutputIterator d_first)
    {
#ifdef ALLOW_UB
      using T = typename std::iterator_traits<InputIterator>::value_type;
      if constexpr (std::is_trivial_v<T>)
        return OutputIterator(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else
#endif
        return std::copy(first, last, d_first);
    }

    template<typename InputIterator, typename OutputIterator>
    OutputIterator uninitialized_copy_range_optimal(InputIterator first, InputIterator last, OutputIterator d_first)
    {
#ifdef ALLOW_UB
      using T = typename std::iterator_traits<InputIterator>::value_type;
      if constexpr (std::is_trivial_v<T>)
        return OutputIterator(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else
#endif
        return std::uninitialized_copy(first, last, d_first);
    }

    template<typename ForwardIt, typename T>
    void uninitialized_fill_range_optimal(ForwardIt first, ForwardIt last, const T& value)
    {
#ifdef ALLOW_UB
      if constexpr (std::is_trivial_v<T>)
        for (; first != last; ++first)
          std::memcpy(first, &value, sizeof(T));
      else
#endif
        std::uninitialized_fill(first, last, value);
    }

    template<typename ForwardIt, typename T>
    void fill_range_optimal(ForwardIt first, ForwardIt last, const T& value)
    {
#ifdef ALLOW_UB
      if constexpr (std::is_trivial_v<T>)
        for (; first != last; ++first)
          std::memcpy(first, &value, sizeof(T));
      else
#endif
        std::fill(first, last, value);
    }
  }
}