///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
http://www.etlcpp.com

Copyright(c) 2015 jwellbelove

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

#ifndef __ETL_IFLAT_MAP__
#define __ETL_IFLAT_MAP__
#define __ETL_IN_IFLAT_MAP_H__

#include <iterator>
#include <algorithm>
#include <functional>
#include <utility>
#include <stddef.h>

#include "private/flat_map_base.h"
#include "type_traits.h"
#include "parameter_type.h"
#include "ivector.h"
#include "error_handler.h"

namespace etl
{
  //***************************************************************************
  /// The base class for specifically sized flat_maps.
  /// Can be used as a reference type for all flat_maps containing a specific type.
  ///\ingroup flat_map
  //***************************************************************************
  template <typename TKey, typename TMapped, typename TKeyCompare = std::less<TKey> >
  class iflat_map : public flat_map_base
  {
  public:

    typedef std::pair<TKey, TMapped> value_type;

  private:

    typedef etl::ivector<value_type> buffer_t;

  public:

    typedef TKey                                      key_type;
    typedef TMapped                                   mapped_type;
    typedef TKeyCompare                               key_compare;
    typedef value_type&                               reference;
    typedef const value_type&                         const_reference;
    typedef value_type*                               pointer;
    typedef const value_type*                         const_pointer;
    typedef typename buffer_t::iterator               iterator;
    typedef typename buffer_t::const_iterator         const_iterator;
    typedef typename buffer_t::reverse_iterator       reverse_iterator;
    typedef typename buffer_t::const_reverse_iterator const_reverse_iterator;
    typedef size_t                                    size_type;
    typedef typename std::iterator_traits<iterator>::difference_type difference_type;

  protected:

    typedef typename parameter_type<TKey>::type key_value_parameter_t;

  private:

    //*********************************************************************
    /// How to compare elements and keys.
    //*********************************************************************
    class compare
    {
    public:

      bool operator ()(const value_type& element, key_type key) const
      {
        return key_compare()(element.first, key);
      }

      bool operator ()(key_type key, const value_type& element) const
      {
        return key_compare()(key, element.first);
      }
    };

  public:

    //*********************************************************************
    /// Returns an iterator to the beginning of the flat_map.
    ///\return An iterator to the beginning of the flat_map.
    //*********************************************************************
    iterator begin()
    {
      return buffer.begin();
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the flat_map.
    ///\return A const iterator to the beginning of the flat_map.
    //*********************************************************************
    const_iterator begin() const
    {
      return buffer.begin();
    }

    //*********************************************************************
    /// Returns an iterator to the end of the flat_map.
    ///\return An iterator to the end of the flat_map.
    //*********************************************************************
    iterator end()
    {
      return buffer.end();
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the flat_map.
    ///\return A const iterator to the end of the flat_map.
    //*********************************************************************
    const_iterator end() const
    {
      return buffer.end();
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the flat_map.
    ///\return A const iterator to the beginning of the flat_map.
    //*********************************************************************
    const_iterator cbegin() const
    {
      return buffer.cbegin();
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the flat_map.
    ///\return A const iterator to the end of the flat_map.
    //*********************************************************************
    const_iterator cend() const
    {
      return buffer.cend();
    }

    //*********************************************************************
    /// Returns an reverse iterator to the reverse beginning of the flat_map.
    ///\return Iterator to the reverse beginning of the flat_map.
    //*********************************************************************
    reverse_iterator rbegin()
    {
	    return buffer.rbegin();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the flat_map.
    ///\return Const iterator to the reverse beginning of the flat_map.
    //*********************************************************************
    const_reverse_iterator rbegin() const
    {
      return buffer.rbegin();
    }

    //*********************************************************************
    /// Returns a reverse iterator to the end + 1 of the flat_map.
    ///\return Reverse iterator to the end + 1 of the flat_map.
    //*********************************************************************
    reverse_iterator rend()
    {
      return buffer.rend();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the end + 1 of the flat_map.
    ///\return Const reverse iterator to the end + 1 of the flat_map.
    //*********************************************************************
    const_reverse_iterator rend() const
    {
      return buffer.rend();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the flat_map.
    ///\return Const reverse iterator to the reverse beginning of the flat_map.
    //*********************************************************************
    const_reverse_iterator crbegin() const
    {
      return buffer.crbegin();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the end + 1 of the flat_map.
    ///\return Const reverse iterator to the end + 1 of the flat_map.
    //*********************************************************************
    const_reverse_iterator crend() const
    {
      return buffer.crend();
    }

    //*********************************************************************
    /// Returns a reference to the value at index 'key'
    ///\param i The index.
    ///\return A reference to the value at index 'key'
    //*********************************************************************
    mapped_type& operator [](key_value_parameter_t key)
    {
      iterator i_element = lower_bound(key);

      if (i_element == end())
      {
        // Doesn't exist, so create a new one.
	      value_type value(key, mapped_type());
        i_element = insert(value).first;
      }

      return i_element->second;
    }

    //*********************************************************************
    /// Returns a reference to the value at index 'key'
    /// If asserts or exceptions are enabled, emits an etl::flat_map_out_of_bounds if the key is not in the range.
    ///\param i The index.
    ///\return A reference to the value at index 'key'
    //*********************************************************************
    mapped_type& at(key_value_parameter_t key)
    {
      iterator i_element = lower_bound(key);

      ETL_ASSERT(i_element != end(), ETL_ERROR(flat_map_out_of_bounds));

      return i_element->second;
    }

    //*********************************************************************
    /// Returns a const reference to the value at index 'key'
    /// If asserts or exceptions are enabled, emits an etl::flat_map_out_of_bounds if the key is not in the range.
    ///\param i The index.
    ///\return A const reference to the value at index 'key'
    //*********************************************************************
    const mapped_type& at(key_value_parameter_t key) const
    {
      typename buffer_t::const_iterator i_element = lower_bound(key);

      ETL_ASSERT(i_element != end(), ETL_ERROR(flat_map_out_of_bounds));

      return i_element->second;
    }

    //*********************************************************************
    /// Assigns values to the flat_map.
    /// If ETL_THROW_EXCEPTIONS & _DEBUG are defined, emits flat_map_full if the flat_map does not have enough free space.
    /// If ETL_THROW_EXCEPTIONS & _DEBUG are defined, emits flat_map_iterator if the iterators are reversed.
    ///\param first The iterator to the first element.
    ///\param last  The iterator to the last element + 1.
    //*********************************************************************
    template <typename TIterator>
    void assign(TIterator first, TIterator last)
    {
#ifdef _DEBUG
      difference_type count = std::distance(first, last);
      ETL_ASSERT(count <= difference_type(capacity()), ETL_ERROR(flat_map_full));
#endif

      clear();

      while (first != last)
      {
        insert(*first++);
      }
    }

    //*********************************************************************
    /// Inserts a value to the flat_map.
    /// If asserts or exceptions are enabled, emits flat_map_full if the flat_map is already full.
    ///\param value    The value to insert.
    //*********************************************************************
    std::pair<iterator, bool> insert(const value_type& value)
    {
      std::pair<iterator, bool> result(end(), false);

      iterator i_element = lower_bound(value.first);

      if (i_element == end())
      {
        // At the end.
        ETL_ASSERT(!buffer.full(), ETL_ERROR(flat_map_full));
        buffer.push_back(value);
        result.first  = end() - 1;
        result.second = true;
      }
      else
      {
        // Not at the end.
        // Existing element?
        if (value.first != i_element->first)
        {
          // A new one.
          ETL_ASSERT(!buffer.full(), ETL_ERROR(flat_map_full));
          buffer.insert(i_element, value);
          result.first  = i_element;
          result.second = true;
        }
      }

      return result;
    }

    //*********************************************************************
    /// Inserts a value to the flat_map.
    /// If asserts or exceptions are enabled, emits flat_map_full if the flat_map is already full.
    ///\param position The position to insert at.
    ///\param value    The value to insert.
    //*********************************************************************
    iterator insert(iterator position, const value_type& value)
    {
      return insert(value).first;
    }

    //*********************************************************************
    /// Inserts a range of values to the flat_map.
    /// If asserts or exceptions are enabled, emits flat_map_full if the flat_map does not have enough free space.
    ///\param position The position to insert at.
    ///\param first    The first element to add.
    ///\param last     The last + 1 element to add.
    //*********************************************************************
    template <class TIterator>
    void insert(TIterator first, TIterator last)
    {
      while (first != last)
      {
        insert(*first++);
      }
    }

    //*********************************************************************
    /// Erases an element.
    ///\param key The key to erase.
    ///\return The number of elements erased. 0 or 1.
    //*********************************************************************
    size_t erase(key_value_parameter_t key)
    {
      iterator i_element = find(key);

      if (i_element == end())
      {
        return 0;
      }
      else
      {
        buffer.erase(i_element);
        return 1;
      }
    }

    //*********************************************************************
    /// Erases an element.
    ///\param i_element Iterator to the element.
    //*********************************************************************
    void erase(iterator i_element)
    {
      buffer.erase(i_element);
    }

    //*********************************************************************
    /// Erases a range of elements.
    /// The range includes all the elements between first and last, including the
    /// element pointed by first, but not the one pointed by last.
    ///\param first Iterator to the first element.
    ///\param last  Iterator to the last element.
    //*********************************************************************
    void erase(iterator first, iterator last)
    {
      buffer.erase(first, last);
    }

    //*************************************************************************
    /// Clears the flat_map.
    //*************************************************************************
    void clear()
    {
      buffer.clear();
    }

    //*********************************************************************
    /// Finds an element.
    ///\param key The key to search for.
    ///\return An iterator pointing to the element or end() if not found.
    //*********************************************************************
    iterator find(key_value_parameter_t key)
    {
      iterator itr = lower_bound(key);

      if (itr != end())
      {
        if (!key_compare()(itr->first, key) && !key_compare()(key, itr->first))
        {
          return itr;
        }
        else
        {
          return end();
        }
      }

      return end();
    }

    //*********************************************************************
    /// Finds an element.
    ///\param key The key to search for.
    ///\return An iterator pointing to the element or end() if not found.
    //*********************************************************************
    const_iterator find(key_value_parameter_t key) const
    {
      const_iterator itr = lower_bound(key);

      if (itr != end())
      {
        if (!key_compare()(itr->first, key) && !key_compare()(key, itr->first))
        {
          return itr;
        }
        else
        {
          return end();
        }
      }

      return end();
    }

    //*********************************************************************
    /// Counts an element.
    ///\param key The key to search for.
    ///\return 1 if the key exists, otherwise 0.
    //*********************************************************************
    size_t count(key_value_parameter_t key) const
    {
      return (find(key) == end()) ? 0 : 1;
    }

    //*********************************************************************
    /// Finds the lower bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    iterator lower_bound(key_value_parameter_t key)
    {
      return std::lower_bound(begin(), end(), key, compare());
    }

    //*********************************************************************
    /// Finds the lower bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    const_iterator lower_bound(key_value_parameter_t key) const
    {
      return std::lower_bound(cbegin(), cend(), key, compare());
    }

    //*********************************************************************
    /// Finds the upper bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    iterator upper_bound(key_value_parameter_t key)
    {
      return std::upper_bound(begin(), end(), key, compare());
    }

    //*********************************************************************
    /// Finds the upper bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    const_iterator upper_bound(key_value_parameter_t key) const
    {
      return std::upper_bound(begin(), end(), key, compare());
    }

    //*********************************************************************
    /// Finds the range of equal elements of a key
    ///\param key The key to search for.
    ///\return An iterator pair.
    //*********************************************************************
    std::pair<iterator, iterator> equal_range(key_value_parameter_t key)
    {
      iterator i_lower = std::lower_bound(begin(), end(), key, compare());

      return std::make_pair(i_lower, std::upper_bound(i_lower, end(), key, compare()));
    }

    //*********************************************************************
    /// Finds the range of equal elements of a key
    ///\param key The key to search for.
    ///\return An iterator pair.
    //*********************************************************************
    std::pair<const_iterator, const_iterator> equal_range(key_value_parameter_t key) const
    {
      const_iterator i_lower = std::lower_bound(cbegin(), cend(), key, compare());

      return std::make_pair(i_lower, std::upper_bound(i_lower, cend(), key, compare()));
    }

    //*************************************************************************
    /// Assignment operator.
    //*************************************************************************
    iflat_map& operator = (const iflat_map& rhs)
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
    iflat_map(buffer_t& buffer)
      : flat_map_base(buffer),
        buffer(buffer)
    {
    }

  private:

    // Disable copy construction.
    iflat_map(const iflat_map&);

    buffer_t& buffer;
  };

  //***************************************************************************
  /// Equal operator.
  ///\param lhs Reference to the first flat_map.
  ///\param rhs Reference to the second flat_map.
  ///\return <b>true</b> if the arrays are equal, otherwise <b>false</b>
  ///\ingroup flat_map
  //***************************************************************************
  template <typename TKey, typename TMapped, typename TKeyCompare>
  bool operator ==(const etl::iflat_map<TKey, TMapped, TKeyCompare>& lhs, const etl::iflat_map<TKey, TMapped, TKeyCompare>& rhs)
  {
    return (lhs.size() == rhs.size()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  //***************************************************************************
  /// Not equal operator.
  ///\param lhs Reference to the first flat_map.
  ///\param rhs Reference to the second flat_map.
  ///\return <b>true</b> if the arrays are not equal, otherwise <b>false</b>
  ///\ingroup flat_map
  //***************************************************************************
  template <typename TKey, typename TMapped, typename TKeyCompare>
  bool operator !=(const etl::iflat_map<TKey, TMapped, TKeyCompare>& lhs, const etl::iflat_map<TKey, TMapped, TKeyCompare>& rhs)
  {
    return !(lhs == rhs);
  }
}

#undef __ETL_IN_IFLAT_MAP_H__
#endif
