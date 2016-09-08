///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
http://www.etlcpp.com

Copyright(c) 2014 jwellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef __ETL_IVECTOR__
#define __ETL_IVECTOR__
#define __ETL_IN_IVECTOR_H__

#include <iterator>
#include <algorithm>
#include <functional>
#include <stddef.h>

#include "platform.h"
#include "algorithm.h"
#include "private/vector_base.h"
#include "type_traits.h"
#include "parameter_type.h"
#include "error_handler.h"

#ifdef ETL_COMPILER_GCC
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

namespace etl
{
  //***************************************************************************
  /// The base class for specifically sized vectors.
  /// Can be used as a reference type for all vectors containing a specific type.
  ///\ingroup vector
  //***************************************************************************
  template <typename T>
  class ivector : public vector_base
  {
  public:

    typedef T                                     value_type;
    typedef T&                                    reference;
    typedef const T&                              const_reference;
    typedef T*                                    pointer;
    typedef const T*                              const_pointer;
    typedef T*                                    iterator;
    typedef const T*                              const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef size_t                                size_type;
    typedef typename std::iterator_traits<iterator>::difference_type difference_type;

  private:

    // Type trait to allow selection of simple algorithms for simple types.
    template <typename U>
    struct is_simple_type : integral_constant<bool, etl::is_fundamental<U>::value || etl::is_pointer<U>::value>
    {
    };

  protected:

    typedef typename parameter_type<T>::type parameter_t;

  public:

    //*********************************************************************
    /// Returns an iterator to the beginning of the vector.
    ///\return An iterator to the beginning of the vector.
    //*********************************************************************
    iterator begin()
    {
      return &p_buffer[0];
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the vector.
    ///\return A const iterator to the beginning of the vector.
    //*********************************************************************
    const_iterator begin() const
    {
      return &p_buffer[0];
    }

    //*********************************************************************
    /// Returns an iterator to the end of the vector.
    ///\return An iterator to the end of the vector.
    //*********************************************************************
    iterator end()
    {
      return &p_buffer[current_size];
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the vector.
    ///\return A const iterator to the end of the vector.
    //*********************************************************************
    const_iterator end() const
    {
      return &p_buffer[current_size];
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the vector.
    ///\return A const iterator to the beginning of the vector.
    //*********************************************************************
    const_iterator cbegin() const
    {
      return &p_buffer[0];
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the vector.
    ///\return A const iterator to the end of the vector.
    //*********************************************************************
    const_iterator cend() const
    {
      return &p_buffer[current_size];
    }

    //*********************************************************************
    /// Returns an reverse iterator to the reverse beginning of the vector.
    ///\return Iterator to the reverse beginning of the vector.
    //*********************************************************************
    reverse_iterator rbegin()
    {
      return reverse_iterator(end());
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the vector.
    ///\return Const iterator to the reverse beginning of the vector.
    //*********************************************************************
    const_reverse_iterator rbegin() const
    {
      return const_reverse_iterator(end());
    }

    //*********************************************************************
    /// Returns a reverse iterator to the end + 1 of the vector.
    ///\return Reverse iterator to the end + 1 of the vector.
    //*********************************************************************
    reverse_iterator rend()
    {
      return reverse_iterator(begin());
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the end + 1 of the vector.
    ///\return Const reverse iterator to the end + 1 of the vector.
    //*********************************************************************
    const_reverse_iterator rend() const
    {
      return const_reverse_iterator(begin());
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the vector.
    ///\return Const reverse iterator to the reverse beginning of the vector.
    //*********************************************************************
    const_reverse_iterator crbegin() const
    {
      return const_reverse_iterator(cend());
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the end + 1 of the vector.
    ///\return Const reverse iterator to the end + 1 of the vector.
    //*********************************************************************
    const_reverse_iterator crend() const
    {
      return const_reverse_iterator(cbegin());
    }

    //*********************************************************************
    /// Resizes the vector.
    /// If asserts or exceptions are enabled and the new size is larger than the
    /// maximum then a vector_full is thrown.
    ///\param new_size The new size.
    //*********************************************************************
    void resize(size_t new_size)
    {
      ETL_ASSERT(new_size <= MAX_SIZE, ETL_ERROR(vector_full));

      // Choose the algorithm according to type.
      // The unused branch should be optimised away.
      if (is_simple_type<value_type>::value)
      {
        current_size = new_size;
      }
      else
      {
        // Size up or size down?
        if (new_size > current_size)
        {
          while (current_size < new_size)
          {
            create_element();
          }
        }
        else if (new_size < current_size)
        {
          while (current_size > new_size)
          {
            destroy_element();
          }
        }
      }
    }

    //*********************************************************************
    /// Resizes the vector.
    /// If asserts or exceptions are enabled and the new size is larger than the
    /// maximum then a vector_full is thrown.
    ///\param new_size The new size.
    ///\param value   The value to fill new elements with. Default = default constructed value.
    //*********************************************************************
    void resize(size_t new_size, T value)
    {
      ETL_ASSERT(new_size <= MAX_SIZE, ETL_ERROR(vector_full));

      // Choose the algorithm according to type.
      // The unused branch should be optimised away.
      if (is_simple_type<value_type>::value)
      {
        // Size up if necessary.
        while (current_size < new_size)
        {
          p_buffer[current_size++] = value;
        }

        current_size = new_size;
      }
      else
      {
        // Size up?
        if (new_size > current_size)
        {
          while (current_size < new_size)
          {
            create_element(value);
          }
        }
        // Size down?
        else if (new_size < current_size)
        {
          while (current_size > new_size)
          {
            destroy_element();
          }
        }
      }
    }

    //*********************************************************************
    /// Returns a reference to the value at index 'i'
    ///\param i The index.
    ///\return A reference to the value at index 'i'
    //*********************************************************************
    reference operator [](size_t i)
    {
      return p_buffer[i];
    }

    //*********************************************************************
    /// Returns a const reference to the value at index 'i'
    ///\param i The index.
    ///\return A const reference to the value at index 'i'
    //*********************************************************************
    const_reference operator [](size_t i) const
    {
      return p_buffer[i];
    }

    //*********************************************************************
    /// Returns a reference to the value at index 'i'
    /// If asserts or exceptions are enabled, emits an etl::vector_out_of_bounds if the index is out of range.
    ///\param i The index.
    ///\return A reference to the value at index 'i'
    //*********************************************************************
    reference at(size_t i)
    {
      ETL_ASSERT(i < current_size, ETL_ERROR(vector_out_of_bounds));
      return p_buffer[i];
    }

    //*********************************************************************
    /// Returns a const reference to the value at index 'i'
    /// If asserts or exceptions are enabled, emits an etl::vector_out_of_bounds if the index is out of range.
    ///\param i The index.
    ///\return A const reference to the value at index 'i'
    //*********************************************************************
    const_reference at(size_t i) const
    {
      ETL_ASSERT(i < current_size, ETL_ERROR(vector_out_of_bounds));
      return p_buffer[i];
    }

    //*********************************************************************
    /// Returns a reference to the first element.
    ///\return A reference to the first element.
    //*********************************************************************
    reference front()
    {
      return p_buffer[0];
    }

    //*********************************************************************
    /// Returns a const reference to the first element.
    ///\return A const reference to the first element.
    //*********************************************************************
    const_reference front() const
    {
      return p_buffer[0];
    }

    //*********************************************************************
    /// Returns a reference to the last element.
    ///\return A reference to the last element.
    //*********************************************************************
    reference back()
    {
      return p_buffer[current_size - 1];
    }

    //*********************************************************************
    /// Returns a const reference to the last element.
    ///\return A const reference to the last element.
    //*********************************************************************
    const_reference back() const
    {
      return p_buffer[current_size - 1];
    }

    //*********************************************************************
    /// Returns a pointer to the beginning of the vector data.
    ///\return A pointer to the beginning of the vector data.
    //*********************************************************************
    pointer data()
    {
      return p_buffer;
    }

    //*********************************************************************
    /// Returns a const pointer to the beginning of the vector data.
    ///\return A const pointer to the beginning of the vector data.
    //*********************************************************************
    const_pointer data() const
    {
      return p_buffer;
    }

    //*********************************************************************
    /// Assigns values to the vector.
    /// If asserts or exceptions are enabled, emits vector_full if the vector does not have enough free space.
    /// If asserts or exceptions are enabled, emits vector_iterator if the iterators are reversed.
    ///\param first The iterator to the first element.
    ///\param last  The iterator to the last element + 1.
    //*********************************************************************
    template <typename TIterator>
    void assign(TIterator first, TIterator last)
    {
#ifdef _DEBUG
      difference_type count = std::distance(first, last);
      ETL_ASSERT(static_cast<size_t>(count) <= MAX_SIZE, ETL_ERROR(vector_full));
#endif

      initialise();

      // Choose the algorithm according to type.
      // The unused branch should be optimised away.
      if (is_simple_type<value_type>::value)
      {
        while (first != last)
        {
          p_buffer[current_size++] = *first++;
        }
      }
      else
      {
        while (first != last)
        {
          create_element(*first);
          ++first;
        }
      }
    }

    //*********************************************************************
    /// Assigns values to the vector.
    /// If asserts or exceptions are enabled, emits vector_full if the vector does not have enough free space.
    ///\param n     The number of elements to add.
    ///\param value The value to insert for each element.
    //*********************************************************************
    void assign(size_t n, parameter_t value)
    {
      initialise();

      ETL_ASSERT(n <= MAX_SIZE, ETL_ERROR(vector_full));

      // Choose the algorithm according to type.
      // The unused branch should be optimised away.
      if (is_simple_type<value_type>::value)
      {
        while (n > 0)
        {
          p_buffer[current_size++] = value;
          --n;
        }
      }
      else
      {
        while (n > 0)
        {
          create_element(value);
          --n;
        }
      }
    }

    //*************************************************************************
    /// Clears the vector.
    //*************************************************************************
    void clear()
    {
      initialise();
    }

    //*************************************************************************
    /// Increases the size of the vector by one, but does not initialise the new element.
    /// If asserts or exceptions are enabled, throws a vector_full if the vector is already full.
    //*************************************************************************
    void push_back()
    {
#if defined(ETL_CHECK_PUSH_POP)
      ETL_ASSERT(current_size != MAX_SIZE, ETL_ERROR(vector_full));
#endif
      create_element();
    }

    //*********************************************************************
    /// Inserts a value at the end of the vector.
    /// If asserts or exceptions are enabled, emits vector_full if the vector is already full.
    ///\param value The value to add.
    //*********************************************************************
    void push_back(parameter_t value)
    {
#if defined(ETL_CHECK_PUSH_POP)
      ETL_ASSERT(current_size != MAX_SIZE, ETL_ERROR(vector_full));
#endif
      create_element(value);
    }

    //*************************************************************************
    /// Removes an element from the end of the vector.
    /// Does nothing if the vector is empty.
    //*************************************************************************
    void pop_back()
    {
#if defined(ETL_CHECK_PUSH_POP)
      ETL_ASSERT(current_size > 0, ETL_ERROR(vector_empty));
#endif
      destroy_element();
    }

    //*********************************************************************
    /// Inserts a value to the vector.
    /// If asserts or exceptions are enabled, emits vector_full if the vector is already full.
    ///\param position The position to insert before.
    ///\param value    The value to insert.
    //*********************************************************************
    iterator insert(iterator position, parameter_t value)
    {
      ETL_ASSERT((current_size)+1 <= MAX_SIZE, ETL_ERROR(vector_full));

      if (position != end())
      {
        create_element();
        std::copy_backward(position, end() - 1, end());
        *position = value;
      }
      else
      {
        create_element(value);
      }

      return position;
    }

    //*********************************************************************
    /// Inserts 'n' values to the vector.
    /// If asserts or exceptions are enabled, emits vector_full if the vector does not have enough free space.
    ///\param position The position to insert before.
    ///\param n        The number of elements to add.
    ///\param value    The value to insert.
    //*********************************************************************
    void insert(iterator position, size_t n, parameter_t value)
    {
      ETL_ASSERT((current_size)+1 <= MAX_SIZE, ETL_ERROR(vector_full));

      if (position == end())
      {
        while (n > 0)
        {
          create_element(value);
          --n;
        }
      }
      else
      {
        // Create copy (backwards).
        size_t n_insert      = n;
        size_t from          = size() - 1;
        size_t to            = from + n_insert;
        size_t n_move        = std::distance(position, end());
        size_t n_create_copy = std::min(n_insert, n_move);

        for (size_t i = 0; i < n_create_copy; ++i)
        {
          create_element_at(to--, p_buffer[from--]);
        }

        // Copy old.
        size_t insert_index = std::distance(begin(), position);
        from = insert_index;
        to   = from + n_insert;
        size_t n_copy_old = (size() > n_insert) ? size() - n_insert : 0;
        etl::copy_n(&p_buffer[from], n_copy_old, &p_buffer[to]);

        // Copy new.
        to = insert_index;
        
        size_t n_create_new = (n_insert > n_create_copy) ? n_insert - n_create_copy : 0;
        size_t n_copy_new = (n_insert > n_create_new) ? n_insert - n_create_new : 0;
        std::fill_n(&p_buffer[to], n_copy_new, value);

        // Create new.
        to = size();

        for (size_t i = 0; i < n_create_new; ++i)
        {
          create_element_at(to++, value);
        }

        current_size += n_insert;
      }
    }

    //*********************************************************************
    /// Inserts a range of values to the vector.
    /// If asserts or exceptions are enabled, emits vector_full if the vector does not have enough free space.
    /// For fundamental and pointer types.
    ///\param position The position to insert before.
    ///\param first    The first element to add.
    ///\param last     The last + 1 element to add.
    //*********************************************************************
    template <class TIterator>
    typename etl::enable_if<is_simple_type<typename std::iterator_traits<TIterator>::value_type>::value, void>::type
    insert(iterator position, TIterator first, TIterator last)
    {
      size_t count = std::distance(first, last);

      ETL_ASSERT((current_size)+count <= MAX_SIZE, ETL_ERROR(vector_full));

      if (position == end())
      {
        while (first != last)
        {
          p_buffer[current_size++] = *first++;
        }
      }
      else
      {
        size_t insert_index = std::distance(begin(), position);
        size_t n_insert = count;

        // Create copy (backwards).
        size_t from = size() - 1;
        size_t to = from + n_insert;
        size_t n_move = std::distance(position, end());
        size_t n_create_copy = std::min(n_insert, n_move);

        for (size_t i = 0; i < n_create_copy; ++i)
        {
          p_buffer[to--] = p_buffer[from--];
        }

        // Copy old.
        from = insert_index;
        to = from + n_insert;
        size_t n_copy_old = (size() > n_insert) ? size() - n_insert : 0;
        etl::copy_n(&p_buffer[from], n_copy_old, &p_buffer[to]);

        // Copy new.
        to = insert_index;
        size_t n_create_new = (n_insert > n_create_copy) ? n_insert - n_create_copy : 0;
        size_t n_copy_new = (n_insert > n_create_new) ? n_insert - n_create_new : 0;
        etl::copy_n(first, n_copy_new, &p_buffer[to]);
        first += n_copy_new;

        // Create new.
        to = size();
        for (size_t i = 0; i < n_create_new; ++i)
        {
          p_buffer[to++] = *first++;
        }

        current_size += n_insert;
      }
    }
    
    //*********************************************************************
    /// Inserts a range of values to the vector.
    /// If asserts or exceptions are enabled, emits vector_full if the vector does not have enough free space.
    /// For non-fundamental types.
    ///\param position The position to insert before.
    ///\param first    The first element to add.
    ///\param last     The last + 1 element to add.
    //*********************************************************************
    template <class TIterator>
    typename etl::enable_if<!is_simple_type<typename std::iterator_traits<TIterator>::value_type>::value, void>::type
    insert(iterator position, TIterator first, TIterator last)
    {
      size_t count = std::distance(first, last);

      ETL_ASSERT((current_size)+count <= MAX_SIZE, ETL_ERROR(vector_full));

      if (position == end())
      {
        while (first != last)
        {
          create_element(*first);
          ++first;
        }
      }
      else
      {
        size_t insert_index = std::distance(begin(), position);
        size_t n_insert = count;

        // Create copy (backwards).
        size_t from = size() - 1;
        size_t to = from + n_insert;
        size_t n_move = std::distance(position, end());
        size_t n_create_copy = std::min(n_insert, n_move);
        for (size_t i = 0; i < n_create_copy; ++i)
        {
          create_element_at(to--, p_buffer[from--]);
        }

        // Copy old.
        from = insert_index;
        to = from + n_insert;
        size_t n_copy_old = (size() > n_insert) ? size() - n_insert : 0;
        etl::copy_n(&p_buffer[from], n_copy_old, &p_buffer[to]);

        // Copy new.
        to = insert_index;
        size_t n_create_new = (n_insert > n_create_copy) ? n_insert - n_create_copy : 0;
        size_t n_copy_new = (n_insert > n_create_new) ? n_insert - n_create_new : 0;
        etl::copy_n(first, n_copy_new, &p_buffer[to]);
        first += n_copy_new;

        // Create new.
        to = size();
        for (size_t i = 0; i < n_create_new; ++i)
        {
          create_element_at(to++, *first);
          ++first;
        }

        current_size += n_insert;
      }
    }

    //*********************************************************************
    /// Erases an element.
    ///\param i_element Iterator to the element.
    ///\return An iterator pointing to the element that followed the erased element.
    //*********************************************************************
    iterator erase(iterator i_element)
    {
      std::copy(i_element + 1, end(), i_element);
      destroy_element();

      return i_element;
    }

    //*********************************************************************
    /// Erases a range of elements.
    /// The range includes all the elements between first and last, including the
    /// element pointed by first, but not the one pointed by last.
    ///\param first Iterator to the first element.
    ///\param last  Iterator to the last element.
    ///\return An iterator pointing to the element that followed the erased element.
    //*********************************************************************
    iterator erase(iterator first, iterator last)
    {
      std::copy(last, end(), first);
      size_t n_delete = std::distance(first, last);

      if (is_simple_type<value_type>::value)
      {
        // Just adjust the count.
        current_size -= n_delete;
      }
      else
      {
        // Destroy the elements left over at the end.
        while (n_delete-- > 0)
        {
          destroy_element();
        }
      }

      return first;
    }

    //*************************************************************************
    /// Assignment operator.
    //*************************************************************************
    ivector& operator = (const ivector& rhs)
    {
      if (&rhs != this)
      {
        assign(rhs.cbegin(), rhs.cend());
      }

      return *this;
    }

  protected:

    //*********************************************************************
    /// Constructor.
    //*********************************************************************
    ivector(T* p_buffer, size_t MAX_SIZE)
      : vector_base(MAX_SIZE),
        p_buffer(p_buffer)
    {
    }

    //*********************************************************************
    /// Initialise the vector.
    //*********************************************************************
    void initialise()
    {
      // Choose the algorithm according to type.
      // The unused branch should be optimised away.
      if (is_simple_type<value_type>::value)
      {
        current_size = 0;
      }
      else
      {
        while (current_size > 0)
        {
          destroy_element();
        }
      }
    }

    T* p_buffer;

  private:

    //*********************************************************************
    /// Create a new element with a default value at the back.
    //*********************************************************************
    inline void create_element()
    {
      new(&p_buffer[current_size++]) T();
    }

    //*********************************************************************
    /// Create a new element with a value at the back
    //*********************************************************************
    inline void create_element(parameter_t value)
    {
      new(&p_buffer[current_size++]) T(value);
    }

    //*********************************************************************
    /// Create a new element with a value at the index
    //*********************************************************************
    inline void create_element_at(size_t index, parameter_t value)
    {
      new(&p_buffer[index]) T(value);
    }

    //*********************************************************************
    /// Destroy an element at the back.
    //*********************************************************************
    inline void destroy_element()
    {
      p_buffer[--current_size].~T();
    }

    // Disable copy construction.
    ivector(const ivector&);
  };

  //***************************************************************************
  /// Equal operator.
  ///\param lhs Reference to the first vector.
  ///\param rhs Reference to the second vector.
  ///\return <b>true</b> if the arrays are equal, otherwise <b>false</b>
  ///\ingroup vector
  //***************************************************************************
  template <typename T>
  bool operator ==(const etl::ivector<T>& lhs, const etl::ivector<T>& rhs)
  {
    return (lhs.size() == rhs.size()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  //***************************************************************************
  /// Not equal operator.
  ///\param lhs Reference to the first vector.
  ///\param rhs Reference to the second vector.
  ///\return <b>true</b> if the arrays are not equal, otherwise <b>false</b>
  ///\ingroup vector
  //***************************************************************************
  template <typename T>
  bool operator !=(const etl::ivector<T>& lhs, const etl::ivector<T>& rhs)
  {
    return !(lhs == rhs);
  }

  //***************************************************************************
  /// Less than operator.
  ///\param lhs Reference to the first vector.
  ///\param rhs Reference to the second vector.
  ///\return <b>true</b> if the first vector is lexicographically less than the second, otherwise <b>false</b>
  ///\ingroup vector
  //***************************************************************************
  template <typename T>
  bool operator <(const etl::ivector<T>& lhs, const etl::ivector<T>& rhs)
  {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  }

  //***************************************************************************
  /// Greater than operator.
  ///\param lhs Reference to the first vector.
  ///\param rhs Reference to the second vector.
  ///\return <b>true</b> if the first vector is lexicographically greater than the second, otherwise <b>false</b>
  ///\ingroup vector
  //***************************************************************************
  template <typename T>
  bool operator >(const etl::ivector<T>& lhs, const etl::ivector<T>& rhs)
  {
    return (rhs < lhs);
  }

  //***************************************************************************
  /// Less than or equal operator.
  ///\param lhs Reference to the first vector.
  ///\param rhs Reference to the second vector.
  ///\return <b>true</b> if the first vector is lexicographically less than or equal to the second, otherwise <b>false</b>
  ///\ingroup vector
  //***************************************************************************
  template <typename T>
  bool operator <=(const etl::ivector<T>& lhs, const etl::ivector<T>& rhs)
  {
    return !(lhs > rhs);
  }

  //***************************************************************************
  /// Greater than or equal operator.
  ///\param lhs Reference to the first vector.
  ///\param rhs Reference to the second vector.
  ///\return <b>true</b> if the first vector is lexicographically greater than or equal to the second, otherwise <b>false</b>
  ///\ingroup vector
  //***************************************************************************
  template <typename T>
  bool operator >=(const etl::ivector<T>& lhs, const etl::ivector<T>& rhs)
  {
    return !(lhs < rhs);
  }
}

#include "private/ivectorpointer.h"

#undef __ETL_IN_IVECTOR_H__
#endif
