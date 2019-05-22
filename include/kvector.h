#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include "kmemory.h"
#include "ktype_traits.h"

namespace kstd
{
  namespace detail
  {
    template<typename T, typename = void>
    struct allocator_base
    {
    public:
      allocator_base() noexcept(noexcept(T())) : allocator_(T()) { }

      allocator_base(const T& alloc) noexcept : allocator_(traits::select_on_container_copy_construction(alloc)) { }

      allocator_base(T&& alloc) noexcept : allocator_(std::move(alloc)) { }

      allocator_base operator=(const T& alloc)
      {
        if constexpr (traits::propagate_on_container_copy_assignment::value)
          allocator_ = alloc;
        return *this;
      }

      allocator_base operator=(T&& alloc) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value)
      {
        if constexpr (traits::propagate_on_container_move_assignment::value)
          allocator_ = std::move(alloc);
        return *this;
      }

      using traits = std::allocator_traits<T>;
    protected:
      T& allocator() noexcept
      {
        return allocator_;
      }

      const T& allocator() const noexcept
      {
        return allocator_;
      }
    private:
      T allocator_;
    };

    template<typename T>
    struct allocator_base<T, std::void_t<T>> : protected T // protected because intellisense thinks inherited members are still accessable >:(
    {
    public:
      allocator_base() noexcept(noexcept(T())) : T(T()) { }

      allocator_base(const T& alloc) noexcept : T(traits::select_on_container_copy_construction(alloc)) { }

      allocator_base(T&& alloc) noexcept : T(std::move(alloc)) { }

      allocator_base operator=(const T& alloc)
      {
        if constexpr (traits::propagate_on_container_copy_assignment::value)
          allocator() = alloc;
        return *this;
      }

      allocator_base operator=(T&& alloc) noexcept(traits::propagate_on_container_move_assignment::value || traits::is_always_equal::value)
      {
        if constexpr (traits::propagate_on_container_move_assignment::value)
          allocator() = std::move(alloc);
        return *this;
      }

      using traits = std::allocator_traits<T>;
    protected:
      T& allocator() noexcept
      {
        return *static_cast<T*>(this);
      }

      const T& allocator() const noexcept
      {
        return *static_cast<T*>(this);
      }
    };

    template<typename Alloc, typename ForwardIt>
    void destroy_alloc(Alloc& alloc, ForwardIt first, ForwardIt last)
    {
      if constexpr (!std::is_trivial_v<std::iterator_traits<ForwardIt>::value_type>)
        for (; first != last; ++first)
          std::allocator_traits<Alloc>::destroy(alloc, std::addressof(*first));
    }

    template<typename Alloc, typename InputIt, typename OutputIt>
    OutputIt uninitialized_move_alloc(Alloc& alloc, InputIt first, InputIt last, OutputIt d_first)
    {
      for (; first != last; ++first, ++d_first)
        std::allocator_traits<Alloc>::construct(alloc, std::addressof(*d_first), std::move(*first)); // Yeah, its UB, I know
      return d_first;
    }

    template<typename Alloc, typename InputIt, typename OutputIt>
    OutputIt uninitialized_copy_alloc(Alloc& alloc, InputIt first, InputIt last, OutputIt d_first)
    {
      for (; first != last; ++first, ++d_first)
        std::allocator_traits<Alloc>::construct(alloc, std::addressof(*d_first), *first); // Yeah, its UB, I know
      return d_first;
    }

    template<typename Alloc, typename ForwardIt, typename T>
    ForwardIt uninitialized_fill_alloc(Alloc& alloc, ForwardIt first, ForwardIt last, const T& value)
    {
      for (; first != last; ++first)
        std::allocator_traits<Alloc>::construct(alloc, std::addressof(*first), value); // Yeah, its UB, I know
      return first;
    }

    template<typename Alloc, typename ForwardIt>
    ForwardIt uninitialized_default_fill_alloc(Alloc& alloc, ForwardIt first, ForwardIt last)
    {
      for (; first != last; ++first)
        std::allocator_traits<Alloc>::construct(alloc, std::addressof(*first)); // Yeah, its UB, I know
      return first;
    }

    template<typename Alloc, typename InputIterator, typename OutputIterator>
    OutputIterator uninitialized_move_range_optimal_alloc(Alloc& alloc, InputIterator first, InputIterator last, OutputIterator d_first)
    {
      using T = typename std::iterator_traits<InputIterator>::value_type;
#ifdef ALLOW_UB
      if constexpr (std::is_trivial_v<T>)
        return OutputIterator(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
#else
      if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
#endif
        return uninitialized_move_alloc(alloc, first, last, d_first);
      else
        return uninitialized_copy_alloc(alloc, first, last, d_first);
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
      if constexpr (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
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
      if constexpr (std::is_nothrow_move_assignable_v<T> || !std::is_copy_assignable_v<T>)
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

    template<typename Alloc, typename InputIterator, typename OutputIterator>
    OutputIterator uninitialized_copy_range_optimal_alloc(Alloc& alloc, InputIterator first, InputIterator last, OutputIterator d_first)
    {
#ifdef ALLOW_UB
      using T = typename std::iterator_traits<InputIterator>::value_type;
      if constexpr (std::is_trivial_v<T>)
        return OutputIterator(std::memcpy(d_first, first, (last - first) * sizeof(T)));
      else
#endif
        return uninitialized_copy_alloc(alloc, first, last, d_first);
    }

    template<typename Alloc, typename ForwardIt, typename T>
    void uninitialized_fill_range_optimal_alloc(Alloc& alloc, ForwardIt first, ForwardIt last, const T& value)
    {
#ifdef ALLOW_UB
      if constexpr (std::is_trivial_v<T>)
        for (; first != last; ++first)
          std::memcpy(first, &value, sizeof(T));
      else
#endif
        detail::uninitialized_fill_alloc(alloc, first, last, value);
    }

    template<typename Alloc, typename ForwardIterator>
    void uninitialized_default_fill_range_optimal_alloc(Alloc& alloc, ForwardIterator first, ForwardIterator last)
    {
      if constexpr (!std::is_trivial_v<std::iterator_traits<ForwardIterator>::value_type>)
        detail::uninitialized_default_fill_alloc(alloc, first, last);
    }

    template<typename ForwardIterator, typename T>
    void fill_range_optimal(ForwardIterator first, ForwardIterator last, const T& value)
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

  template<typename T, typename Allocator = std::allocator<T>>
  class vector : protected detail::allocator_base<Allocator>
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
    vector() noexcept(noexcept(Allocator())) : vector(Allocator()) { }

    explicit vector(const Allocator& alloc) noexcept : detail::allocator_base(alloc) { }

    explicit vector(size_type n, const Allocator& alloc = Allocator()) : detail::allocator_base(alloc) 
    {
      reserve(n);
      detail::uninitialized_default_fill_range_optimal_alloc(allocator(), data_, data_ + n);
    }

    vector(size_type n, const T& value, const Allocator& alloc = Allocator()) : detail::allocator_base(alloc), size_(n)
    {
      reserve(n);
      detail::uninitialized_fill_range_optimal_alloc(allocator(), data_, data_ + size_, value);
    }

    template<class InputIterator, typename = std::enable_if_t<detail::is_iterator_v<InputIterator>>>
    vector(InputIterator first, InputIterator last, const Allocator& alloc = Allocator()) : detail::allocator_base(alloc), size_(last - first)
    {
      reserve(size_);
      detail::uninitialized_copy_range_optimal_alloc(allocator(), first, last, data_);
    }

    vector(const vector& other) : detail::allocator_base(other.allocator()), size_(other.size_)
    {
      reserve(other.capacity_);
      detail::uninitialized_copy_range_optimal_alloc(allocator(), other.data_, other.data_ + size_, data_);
    }

    vector(vector&& other) noexcept : detail::allocator_base(std::move(other.allocator())), size_(other.size_), capacity_(other.capacity_), data_(other.data_)
    {
      other.size_ = 0;
      other.capacity_ = 0;
      other.data_ = nullptr;
    }
    
    vector(const vector& other, const Allocator& alloc) : detail::allocator_base(alloc), size_(other.size_)
    {
      reserve(other.capacity_);
      detail::uninitialized_copy_range_optimal_alloc(allocator(), other.data_, other.data_ + size_, data_);
    }
    
    vector(vector&& other, const Allocator& alloc) noexcept : detail::allocator_base(alloc), size_(other.size_), capacity_(other.capacity_), data_(other.data_)
    {
      other.size_ = 0;
      other.capacity_ = 0;
      other.data_ = nullptr;
    }

    vector(std::initializer_list<T> list, const Allocator& alloc = Allocator()) : detail::allocator_base(alloc), size_(list.size())
    {
      reserve(size_);
      detail::uninitialized_copy_range_optimal_alloc(allocator(), list.begin(), list.end(), data_);
    }

    ~vector()
    {
      if (data_)
      {
        detail::destroy_alloc(allocator(), data_, data_ + size_);
        traits::deallocate(allocator(), data_, capacity_);
      }
    }

    Allocator get_allocator() const noexcept
    {
      return allocator();
    }

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

    size_type capacity() const noexcept
    {
      return capacity_;
    }

    void resize(size_type sz)
    {

    }

    void resize(size_type sz, const T& value)
    {

    }

    inline void reserve(size_type cap)
    {
      if (needs_to_reallocate(cap + 1))
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
      if (n >= size_)
        throw std::out_of_range("n is out of range");
      return data_[n];
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
    template<typename... Args>
    reference emplace_back(Args&&... args)
    {
      reserve(size_ + 1);
      traits::construct(allocator(), data_ + size_, std::forward<Args>(args)...);
      ++size_;
      return *(data_ + size_ - 1);
    }

    void push_back(const T& value)
    {
      reserve(size_ + 1);
      traits::construct(allocator(), data_ + size_, value);
      ++size_;
    }

    void push_back(T&& value)
    {
      reserve(size_ + 1);
      traits::construct(allocator(), data_ + size_, std::move(value));
      ++size_;
    }

    void pop_back()
    {
      if constexpr (!std::is_trivially_destructible_v<T>)
        traits::destroy(allocator(), data_ + size_ - 1);
      --size_;
    }

    template<typename... Args> 
    iterator emplace(const_iterator pos, Args&&... args)
    {
      size_type emplaced_pos = pos - begin();
      if (!shift_elements_right(emplaced_pos, 1) && emplaced_pos < size_)
        *(data_ + emplaced_pos) = T(std::forward<Args>(args)...);
      else
        traits::construct(allocator(), data_ + emplaced_pos, std::forward<Args>(args)...);
      ++size_;
      return data_ + emplaced_pos;
    }

    iterator insert(const_iterator pos, const T& value)
    {
      return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value)
    {
      return emplace(pos, std::move(value));
    }

    iterator insert(const_iterator pos, size_type count, const T& value)
    {
      size_type inserted_pos = pos - begin();
      if (needs_to_reallocate(size_ + count))
      {
        reserve_offset(size_ + count, inserted_pos, count);
        detail::uninitialized_fill_range_optimal_alloc(allocator(), data_ + inserted_pos, data_ + inserted_pos + count, value);
      }
      else
      {
        size_type uninit_to_copy = std::clamp((inserted_pos + count) - size_, size_type(0), count);
        shift_elements_right(inserted_pos, count);
        detail::fill_range_optimal(data_ + inserted_pos, data_ + inserted_pos + count - uninit_to_copy, value);
        detail::uninitialized_fill_range_optimal_alloc(allocator(), data_ + inserted_pos + count - uninit_to_copy, data_ + inserted_pos + count, value);
      }
      size_ += count;
      return data_ + inserted_pos;
    }

    template<typename InputIterator, typename = std::enable_if_t<detail::is_iterator_v<InputIterator>>>
    iterator insert(const_iterator pos, InputIterator first, InputIterator last)
    {
      size_type inserted_pos = pos - begin();
      size_type count = last - first;
      if (needs_to_reallocate(size_ + count))
      {
        reserve_offset(size_ + count, inserted_pos, count);
        detail::uninitialized_copy_range_optimal_alloc(allocator(), first, last, data_ + inserted_pos);
      }
      else
      {
        shift_elements_right(inserted_pos, count);
        size_type uninit_to_copy = std::clamp((inserted_pos + count) - size_, size_type(0), count);
        detail::copy_range_optimal(first, last - uninit_to_copy, data_ + inserted_pos);
        detail::uninitialized_copy_range_optimal_alloc(allocator(), last - uninit_to_copy, last, data_ + inserted_pos + count - uninit_to_copy);
      }
      size_ += count;
      return data_ + inserted_pos;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> list)
    {
      return insert(pos, list.begin(), list.end());
    }

    iterator erase(const_iterator first, const_iterator last)
    {
      size_type count = last - first;
      size_type pos = first - begin();
      detail::move_range_optimal(data_ + pos + count, data_ + size_, data_ + pos);
      detail::destroy_alloc(allocator(), data_ + size_ - count, data_ + size_);
      size_ -= count;
      return data_ + pos + 1;
    }

    iterator erase(const_iterator pos)
    {
      return erase(pos, pos + 1);
    }

    void clear() noexcept
    {
      erase(begin(), end());
    }
  private:
    inline bool needs_to_reallocate(size_type cap)
    {
      return cap > capacity_;
    }

    void shift_elements_right(size_type pos, size_type count)
    {
      size_type uninit_to_move = std::clamp(size_ - pos, size_type(0), count);
      detail::uninitialized_move_range_optimal_alloc(allocator(), data_ + size_ - uninit_to_move, data_ + size_, data_ + size_ + count - uninit_to_move);
      detail::move_range_optimal_backward(data_ + pos, data_ + size_ - uninit_to_move, data_ + pos + count);
    }

    bool reserve_offset(size_type cap, size_type pos, size_type count)
    {
      size_type new_cap = capacity_ ? capacity_ : 1;
      for (; new_cap <= cap; new_cap <<= 1);
      if (data_ != nullptr)
      {
        pointer new_data = traits::allocate(allocator(), new_cap);
        detail::uninitialized_move_range_optimal_alloc(allocator(), data_, data_ + pos, new_data);
        detail::uninitialized_move_range_optimal_alloc(allocator(), data_ + pos, data_ + size_, new_data + pos + count);
        detail::destroy_alloc(allocator(), data_, data_ + size_);
        traits::deallocate(allocator(), data_, capacity_);
        data_ = new_data;
      }
      else
      {
        data_ = traits::allocate(allocator(), new_cap);
      }
      capacity_ = new_cap;
      return true;
    }
    using detail::allocator_base<Allocator>::allocator;
    using typename detail::allocator_base<Allocator>::traits;

    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
  };
}