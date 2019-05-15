#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include "kmemory.h"

namespace kstd
{
  namespace detail
  {
    template<typename T, typename U>
    T* uninitialized_move_range_optimal(T* first, T* last, U* d_first) // pointers so it works with memcpy
    {
      if constexpr (std::is_trivial_v<T>)
        return static_cast<T*>(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else if constexpr(std::is_nothrow_move_constructible_v<T>)
        return std::uninitialized_move(first, last, d_first);
      else
        return std::uninitialized_copy(first, last, d_first);
    }

    template<typename T, typename U>
    T* move_range_optimal(T* first, T* last, U* d_first) // pointers so it works with memcpy
    {
      if constexpr (std::is_trivial_v<T>)
        return static_cast<T*>(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else if constexpr (std::is_nothrow_move_constructible_v<T>)
        return std::move(first, last, d_first);
      else
        return std::copy(first, last, d_first);
    }

    template<typename T, typename U>
    T* move_range_optimal_backward(T* first, T* last, U* d_first) // pointers so it works with memcpy
    {
      if constexpr (std::is_trivial_v<T>)
        return static_cast<T*>(std::memmove(d_first, first, (last - first) * sizeof(T)));
      else if constexpr (std::is_nothrow_move_constructible_v<T>)
        return std::move_backward(first, last, d_first);
      else
        return std::copy_backward(first, last, d_first);
    }

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
  }

  template<typename T, typename Allocator = std::allocator<T>>
  class vector : public detail::allocator_base<Allocator>
  {
  public:
    // typedefs
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // constructors

    // iterators
    iterator begin() noexcept
    {
      return data_;
    }

    const_iterator begin() const noexcept
    {
      return data_;
    }

    iterator end() noexcept
    {
      return data_ + size_;
    }

    const_iterator end() const noexcept
    {
      return data_ + size_;
    }

    reverse_iterator rbegin() noexcept
    {
      return data_ + size_;
    }

    const_reverse_iterator rbegin() const noexcept
    {
      return data_ + size_;
    }

    reverse_iterator rend() noexcept
    {
      return data_;
    }

    const_reverse_iterator rend() const noexcept
    {
      return data_;
    }

    const_iterator cbegin() const noexcept
    {
      return data_;
    }

    const_iterator cend() const noexcept
    {
      return data_ + size_;
    }

    const_reverse_iterator crbegin() const noexcept
    {
      return data_ + size_;
    }

    const_reverse_iterator crend() const noexcept
    {
      return data_;
    }

    // capacity
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

    void resize(size_type sz)
    {

    }

    void resize(size_type sz, const T& t)
    {

    }

    void reserve(size_type cap)
    {
      reserve_offset(cap, size_, 0);
    }

    void shrink_to_fit()
    {
      return;
    }

    // element access
    reference operator[](size_type n)
    {
      return data_[n];
    }
    
    const_reference operator[](size_type n) const
    {
      return data_[n];
    }
    
    reference at(size_type n)
    {
      if (n >= size_)
        throw std::out_of_range("n is out of range");
      return data_[n];
    }
    
    const_reference at(size_type n) const
    {
      return at(n);
    }
    
    reference front()
    {
      return *data_;
    }
    
    const_reference front() const
    {
      return *data_;
    }
    
    reference back()
    {
      return data_[size_ - 1];
    }
    
    const_reference back() const
    {
      return data_[size_ - 1];
    }

    // data access
    T* data() noexcept
    {
      return data_;
    }

    const T* data() const noexcept
    {
      return data_;
    }

    // modifiers
    template<class... Args>
    reference emplace_back(Args&&... args)
    {
      reserve(size_ + 1);
      new (data_ + size_) T(std::forward<Args>(args)...);
      ++size_;
      return back();
    }

    void push_back(const T& value)
    {
      reserve(size_ + 1);
      new (data_ + size_) T(value);
      ++size_;
    }

    void push_back(T&& value)
    {
      reserve(size_ + 1);
      new (data_ + size_) T(std::move(value));
      ++size_;
    }

    void pop_back()
    {
      if (!std::is_trivially_destructible_v<T>)
        back().~T();
      --size_;
    }

    template<class... Args> 
    iterator emplace(const_iterator pos, Args&&... args)
    {
      size_type emplaced_pos = pos - begin();
      if (!reserve_insert(size_ + 1, emplaced_pos, 1) && emplaced_pos < size_)
      {
        detail::uninitialized_move_range_optimal(data_ + size_ - 1, data_ + size_, data_ + size_);
        detail::move_range_optimal_backward(data_ + emplaced_pos, data_ + size_ - 1, data_ + emplaced_pos + 1);
        *(data_ + emplaced_pos) = T(std::forward<Args>(args)...);
      }
      else
      {
        new (data_ + emplaced_pos) T(std::forward<Args>(args)...);
      }
      ++size_;
      return data_ + emplaced_pos;
    }

  private:
    /*template<class... Args>
    iterator emplace_range(const_iterator pos,  Args&&... args)
    {
      size_type emplaced_pos = pos - begin();
      if (!reserve_insert(size_ + 1, emplaced_pos, 1) && emplaced_pos < size_)
      {
        detail::uninitialized_move_range_optimal(data_ + size_ - 1, data_ + size_, data_ + size_);
        detail::move_range_optimal_backward(data_ + emplaced_pos, data_ + size_ - 1, data_ + emplaced_pos + 1);
        *(data_ + emplaced_pos) = T(std::forward<Args>(args)...);
      }
      else
      {
        new (data_ + emplaced_pos) T(std::forward<Args>(args)...);
      }
      ++size_;
      return data_ + emplaced_pos;
    }*/

    bool reserve_insert(size_type cap, size_type pos, int count)
    {
      return reserve_offset(cap, pos, count);
    }

    bool reserve_erase(size_type cap, size_type pos, int count)
    {
      return reserve_offset(cap, pos, -count);
    }

    bool reserve_offset(size_type cap, size_type pos, int count)
    {
      if (cap <= capacity_)
        return false;
      if (data_ != nullptr)
      {
        pointer new_data = std::allocator_traits<Allocator>::allocate(allocator(), cap);
        detail::uninitialized_move_range_optimal(data_, data_ + pos, new_data);
        if (count < 0)
          detail::uninitialized_move_range_optimal(data_ + pos - count, data_ + size_, new_data + pos);
        else
          detail::uninitialized_move_range_optimal(data_ + pos, data_ + size_, new_data + pos + count);
        if constexpr (!std::is_trivial_v<T>)
          std::destroy(data_, data_ + size_);
        std::allocator_traits<Allocator>::deallocate(allocator(), data_, capacity_);
        data_ = new_data;
      }
      else
      {
        data_ = std::allocator_traits<Allocator>::allocate(allocator(), cap);
      }
      capacity_ = cap;
      return true;
    }
    using detail::allocator_base<Allocator>::allocator;

    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
  };
}