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

#ifndef __ETL_IFLAT_MULTISET__
#define __ETL_IFLAT_MULTISET__
#define __ETL_IN_IFLAT_MULTISET_H__

#include <iterator>
#include <algorithm>
#include <functional>
#include <utility>
#include <stddef.h>

#include "private/flat_multiset_base.h"
#include "type_traits.h"
#include "parameter_type.h"
#include "ivector.h"
#include "error_handler.h"

namespace etl
{
  //***************************************************************************
  /// The base class for specifically sized flat_multisets.
  /// Can be used as a reference type for all flat_multisets containing a specific type.
  ///\ingroup flat_multiset
  //***************************************************************************
  template <typename T, typename TKeyCompare = std::less<T> >
  class iflat_multiset : public flat_multiset_base
  {
  private:

    typedef etl::ivector<T> buffer_t;

  public:

    typedef T                                         key_type;
    typedef T                                         value_type;
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

    typedef typename parameter_type<T>::type parameter_t;

  public:

    //*********************************************************************
    /// Returns an iterator to the beginning of the flat_multiset.
    ///\return An iterator to the beginning of the flat_multiset.
    //*********************************************************************
    iterator begin()
    {
      return buffer.begin();
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the flat_multiset.
    ///\return A const iterator to the beginning of the flat_multiset.
    //*********************************************************************
    const_iterator begin() const
    {
      return buffer.begin();
    }

    //*********************************************************************
    /// Returns an iterator to the end of the flat_multiset.
    ///\return An iterator to the end of the flat_multiset.
    //*********************************************************************
    iterator end()
    {
      return buffer.end();
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the flat_multiset.
    ///\return A const iterator to the end of the flat_multiset.
    //*********************************************************************
    const_iterator end() const
    {
      return buffer.end();
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the flat_multiset.
    ///\return A const iterator to the beginning of the flat_multiset.
    //*********************************************************************
    const_iterator cbegin() const
    {
      return buffer.cbegin();
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the flat_multiset.
    ///\return A const iterator to the end of the flat_multiset.
    //*********************************************************************
    const_iterator cend() const
    {
      return buffer.cend();
    }

    //*********************************************************************
    /// Returns an reverse iterator to the reverse beginning of the flat_multiset.
    ///\return Iterator to the reverse beginning of the flat_multiset.
    //*********************************************************************
    reverse_iterator rbegin()
    {
        return buffer.rbegin();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the flat_multiset.
    ///\return Const iterator to the reverse beginning of the flat_multiset.
    //*********************************************************************
    const_reverse_iterator rbegin() const
    {
      return buffer.rbegin();
    }

    //*********************************************************************
    /// Returns a reverse iterator to the end + 1 of the flat_multiset.
    ///\return Reverse iterator to the end + 1 of the flat_multiset.
    //*********************************************************************
    reverse_iterator rend()
    {
      return buffer.rend();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the end + 1 of the flat_multiset.
    ///\return Const reverse iterator to the end + 1 of the flat_multiset.
    //*********************************************************************
    const_reverse_iterator rend() const
    {
      return buffer.rend();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the flat_multiset.
    ///\return Const reverse iterator to the reverse beginning of the flat_multiset.
    //*********************************************************************
    const_reverse_iterator crbegin() const
    {
      return buffer.crbegin();
    }

    //*********************************************************************
    /// Returns a const reverse iterator to the end + 1 of the flat_multiset.
    ///\return Const reverse iterator to the end + 1 of the flat_multiset.
    //*********************************************************************
    const_reverse_iterator crend() const
    {
      return buffer.crend();
    }

    //*********************************************************************
    /// Assigns values to the flat_multiset.
    /// If asserts or exceptions are enabled, emits flat_multiset_full if the flat_multiset does not have enough free space.
    /// If asserts or exceptions are enabled, emits flat_multiset_iterator if the iterators are reversed.
    ///\param first The iterator to the first element.
    ///\param last  The iterator to the last element + 1.
    //*********************************************************************
    template <typename TIterator>
    void assign(TIterator first, TIterator last)
    {
#ifdef _DEBUG
      difference_type count = std::distance(first, last);
      ETL_ASSERT(count <= difference_type(capacity()), ETL_ERROR(flat_multiset_full));
#endif

      clear();

      while (first != last)
      {
        insert(*first++);
      }
    }

    //*********************************************************************
    /// Inserts a value to the flat_multiset.
    /// If asserts or exceptions are enabled, emits flat_multiset_full if the flat_multiset is already full.
    ///\param value    The value to insert.
    //*********************************************************************
    std::pair<iterator, bool> insert(parameter_t value)
    {
      std::pair<iterator, bool> result(end(), false);

      ETL_ASSERT(!buffer.full(), ETL_ERROR(flat_multiset_full));

      iterator i_element = std::lower_bound(begin(), end(), value, TKeyCompare());

      if (i_element == end())
      {
        // At the end.
        buffer.push_back(value);
        result.first = end() - 1;
        result.second = true;
      }
      else
      {
        // Not at the end.
        buffer.insert(i_element, value);
        result.first = i_element;
        result.second = true;
      }
      
      return result;
    }

    //*********************************************************************
    /// Inserts a value to the flat_multiset.
    /// If asserts or exceptions are enabled, emits flat_multiset_full if the flat_multiset is already full.
    ///\param position The position to insert at.
    ///\param value    The value to insert.
    //*********************************************************************
    iterator insert(iterator position, parameter_t value)
    {
      return insert(value).first;
    }

    //*********************************************************************
    /// Inserts a range of values to the flat_multiset.
    /// If asserts or exceptions are enabled, emits flat_multiset_full if the flat_multiset does not have enough free space.
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
    size_t erase(parameter_t key)
    {
	  std::pair<iterator, iterator> range = equal_range(key);

      if (range.first == end())
      {
        return 0;
      }
      else
      {
		size_t count = std::distance(range.first, range.second);
		erase(range.first, range.second);
		return count;
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
    /// Clears the flat_multiset.
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
    iterator find(parameter_t key)
    {
      iterator itr = std::lower_bound(begin(), end(), key, TKeyCompare());

      if (itr != end())
      {
        if (!key_compare()(*itr, key) && !key_compare()(key, *itr))
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
    const_iterator find(parameter_t key) const
    {
      const_iterator itr = std::lower_bound(begin(), end(), key, TKeyCompare());

      if (itr != end())
      {
        if (!key_compare()(*itr, key) && !key_compare()(key, *itr))
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
    size_t count(parameter_t key) const
    {
      std::pair<const_iterator, const_iterator> range = equal_range(key);

  	  return std::distance(range.first, range.second);
    }

    //*********************************************************************
    /// Finds the lower bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    iterator lower_bound(parameter_t key)
    {
      return std::lower_bound(begin(), end(), key, TKeyCompare());
    }

    //*********************************************************************
    /// Finds the lower bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    const_iterator lower_bound(parameter_t key) const
    {
      return std::lower_bound(cbegin(), cend(), key, TKeyCompare());
    }
    
    //*********************************************************************
    /// Finds the upper bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    iterator upper_bound(parameter_t key)
    {
      return std::upper_bound(begin(), end(), key, TKeyCompare());
    }

    //*********************************************************************
    /// Finds the upper bound of a key
    ///\param key The key to search for.
    ///\return An iterator.
    //*********************************************************************
    const_iterator upper_bound(parameter_t key) const
    {
      return std::upper_bound(cbegin(), cend(), key, TKeyCompare());
    }

    //*********************************************************************
    /// Finds the range of equal elements of a key
    ///\param key The key to search for.
    ///\return An iterator pair.
    //*********************************************************************
    std::pair<iterator, iterator> equal_range(parameter_t key)
    {
      return std::equal_range(begin(), end(), key, TKeyCompare());
    }

    //*********************************************************************
    /// Finds the range of equal elements of a key
    ///\param key The key to search for.
    ///\return An iterator pair.
    //*********************************************************************
    std::pair<const_iterator, const_iterator> equal_range(parameter_t key) const
    {
      return std::equal_range(cbegin(), cend(), key, TKeyCompare());
    }

    //*************************************************************************
    /// Assignment operator.
    //*************************************************************************
    iflat_multiset& operator = (const iflat_multiset& rhs)
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
    iflat_multiset(buffer_t& buffer)
      : flat_multiset_base(buffer),
        buffer(buffer)
    {
    }

  private:

    // Disable copy construction.
    iflat_multiset(const iflat_multiset&);

    buffer_t& buffer;
  };

  //***************************************************************************
  /// Equal operator.
  ///\param lhs Reference to the first flat_multiset.
  ///\param rhs Reference to the second flat_multiset.
  ///\return <b>true</b> if the arrays are equal, otherwise <b>false</b>
  ///\ingroup flat_multiset
  //***************************************************************************
  template <typename T, typename TKeyCompare>
  bool operator ==(const etl::iflat_multiset<T, TKeyCompare>& lhs, const etl::iflat_multiset<T, TKeyCompare>& rhs)
  {
    return (lhs.size() == rhs.size()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  //***************************************************************************
  /// Not equal operator.
  ///\param lhs Reference to the first flat_multiset.
  ///\param rhs Reference to the second flat_multiset.
  ///\return <b>true</b> if the arrays are not equal, otherwise <b>false</b>
  ///\ingroup flat_multiset
  //***************************************************************************
  template <typename T, typename TKeyCompare>
  bool operator !=(const etl::iflat_multiset<T, TKeyCompare>& lhs, const etl::iflat_multiset<T, TKeyCompare>& rhs)
  {
    return !(lhs == rhs);
  }
}

#undef __ETL_IN_IFLAT_MULTISET_H__
#endif
