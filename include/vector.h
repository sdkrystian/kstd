#pragma once
#include <memory>

namespace kstd
{
  template<typename T, typename = void>
  struct allocator_base
  {
  public:
    T get_allocator() const noexcept
    {
      return allocator_;
    }
  protected:
    T& allocator() noexcept
    {
      return allocator_;
    }
  private:
    T allocator_;
  };

  template<typename T>
  struct allocator_base<T, std::enable_if_t<!std::is_final_v<T>>> : T
  {
  public:
    T get_allocator() const noexcept
    {
      return *static_cast<T*>(this);
    }
  protected:
    T& allocator() noexcept
    {
      return *static_cast<T*>(this);
    }
  };

  template<typename T, typename Allocator = std::allocator<T>>
  class vector : public allocator_base<Allocator>
  {
  public:
    // Typedefs
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Constructors

    // Capacity
    bool empty() const noexcept
    {
      return !size_;
    }

    size_type size() const noexcept
    {
      return size_;
    }

    size_type capactiy() const noexcept
    {
      return capacity_;
    }
  private:
    pointer data_;
    size_type size_;
    size_type capacity_;
  };
}