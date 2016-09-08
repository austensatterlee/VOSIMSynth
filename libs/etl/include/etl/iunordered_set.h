///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
http://www.etlcpp.com

Copyright(c) 2016 jwellbelove

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

#ifndef __ETL_IUNORDERED_SET__
#define __ETL_IUNORDERED_SET__

#include <iterator>
#include <algorithm>
#include <functional>
#include <utility>
#include <stddef.h>

#include "type_traits.h"
#include "parameter_type.h"
#include "hash.h"
#include "nullptr.h"
#include "ipool.h"
#include "ivector.h"
#include "error_handler.h"
#include "intrusive_forward_list.h"
#include "exception.h"
#include "error_handler.h"

#undef ETL_FILE
#define ETL_FILE "23"

namespace etl
{
  //***************************************************************************
  /// Exception for the unordered_set.
  ///\ingroup unordered_set
  //***************************************************************************
  class unordered_set_exception : public exception
  {
  public:

    unordered_set_exception(string_type what, string_type file_name, numeric_type line_number)
      : exception(what, file_name, line_number)
    {
    }
  };

  //***************************************************************************
  /// Full exception for the unordered_set.
  ///\ingroup unordered_set
  //***************************************************************************
  class unordered_set_full : public unordered_set_exception
  {
  public:

    unordered_set_full(string_type file_name, numeric_type line_number)
      : unordered_set_exception(ETL_ERROR_TEXT("unordered_set:full", ETL_FILE"A"), file_name, line_number)
    {
    }
  };

  //***************************************************************************
  /// Out of range exception for the unordered_set.
  ///\ingroup unordered_set
  //***************************************************************************
  class unordered_set_out_of_range : public unordered_set_exception
  {
  public:

    unordered_set_out_of_range(string_type file_name, numeric_type line_number)
      : unordered_set_exception(ETL_ERROR_TEXT("unordered_set:range", ETL_FILE"B"), file_name, line_number)
    {}
  };

  //***************************************************************************
  /// Iterator exception for the unordered_set.
  ///\ingroup unordered_set
  //***************************************************************************
  class unordered_set_iterator : public unordered_set_exception
  {
  public:

    unordered_set_iterator(string_type file_name, numeric_type line_number)
      : unordered_set_exception(ETL_ERROR_TEXT("unordered_set:iterator", ETL_FILE"C"), file_name, line_number)
    {
    }
  };

  //***************************************************************************
  /// The base class for specifically sized unordered_set.
  /// Can be used as a reference type for all unordered_set containing a specific type.
  ///\ingroup unordered_set
  //***************************************************************************
  template <typename TKey, typename THash = etl::hash<TKey>, typename TKeyEqual = std::equal_to<TKey> >
  class iunordered_set
  {
  public:

    typedef TKey              value_type;
    typedef TKey              key_type;
    typedef THash             hasher;
    typedef TKeyEqual         key_equal;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef size_t            size_type;

    typedef typename parameter_type<TKey>::type key_value_parameter_t;

    typedef etl::forward_link<0> link_t;

    // The nodes that store the elements.
    struct node_t : public link_t
    {
      node_t(const value_type& key)
        : key(key)
      {
      }

      value_type key;
    };

  private:

    typedef etl::intrusive_forward_list<node_t, link_t> bucket_t;
    typedef etl::ipool<node_t>                          pool_t;
    typedef etl::ivector<bucket_t>                      bucket_list_t;

    typedef typename bucket_list_t::iterator  bucket_list_iterator;

  public:

    // Local iterators iterate over one bucket.
    typedef typename bucket_t::iterator       local_iterator;
    typedef typename bucket_t::const_iterator local_const_iterator;

    //*********************************************************************
    class iterator : public std::iterator<std::forward_iterator_tag, TKey>
    {
    public:

      typedef typename iunordered_set::value_type      value_type;
      typedef typename iunordered_set::key_type        key_type;
      typedef typename iunordered_set::hasher          hasher;
      typedef typename iunordered_set::key_equal       key_equal;
      typedef typename iunordered_set::reference       reference;
      typedef typename iunordered_set::const_reference const_reference;
      typedef typename iunordered_set::pointer         pointer;
      typedef typename iunordered_set::const_pointer   const_pointer;
      typedef typename iunordered_set::size_type       size_type;

      friend class iunordered_set;

      //*********************************
      iterator()
      {
      }

      //*********************************
      iterator(const iterator& other)
        : ibuckets_end(other.ibuckets_end),
          ibucket(other.ibucket),
          inode(other.inode)
      {
      }

      //*********************************
      iterator& operator ++()
      {
        ++inode;

        // The end of this node list?
        if (inode == ibucket->end())
        {
          // Search for the next non-empty bucket.
          ++ibucket;
          while ((ibucket != ibuckets_end) && (ibucket->empty()))
          {
            ++ibucket;
          }

          // If not past the end, get the first node in the bucket.
          if (ibucket != ibuckets_end)
          {
            inode = ibucket->begin();
          }
        }

        return *this;
      }

      //*********************************
      iterator operator ++(int)
      {
        iterator temp(*this);
        operator++();
        return temp;
      }

      //*********************************
      iterator operator =(const iterator& other)
      {
        ibuckets_end = other.ibuckets_end;
        ibucket      = other.ibucket;
        inode        = other.inode;
        return *this;
      }

      //*********************************
      reference operator *()
      {
        return inode->key;
      }

      //*********************************
      const_reference operator *() const
      {
        return inode->key;
      }

      //*********************************
      pointer operator &()
      {
        return &(inode->key);
      }

      //*********************************
      const_pointer operator &() const
      {
        return &(inode->key);
      }

      //*********************************
      pointer operator ->()
      {
        return &(inode->key);
      }

      //*********************************
      const_pointer operator ->() const
      {
        return &(inode->key);
      }

      //*********************************
      friend bool operator == (const iterator& lhs, const iterator& rhs)
      {
        return lhs.compare(rhs);
      }

      //*********************************
      friend bool operator != (const iterator& lhs, const iterator& rhs)
      {
        return !(lhs == rhs);
      }

    private:

      //*********************************
      iterator(bucket_list_iterator ibuckets_end, bucket_list_iterator ibucket, local_iterator inode)
        : ibuckets_end(ibuckets_end),
          ibucket(ibucket),
          inode(inode)
      {
      }

      //*********************************
      bool compare(const iterator& rhs) const
      {
        return rhs.inode == inode;
      }

      //*********************************
      bucket_t& get_bucket()
      {
        return *ibucket;
      }

      //*********************************
      bucket_list_iterator& get_bucket_list_iterator()
      {
        return ibucket;
      }

      //*********************************
      local_iterator get_local_iterator()
      {
        return inode;
      }

      bucket_list_iterator ibuckets_end;
      bucket_list_iterator ibucket;
      local_iterator       inode;
    };

    //*********************************************************************
    class const_iterator : public std::iterator<std::forward_iterator_tag, const TKey>
    {
    public:

      typedef typename iunordered_set::value_type      value_type;
      typedef typename iunordered_set::key_type        key_type;
      typedef typename iunordered_set::hasher          hasher;
      typedef typename iunordered_set::key_equal       key_equal;
      typedef typename iunordered_set::reference       reference;
      typedef typename iunordered_set::const_reference const_reference;
      typedef typename iunordered_set::pointer         pointer;
      typedef typename iunordered_set::const_pointer   const_pointer;
      typedef typename iunordered_set::size_type       size_type;

      friend class iunordered_set;
      friend class iterator;

      //*********************************
      const_iterator()
      {
      }

      //*********************************
      const_iterator(const typename iunordered_set::iterator& other)
        : ibuckets_end(other.ibuckets_end),
          ibucket(other.ibucket),
          inode(other.inode)
      {
      }

      //*********************************
      const_iterator(const const_iterator& other)
        : ibuckets_end(other.ibuckets_end),
          ibucket(other.ibucket),
          inode(other.inode)
      {
      }

      //*********************************
      const_iterator& operator ++()
      {
        ++inode;

        // The end of this node list?
        if (inode == ibucket->end())
        {
          // Search for the next non-empty bucket.

          ++ibucket;
          while ((ibucket != ibuckets_end) && (ibucket->empty()))
          {
            ++ibucket;
          }

          // If not past the end, get the first node in the bucket.
          if (ibucket != ibuckets_end)
          {
            inode = ibucket->begin();
          }
        }

        return *this;
      }

      //*********************************
      const_iterator operator ++(int)
      {
        const_iterator temp(*this);
        operator++();
        return temp;
      }

      //*********************************
      const_iterator operator =(const const_iterator& other)
      {
        ibuckets_end = other.ibuckets_end;
        ibucket      = other.ibucket;
        inode        = other.inode;
        return *this;
      }

      //*********************************
      const_reference operator *() const
      {
        return inode->key;
      }

      //*********************************
      const_pointer operator &() const
      {
        return &(inode->key);
      }

      //*********************************
      const_pointer operator ->() const
      {
        return &(inode->key);
      }

      //*********************************
      friend bool operator == (const const_iterator& lhs, const const_iterator& rhs)
      {
        return lhs.compare(rhs);
      }

      //*********************************
      friend bool operator != (const const_iterator& lhs, const const_iterator& rhs)
      {
        return !(lhs == rhs);
      }

    private:

      //*********************************
      const_iterator(bucket_list_iterator ibuckets_end, bucket_list_iterator ibucket, local_iterator inode)
        : ibuckets_end(ibuckets_end),
          ibucket(ibucket),
          inode(inode)
      {
      }

      //*********************************
      bool compare(const const_iterator& rhs) const
      {
        return rhs.inode == inode;
      }

      //*********************************
      bucket_t& get_bucket()
      {
        return *ibucket;
      }

      //*********************************
      bucket_list_iterator& get_bucket_list_iterator()
      {
        return ibucket;
      }

      //*********************************
      local_iterator get_local_iterator()
      {
        return inode;
      }

      bucket_list_iterator ibuckets_end;
      bucket_list_iterator ibucket;
      local_iterator       inode;
    };

    typedef typename std::iterator_traits<iterator>::difference_type difference_type;

    //*********************************************************************
    /// Returns an iterator to the beginning of the unordered_set.
    ///\return An iterator to the beginning of the unordered_set.
    //*********************************************************************
    iterator begin()
    {
      return iterator(pbuckets->end(), first, first->begin());
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the unordered_set.
    ///\return A const iterator to the beginning of the unordered_set.
    //*********************************************************************
    const_iterator begin() const
    {
      return const_iterator(pbuckets->end(), first, first->begin());
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the unordered_set.
    ///\return A const iterator to the beginning of the unordered_set.
    //*********************************************************************
    const_iterator cbegin() const
    {
      return const_iterator(pbuckets->end(), first, first->begin());
    }

    //*********************************************************************
    /// Returns an iterator to the beginning of the unordered_set bucket.
    ///\return An iterator to the beginning of the unordered_set bucket.
    //*********************************************************************
    local_iterator begin(size_t i)
    {
      return (*pbuckets)[i].begin();
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the unordered_set bucket.
    ///\return A const iterator to the beginning of the unordered_set bucket.
    //*********************************************************************
    local_const_iterator begin(size_t i) const
    {
      return (*pbuckets)[i].cbegin();
    }

    //*********************************************************************
    /// Returns a const_iterator to the beginning of the unordered_set bucket.
    ///\return A const iterator to the beginning of the unordered_set bucket.
    //*********************************************************************
    local_const_iterator cbegin(size_t i) const
    {
      return (*pbuckets)[i].cbegin();
    }

    //*********************************************************************
    /// Returns an iterator to the end of the unordered_set.
    ///\return An iterator to the end of the unordered_set.
    //*********************************************************************
    iterator end()
    {
      return iterator(pbuckets->end(), last, last->end());
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the unordered_set.
    ///\return A const iterator to the end of the unordered_set.
    //*********************************************************************
    const_iterator end() const
    {
      return const_iterator(pbuckets->end(), last, last->end());
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the unordered_set.
    ///\return A const iterator to the end of the unordered_set.
    //*********************************************************************
    const_iterator cend() const
    {
      return const_iterator(pbuckets->end(), last, last->end());
    }

    //*********************************************************************
    /// Returns an iterator to the end of the unordered_set bucket.
    ///\return An iterator to the end of the unordered_set bucket.
    //*********************************************************************
    local_iterator end(size_t i)
    {
      return (*pbuckets)[i].end();
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the unordered_set bucket.
    ///\return A const iterator to the end of the unordered_set bucket.
    //*********************************************************************
    local_const_iterator end(size_t i) const
    {
      return (*pbuckets)[i].cend();
    }

    //*********************************************************************
    /// Returns a const_iterator to the end of the unordered_set bucket.
    ///\return A const iterator to the end of the unordered_set bucket.
    //*********************************************************************
    local_const_iterator cend(size_t i) const
    {
      return (*pbuckets)[i].cend();
    }

    //*********************************************************************
    /// Returns the bucket index for the key.
    ///\return The bucket index for the key.
    //*********************************************************************
    size_type bucket(key_value_parameter_t key) const
    {
      return key_hash_function(key) % pbuckets->size();
    }

    //*********************************************************************
    /// Returns the size of the bucket key.
    ///\return The bucket size of the bucket key.
    //*********************************************************************
    size_type bucket_size(key_value_parameter_t key) const
    {
      size_t index = bucket(key);

      return std::distance((*pbuckets)[index].begin(), (*pbuckets)[index].end());
    }

    //*********************************************************************
    /// Returns the maximum number of the buckets the container can hold.
    ///\return The maximum number of the buckets the container can hold.
    //*********************************************************************
    size_type max_bucket_count() const
    {
      return max_size();
    }

    //*********************************************************************
    /// Returns the number of the buckets the container holds.
    ///\return The number of the buckets the container holds.
    //*********************************************************************
    size_type bucket_count() const
    {
      return max_size();
    }

    //*********************************************************************
    /// Assigns values to the unordered_set.
    /// If asserts or exceptions are enabled, emits unordered_set_full if the unordered_set does not have enough free space.
    /// If asserts or exceptions are enabled, emits unordered_set_iterator if the iterators are reversed.
    ///\param first The iterator to the first element.
    ///\param last  The iterator to the last element + 1.
    //*********************************************************************
    template <typename TIterator>
    void assign(TIterator first, TIterator last)
    {
#ifdef _DEBUG
      difference_type count = std::distance(first, last);
      ETL_ASSERT(count >= 0, ETL_ERROR(unordered_set_iterator));
      ETL_ASSERT(size_t(count) <= max_size() , ETL_ERROR(unordered_set_full));
#endif

      clear();

      while (first != last)
      {
        insert(*first++);
      }
    }

    //*********************************************************************
    /// Inserts a value to the unordered_set.
    /// If asserts or exceptions are enabled, emits unordered_set_full if the unordered_set is already full.
    ///\param value The value to insert.
    //*********************************************************************
    std::pair<iterator, bool> insert(const value_type& key)
    {
      std::pair<iterator, bool> result(end(), false);

      ETL_ASSERT(!full(), ETL_ERROR(unordered_set_full));

      // Get the hash index.
      size_t index = bucket(key);

      // Get the bucket & bucket iterator.
      bucket_list_iterator ibucket = pbuckets->begin() + index;
      bucket_t& bucket = *ibucket;

      // The first one in the bucket?
      if (bucket.empty())
      {
        // Get a new node.
        node_t& node = *pnodepool->allocate(node_t(key));

        // Just add the pointer to the bucket;
        bucket.insert_after(bucket.before_begin(), node);

        result.first  = iterator(pbuckets->end(), ibucket, ibucket->begin());
        result.second = true;

        adjust_first_last_markers(ibucket);
      }
      else
      {
        // Step though the bucket looking for a place to insert.
        local_iterator inode_previous = bucket.before_begin();
        local_iterator inode = bucket.begin();

        while (inode != bucket.end())
        {
          // Do we already have this key?
          if (inode->key == key)
          {
            break;
          }

          ++inode_previous;
          ++inode;
        }

        // Not already there?
        if (inode == bucket.end())
        {
          // Get a new node.
          node_t& node = *pnodepool->allocate(node_t(key));

          // Add the node to the end of the bucket;
          bucket.insert_after(inode_previous, node);
          ++inode_previous;

          result.first  = iterator(pbuckets->end(), ibucket, inode_previous);
          result.second = true;
        }
      }

      return result;
    }

    //*********************************************************************
    /// Inserts a value to the unordered_set.
    /// If asserts or exceptions are enabled, emits unordered_set_full if the unordered_set is already full.
    ///\param position The position to insert at.
    ///\param value    The value to insert.
    //*********************************************************************
    iterator insert(const_iterator position, const value_type& key)
    {
      return insert(key).first;
    }

    //*********************************************************************
    /// Inserts a range of values to the unordered_set.
    /// If asserts or exceptions are enabled, emits unordered_set_full if the unordered_set does not have enough free space.
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
      size_t count = 0;
      size_t bucket_id = bucket(key);

      bucket_t& bucket = (*pbuckets)[bucket_id];

      local_iterator iprevious = bucket.before_begin();
      local_iterator icurrent  = bucket.begin();

      while ((icurrent != bucket.end()) && (icurrent->key != key))
      {
        ++iprevious;
        ++icurrent;
      }

      if (icurrent != bucket.end())
      {
        bucket.erase_after(iprevious);
        count = 1;
      }

      return count;
    }

    //*********************************************************************
    /// Erases an element.
    ///\param ielement Iterator to the element.
    //*********************************************************************
    iterator erase(const_iterator ielement)
    {
      // Make a note of the next one.
      iterator inext(pbuckets->end(), ielement.get_bucket_list_iterator(), ielement.get_local_iterator());
      ++inext;

      bucket_t&      bucket    = ielement.get_bucket();
      local_iterator icurrent  = ielement.get_local_iterator();
      local_iterator iprevious = bucket.before_begin();

      // Find the node we're interested in.
      while (iprevious->etl_next != &*icurrent)
      {
        ++iprevious;
      }

      bucket.erase_after(iprevious);

      return inext;
    }

    //*********************************************************************
    /// Erases a range of elements.
    /// The range includes all the elements between first and last, including the
    /// element pointed by first, but not the one pointed to by last.
    ///\param first Iterator to the first element.
    ///\param last  Iterator to the last element.
    //*********************************************************************
    iterator erase(const_iterator first, const_iterator last)
    {
      // Make a note of the last.
      iterator result(pbuckets->end(), last.get_bucket_list_iterator(), last.get_local_iterator());

      // Get the starting point.
      bucket_list_iterator ibucket   = first.get_bucket_list_iterator();
      local_iterator       ifirst    = first.get_local_iterator();
      local_iterator       iprevious = ibucket->before_begin();
      local_iterator       iend;

      // Find the first node we're interested in.
      while (iprevious->etl_next != &*ifirst)
      {
        ++iprevious;
      }

      iend = iprevious;
      iend++;

      while (first != last)
      {
        // Find how far we can go in this bucket.
        while ((first != last) && (iend != ibucket->end()))
        {
          ++first;
          ++iend;
        }

        // Erase the range.
        ibucket->erase_after(iprevious, iend);

        // At the end of this bucket?
        if (iend == ibucket->end())
        {
          // Move on to the next bucket.
          ++ibucket;
          iprevious = ibucket->before_begin();
          iend = iprevious;
          ++iend;
        }
        else
        {
          // Still in the same bucket.
          iprevious = iend;
        }
      }

      return result;
    }

    //*************************************************************************
    /// Clears the unordered_set.
    //*************************************************************************
    void clear()
    {
      initialise();
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
    /// Finds an element.
    ///\param key The key to search for.
    ///\return An iterator to the element if the key exists, otherwise end().
    //*********************************************************************
    iterator find(key_value_parameter_t key)
    {
      size_t index = bucket(key);

      bucket_list_iterator ibucket = pbuckets->begin() + index;
      bucket_t&            bucket  = *ibucket;

      // Is the bucket not empty?
      if (!bucket.empty())
      {
        // Step though the list until we find the end or an equivalent key.
        local_iterator inode = bucket.begin();
        local_iterator iend  = bucket.end();

        while (inode != iend)
        {
          // Do we have this one?
          if (key_equal_function(key, inode->key))
          {
            return iterator(pbuckets->end(), ibucket, inode);
          }

          ++inode;
        }
      }

      return end();
    }

    //*********************************************************************
    /// Finds an element.
    ///\param key The key to search for.
    ///\return An iterator to the element if the key exists, otherwise end().
    //*********************************************************************
    const_iterator find(key_value_parameter_t key) const
    {
      size_t index = bucket(key);

      bucket_list_iterator ibucket = pbuckets->begin() + index;
      bucket_t&            bucket  = *ibucket;

      // Is the bucket not empty?
      if (!bucket.empty())
      {
        // Step though the list until we find the end or an equivalent key.
        local_iterator inode = bucket.begin();
        local_iterator iend = bucket.end();

        while (inode != iend)
        {
          // Do we have this one?
          if (key_equal_function(key, inode->key))
          {
            return iterator(pbuckets->end(), ibucket, inode);
          }

          ++inode;
        }
      }

      return end();
    }

    //*********************************************************************
    /// Returns a range containing all elements with key 'key' in the container.
    /// The range is defined by two iterators, the first pointing to the first 
    /// element of the wanted range and the second pointing past the last 
    /// element of the range.
    ///\param key The key to search for.
    ///\return An iterator pair to the range of elements if the key exists, otherwise end().
    //*********************************************************************
    std::pair<iterator, iterator> equal_range(const key_value_parameter_t& key)
    {
      iterator first = find(key);
      iterator last  = first;
      
      if (last != end())
      {
        ++last;
      }

      return std::pair<iterator, iterator>(first, last);
    }

    //*********************************************************************
    /// Returns a range containing all elements with key 'key' in the container.
    /// The range is defined by two iterators, the first pointing to the first 
    /// element of the wanted range and the second pointing past the last 
    /// element of the range.
    ///\param key The key to search for.
    ///\return A const iterator pair to the range of elements if the key exists, otherwise end().
    //*********************************************************************
    std::pair<const_iterator, const_iterator> equal_range(const key_value_parameter_t& key) const
    {
      const_iterator first = find(key);
      const_iterator last  = first;

      if (last != end())
      {
        ++last;
      }

      return std::pair<const_iterator, const_iterator>(first, last);
    }

    //*************************************************************************
    /// Gets the size of the unordered_set.
    //*************************************************************************
    size_type size() const
    {
      return pnodepool->size();
    }

    //*************************************************************************
    /// Gets the maximum possible size of the unordered_set.
    //*************************************************************************
    size_type max_size() const
    {
      return pnodepool->max_size();
    }

    //*************************************************************************
    /// Checks to see if the unordered_set is empty.
    //*************************************************************************
    bool empty() const
    {
      return pnodepool->empty();
    }

    //*************************************************************************
    /// Checks to see if the unordered_set is full.
    //*************************************************************************
    bool full() const
    {
      return pnodepool->full();
    }

    //*************************************************************************
    /// Returns the remaining capacity.
    ///\return The remaining capacity.
    //*************************************************************************
    size_t available() const
    {
      return pnodepool->available();
    }

    //*************************************************************************
    /// Returns the load factor = size / bucket_count.
    ///\return The load factor = size / bucket_count.
    //*************************************************************************
    float load_factor() const
    {
      return static_cast<float>(size()) / static_cast<float>(bucket_count());
    }

    //*************************************************************************
    /// Returns the function that hashes the keys.
    ///\return The function that hashes the keys..
    //*************************************************************************
    hasher hash_function() const
    {
      return key_hash_function;
    }

    //*************************************************************************
    /// Returns the function that compares the keys.
    ///\return The function that compares the keys..
    //*************************************************************************
    key_equal key_eq() const
    {
      return key_equal_function;
    }

    //*************************************************************************
    /// Assignment operator.
    //*************************************************************************
    iunordered_set& operator = (const iunordered_set& rhs)
    {
      // Skip if doing self assignment
      if (this != &rhs)
      {
        assign(rhs.cbegin(), rhs.cend());
      }

      return *this;
    }

  protected:

    //*********************************************************************
    /// Constructor.
    //*********************************************************************
    iunordered_set(pool_t& node_pool, bucket_list_t& buckets)
      : pnodepool(&node_pool),
        pbuckets(&buckets)
    {
    }

    //*********************************************************************
    /// Initialise the unordered_set.
    //*********************************************************************
    void initialise()
    {
      pbuckets->resize(pnodepool->max_size());

      if (!empty())
      {
        pnodepool->release_all();

        for (size_t i = 0; i < pbuckets->size(); ++i)
        {
          (*pbuckets)[i].clear();
        }
      }

      first = pbuckets->begin();
      last  = first;
    }

  private:

    //*********************************************************************
    /// Adjust the first and last markers according to the new entry.
    //*********************************************************************
    void adjust_first_last_markers(bucket_list_iterator ibucket)
    {
      if (ibucket < first)
      {
        first = ibucket;
      }
      else if (ibucket > last)
      {
        last = ibucket;
      }
    }

    // Disable copy construction.
    iunordered_set(const iunordered_set&);

    /// The pool of data nodes used in the list.
    pool_t* pnodepool;

    /// The bucket list.
    bucket_list_t* pbuckets;

    /// The first and last iterators to buckets with values.
    bucket_list_iterator first;
    bucket_list_iterator last;

    /// The function that creates the hashes.
    hasher key_hash_function;

    /// The function that compares the keys for equality.
    key_equal key_equal_function;
  };

  //***************************************************************************
  /// Equal operator.
  ///\param lhs Reference to the first unordered_set.
  ///\param rhs Reference to the second unordered_set.
  ///\return <b>true</b> if the arrays are equal, otherwise <b>false</b>
  ///\ingroup unordered_set
  //***************************************************************************
  template <typename TKey, typename TMapped, typename TKeyCompare>
  bool operator ==(const etl::iunordered_set<TKey, TMapped, TKeyCompare>& lhs, const etl::iunordered_set<TKey, TMapped, TKeyCompare>& rhs)
  {
    return (lhs.size() == rhs.size()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  //***************************************************************************
  /// Not equal operator.
  ///\param lhs Reference to the first unordered_set.
  ///\param rhs Reference to the second unordered_set.
  ///\return <b>true</b> if the arrays are not equal, otherwise <b>false</b>
  ///\ingroup unordered_set
  //***************************************************************************
  template <typename TKey, typename TMapped, typename TKeyCompare>
  bool operator !=(const etl::iunordered_set<TKey, TMapped, TKeyCompare>& lhs, const etl::iunordered_set<TKey, TMapped, TKeyCompare>& rhs)
  {
    return !(lhs == rhs);
  }
}

#undef ETL_FILE
#endif
