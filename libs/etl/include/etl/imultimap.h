///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
http://www.etlcpp.com

Copyright(c) 2014 jwellbelove, rlindeman

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

#ifndef __ETL_IMULTIMAP__
#define __ETL_IMULTIMAP__
#define __ETL_IN_IMULTIMAP_H__

#include <iterator>
#include <algorithm>
#include <functional>
#include <stddef.h>

#include "nullptr.h"
#include "private/multimap_base.h"
#include "type_traits.h"
#include "parameter_type.h"
#include "pool.h"
#include "platform.h"

#ifdef ETL_COMPILER_MICROSOFT
#undef min
#endif

namespace etl
{
  //***************************************************************************
  /// A templated base for all etl::multimap types.
  ///\ingroup map
  //***************************************************************************
  template <typename TKey, typename TMapped, typename TKeyCompare>
  class imultimap : public multimap_base
  {
  public:

    typedef std::pair<const TKey, TMapped> value_type;
    typedef const TKey                     key_type;
    typedef TMapped                        mapped_type;
    typedef TKeyCompare                    key_compare;
    typedef value_type&                    reference;
    typedef const value_type&              const_reference;
    typedef value_type*                    pointer;
    typedef const value_type*              const_pointer;
    typedef size_t                         size_type;

    //*************************************************************************
    /// How to compare two key elements.
    //*************************************************************************
    struct key_comp
    {
      bool operator ()(const key_type& key1, const key_type& key2) const
      {
        return key_compare()(key1, key2);
      }
    };

    //*************************************************************************
    /// How to compare two value elements.
    //*************************************************************************
    struct value_comp
    {
      bool operator ()(const value_type& value1, const value_type& value2) const
      {
        return key_compare()(value1.first, value2.first);
      }
    };

  protected:

    //*************************************************************************
    /// The data node element in the multimap.
    //*************************************************************************
    struct Data_Node : public Node
    {
      explicit Data_Node(value_type value)
        : value(value)
      {
      }

      value_type value;
    };

    /// Defines the key value parameter type
    typedef typename parameter_type<TKey>::type key_value_parameter_t;

    //*************************************************************************
    /// How to compare node elements.
    //*************************************************************************
    bool node_comp(const Data_Node& node1, const Data_Node& node2) const
    {
      return key_compare()(node1.value.first, node2.value.first);
    }

    bool node_comp(const Data_Node& node, const key_value_parameter_t& key) const
    {
      return key_compare()(node.value.first, key);
    }

    bool node_comp(const key_value_parameter_t& key, const Data_Node& node) const
    {
      return key_compare()(key, node.value.first);
    }

  private:

    /// The pool of data nodes used in the multimap.
    ipool<Data_Node>* p_node_pool;

    //*************************************************************************
    /// Downcast a Node* to a Data_Node*
    //*************************************************************************
    static Data_Node* data_cast(Node* p_node)
    {
      return static_cast<Data_Node*>(p_node);
    }

    //*************************************************************************
    /// Downcast a Node& to a Data_Node&
    //*************************************************************************
    static Data_Node& data_cast(Node& node)
    {
      return static_cast<Data_Node&>(node);
    }

    //*************************************************************************
    /// Downcast a const Node* to a const Data_Node*
    //*************************************************************************
    static const Data_Node* data_cast(const Node* p_node)
    {
      return static_cast<const Data_Node*>(p_node);
    }

    //*************************************************************************
    /// Downcast a const Node& to a const Data_Node&
    //*************************************************************************
    static const Data_Node& data_cast(const Node& node)
    {
      return static_cast<const Data_Node&>(node);
    }

  public:
    //*************************************************************************
    /// iterator.
    //*************************************************************************
    class iterator : public std::iterator<std::bidirectional_iterator_tag, value_type>
    {
    public:

      friend class imultimap;

      iterator()
        : p_multimap(nullptr)
        , p_node(nullptr)
      {
      }

      iterator(imultimap& multimap)
        : p_multimap(&multimap)
        , p_node(nullptr)
      {
      }

      iterator(imultimap& multimap, Node* node)
        : p_multimap(&multimap)
        , p_node(node)
      {
      }

      iterator(const iterator& other)
        : p_multimap(other.p_multimap)
        , p_node(other.p_node)
      {
      }

      ~iterator()
      {
      }

      iterator& operator ++()
      {
        p_multimap->next_node(p_node);
        return *this;
      }

      iterator operator ++(int)
      {
        iterator temp(*this);
        p_multimap->next_node(p_node);
        return temp;
      }

      iterator& operator --()
      {
        p_multimap->prev_node(p_node);
        return *this;
      }

      iterator operator --(int)
      {
        iterator temp(*this);
        p_multimap->prev_node(p_node);
        return temp;
      }

      iterator operator =(const iterator& other)
      {
        p_multimap = other.p_multimap;
        p_node = other.p_node;
        return *this;
      }

      reference operator *()
      {
        return imultimap::data_cast(p_node)->value;
      }

      const_reference operator *() const
      {
        return imultimap::data_cast(p_node)->value;
      }

      pointer operator &()
      {
        return &(imultimap::data_cast(p_node)->value);
      }

      const_pointer operator &() const
      {
        return &(imultimap::data_cast(p_node)->value);
      }

      pointer operator ->()
      {
        return &(imultimap::data_cast(p_node)->value);
      }

      const_pointer operator ->() const
      {
        return &(imultimap::data_cast(p_node)->value);
      }

      friend bool operator == (const iterator& lhs, const iterator& rhs)
      {
        return lhs.p_multimap == rhs.p_multimap && lhs.p_node == rhs.p_node;
      }

      friend bool operator != (const iterator& lhs, const iterator& rhs)
      {
        return !(lhs == rhs);
      }

    private:

      // Pointer to multimap associated with this iterator
      imultimap* p_multimap;

      // Pointer to the current node for this iterator
      Node* p_node;
    };
    friend class iterator;

    //*************************************************************************
    /// const_iterator
    //*************************************************************************
    class const_iterator : public std::iterator<std::bidirectional_iterator_tag, const value_type>
    {
    public:

      friend class imultimap;

      const_iterator()
        : p_multimap(nullptr)
        , p_node(nullptr)
      {
      }

      const_iterator(const imultimap& multimap)
        : p_multimap(&multimap)
        , p_node(nullptr)
      {
      }

      const_iterator(const imultimap& multimap, const Node* node)
        : p_multimap(&multimap)
        , p_node(node)
      {
      }

      const_iterator(const typename imultimap::iterator& other)
        : p_multimap(other.p_multimap)
        , p_node(other.p_node)
      {
      }

      const_iterator(const const_iterator& other)
        : p_multimap(other.p_multimap)
        , p_node(other.p_node)
      {
      }

      ~const_iterator()
      {
      }

      const_iterator& operator ++()
      {
        p_multimap->next_node(p_node);
        return *this;
      }

      const_iterator operator ++(int)
      {
        const_iterator temp(*this);
        p_multimap->next_node(p_node);
        return temp;
      }

      const_iterator& operator --()
      {
        p_multimap->prev_node(p_node);
        return *this;
      }

      const_iterator operator --(int)
      {
        const_iterator temp(*this);
        p_multimap->prev_node(p_node);
        return temp;
      }

      const_iterator operator =(const const_iterator& other)
      {
        p_multimap = other.p_multimap;
        p_node = other.p_node;
        return *this;
      }

      const_reference operator *() const
      {
        return imultimap::data_cast(p_node)->value;
      }

      const_pointer operator &() const
      {
        return imultimap::data_cast(p_node)->value;
      }

      const_pointer operator ->() const
      {
        return &(imultimap::data_cast(p_node)->value);
      }

      friend bool operator == (const const_iterator& lhs, const const_iterator& rhs)
      {
        return lhs.p_multimap == rhs.p_multimap && lhs.p_node == rhs.p_node;
      }

      friend bool operator != (const const_iterator& lhs, const const_iterator& rhs)
      {
        return !(lhs == rhs);
      }

    private:
      // Pointer to multimap associated with this iterator
      const imultimap* p_multimap;

      // Pointer to the current node for this iterator
      const Node* p_node;
    };
    friend class const_iterator;

    typedef typename std::iterator_traits<iterator>::difference_type difference_type;

    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


    //*************************************************************************
    /// Gets the beginning of the multimap.
    //*************************************************************************
    iterator begin()
    {
      return iterator(*this, find_limit_node(root_node, kLeft));
    }

    //*************************************************************************
    /// Gets the beginning of the multimap.
    //*************************************************************************
    const_iterator begin() const
    {
      return const_iterator(*this, find_limit_node(root_node, kLeft));
    }

    //*************************************************************************
    /// Gets the end of the multimap.
    //*************************************************************************
    iterator end()
    {
      return iterator(*this);
    }

    //*************************************************************************
    /// Gets the end of the multimap.
    //*************************************************************************
    const_iterator end() const
    {
      return const_iterator(*this);
    }

    //*************************************************************************
    /// Gets the beginning of the multimap.
    //*************************************************************************
    const_iterator cbegin() const
    {
      return const_iterator(*this, find_limit_node(root_node, kLeft));
    }

    //*************************************************************************
    /// Gets the end of the multimap.
    //*************************************************************************
    const_iterator cend() const
    {
      return const_iterator(*this);
    }

    //*************************************************************************
    /// Gets the reverse beginning of the list.
    //*************************************************************************
    reverse_iterator rbegin()
    {
      return reverse_iterator(iterator(*this));
    }

    //*************************************************************************
    /// Gets the reverse beginning of the list.
    //*************************************************************************
    const_reverse_iterator rbegin() const
    {
      return const_reverse_iterator(const_iterator(*this));
    }

    //*************************************************************************
    /// Gets the reverse end of the list.
    //*************************************************************************
    reverse_iterator rend()
    {
      return reverse_iterator(iterator(*this, find_limit_node(root_node, kLeft)));
    }

    //*************************************************************************
    /// Gets the reverse end of the list.
    //*************************************************************************
    const_reverse_iterator rend() const
    {
      return const_reverse_iterator(iterator(*this, find_limit_node(root_node, kLeft)));
    }

    //*************************************************************************
    /// Gets the reverse beginning of the list.
    //*************************************************************************
    const_reverse_iterator crbegin() const
    {
      return const_reverse_iterator(const_iterator(*this));
    }

    //*************************************************************************
    /// Gets the reverse end of the list.
    //*************************************************************************
    const_reverse_iterator crend() const
    {
      return const_reverse_iterator(const_iterator(*this, find_limit_node(root_node, kLeft)));
    }

    //*********************************************************************
    /// Assigns values to the multimap.
    /// If asserts or exceptions are enabled, emits map_full if the multimap does not have enough free space.
    /// If asserts or exceptions are enabled, emits map_iterator if the iterators are reversed.
    ///\param first The iterator to the first element.
    ///\param last  The iterator to the last element + 1.
    //*********************************************************************
    template <typename TIterator>
    void assign(TIterator first, TIterator last)
    {
      initialise();
      insert(first, last);
    }

    //*************************************************************************
    /// Clears the multimap.
    //*************************************************************************
    void clear()
    {
      initialise();
    }

    //*********************************************************************
    /// Counts the number of elements that contain the key specified.
    ///\param key The key to search for.
    ///\return 1 if element was found, 0 otherwise.
    //*********************************************************************
    size_type count(const key_value_parameter_t& key) const
    {
      return count_nodes(key);
    }

    //*************************************************************************
    /// Returns two iterators with bounding (lower bound, upper bound) the key
    /// provided
    //*************************************************************************
    std::pair<iterator, iterator> equal_range(const key_value_parameter_t& key)
    {
      return std::make_pair<iterator, iterator>(
        iterator(*this, find_lower_node(root_node, key)),
        iterator(*this, find_upper_node(root_node, key)));
    }

    //*************************************************************************
    /// Returns two const iterators with bounding (lower bound, upper bound)
    /// the key provided.
    //*************************************************************************
    std::pair<const_iterator, const_iterator> equal_range(const key_value_parameter_t& key) const
    {
      return std::make_pair<const_iterator, const_iterator>(
        const_iterator(*this, find_lower_node(root_node, key)),
        const_iterator(*this, find_upper_node(root_node, key)));
    }

    //*************************************************************************
    /// Erases the value at the specified position.
    //*************************************************************************
    void erase(iterator position)
    {
      // Remove the node by its node specified in iterator position
      (void)erase(const_iterator(position));
    }

    //*************************************************************************
    /// Erases the value at the specified position.
    //*************************************************************************
    iterator erase(const_iterator position)
    {
      // Cast const away from node to be removed. This is necessary because the
      // STL definition of this method requires we provide the next node in the
      // sequence as an iterator.
      Node* node = const_cast<Node*>(position.p_node);
      iterator next(*this, node);
      ++next;

      // Remove the non-const node provided
      remove_node(node);

      return next;
    }

    //*************************************************************************
    // Erase the key specified.
    //*************************************************************************
    size_type erase(const key_value_parameter_t& key)
    {
      // Number of nodes removed
      size_type count = 0;
      const_iterator lower(*this, find_lower_node(root_node, key));
      const_iterator upper(*this, find_upper_node(root_node, key));
      while (lower != upper)
      {
        // Increment count for each node removed
        ++count;
        // Remove node using the other erase method
        (void)erase(lower++);
      }

      // Return the total count erased
      return count;
    }

    //*************************************************************************
    /// Erases a range of elements.
    //*************************************************************************
    iterator erase(iterator first, iterator last)
    {
      iterator next;
      while (first != last)
      {
        next = erase(const_iterator(first++));
      }

      return next;
    }

    //*************************************************************************
    /// Erases a range of elements.
    //*************************************************************************
    iterator erase(const_iterator first, const_iterator last)
    {
      iterator next;
      while (first != last)
      {
        next = erase(first++);
      }

      return next;
    }

    //*********************************************************************
    /// Finds an element.
    ///\param key The key to search for.
    ///\return An iterator pointing to the element or end() if not found.
    //*********************************************************************
    iterator find(const key_value_parameter_t& key)
    {
      return iterator(*this, find_node(root_node, key));
    }

    //*********************************************************************
    /// Finds an element.
    ///\param key The key to search for.
    ///\return An iterator pointing to the element or end() if not found.
    //*********************************************************************
    const_iterator find(const key_value_parameter_t& key) const
    {
      return const_iterator(*this, find_node(root_node, key));
    }

    //*********************************************************************
    /// Inserts a value to the multimap.
    /// If asserts or exceptions are enabled, emits map_full if the multimap is already full.
    ///\param value    The value to insert.
    //*********************************************************************
    iterator insert(const value_type& value)
    {
      // Default to no inserted node
      Node* inserted_node = nullptr;

      ETL_ASSERT(!full(), ETL_ERROR(multimap_full));

      // Get next available free node
      Data_Node& node = allocate_data_node(value);

      // Obtain the inserted node (might be nullptr if node was a duplicate)
      inserted_node = insert_node(root_node, node);

      // Insert node into tree and return iterator to new node location in tree
      return iterator(*this, inserted_node);
    }

    //*********************************************************************
    /// Inserts a value to the multimap starting at the position recommended.
    /// If asserts or exceptions are enabled, emits map_full if the multimap is already full.
    ///\param position The position that would precede the value to insert.
    ///\param value    The value to insert.
    //*********************************************************************
    iterator insert(iterator position, const value_type& value)
    {
      // Ignore position provided and just do a normal insert
      return insert(value);
    }

    //*********************************************************************
    /// Inserts a value to the multimap starting at the position recommended.
    /// If asserts or exceptions are enabled, emits map_full if the multimap is already full.
    ///\param position The position that would precede the value to insert.
    ///\param value    The value to insert.
    //*********************************************************************
    iterator insert(const_iterator position, const value_type& value)
    {
      // Ignore position provided and just do a normal insert
      return insert(value);
    }

    //*********************************************************************
    /// Inserts a range of values to the multimap.
    /// If asserts or exceptions are enabled, emits map_full if the multimap does not have enough free space.
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
    /// Returns an iterator pointing to the first element in the container
    /// whose key is not considered to go before the key provided or end()
    /// if all keys are considered to go before the key provided.
    ///\return An iterator pointing to the element not before key or end()
    //*********************************************************************
    iterator lower_bound(const key_value_parameter_t& key)
    {
      return iterator(*this, find_lower_node(root_node, key));
    }

    //*********************************************************************
    /// Returns a const_iterator pointing to the first element in the
    /// container whose key is not considered to go before the key provided
    /// or end() if all keys are considered to go before the key provided.
    ///\return An const_iterator pointing to the element not before key or end()
    //*********************************************************************
    const_iterator lower_bound(const key_value_parameter_t& key) const
    {
      return const_iterator(*this, find_lower_node(root_node, key));
    }

    //*********************************************************************
    /// Returns an iterator pointing to the first element in the container
    /// whose key is not considered to go after the key provided or end()
    /// if all keys are considered to go after the key provided.
    ///\return An iterator pointing to the element after key or end()
    //*********************************************************************
    iterator upper_bound(const key_value_parameter_t& key)
    {
      return iterator(*this, find_upper_node(root_node, key));
    }

    //*********************************************************************
    /// Returns a const_iterator pointing to the first element in the
    /// container whose key is not considered to go after the key provided
    /// or end() if all keys are considered to go after the key provided.
    ///\return An const_iterator pointing to the element after key or end()
    //*********************************************************************
    const_iterator upper_bound(const key_value_parameter_t& key) const
    {
      return const_iterator(*this, find_upper_node(root_node, key));
    }

    //*************************************************************************
    /// Assignment operator.
    //*************************************************************************
    imultimap& operator = (const imultimap& rhs)
    {
      // Skip if doing self assignment
      if (this != &rhs)
      {
        assign(rhs.cbegin(), rhs.cend());
      }

      return *this;
    }

  protected:

    //*************************************************************************
    /// Constructor.
    //*************************************************************************
    imultimap(ipool<Data_Node>& node_pool, size_t max_size_)
      : multimap_base(max_size_)
      , p_node_pool(&node_pool)
    {
    }

    //*************************************************************************
    /// Initialise the multimap.
    //*************************************************************************
    void initialise()
    {
      if (!empty())
      {
        p_node_pool->release_all();
      }

      current_size = 0;
      root_node = nullptr;
    }

  private:

    //*************************************************************************
    /// Allocate a Data_Node.
    //*************************************************************************
    Data_Node& allocate_data_node(value_type value) const
    {
      return *(p_node_pool->allocate(Data_Node(value)));
    }

    //*************************************************************************
    /// Destroy a Data_Node.
    //*************************************************************************
    void destroy_data_node(Data_Node& node) const
    {
      p_node_pool->release(&node);
    }

    //*************************************************************************
    /// Count the nodes that match the key provided
    //*************************************************************************
    size_type count_nodes(const key_value_parameter_t& key) const
    {
      // Number of nodes that match the key provided result
      size_type result = 0;

      // Find lower and upper nodes for the key provided
      const Node* lower = find_lower_node(root_node, key);
      const Node* upper = find_upper_node(root_node, key);

      // Loop from lower node to upper node and find nodes that match
      while (lower != upper)
      {
        // Downcast found to Data_Node class for comparison and other operations
        const Data_Node& data_node = imultimap::data_cast(*lower);

        if (!node_comp(key, data_node) && !node_comp(data_node, key))
        {
          // This node matches the key provided
          ++result;
        }

        // Move on to the next node
        next_node(lower);
      }

      // Return the number of nodes that match
      return result;
    }

    //*************************************************************************
    /// Find the value matching the node provided
    //*************************************************************************
    Node* find_node(Node* position, const key_value_parameter_t& key) const
    {
      Node* found = nullptr;
      while (position)
      {
        // Downcast found to Data_Node class for comparison and other operations
        Data_Node& data_node = imultimap::data_cast(*position);
        // Compare the node value to the current position value
        if (node_comp(key, data_node))
        {
          // Keep searching for the node on the left
          position = position->children[kLeft];
        }
        else if (node_comp(data_node, key))
        {
          // Keep searching for the node on the right
          position = position->children[kRight];
        }
        else
        {
          // We found one, keep looking for more on the left
          found = position;
          position = position->children[kLeft];
        }
      }

      // Return the node found (might be nullptr)
      return found;
    }

    //*************************************************************************
    /// Find the value matching the node provided
    //*************************************************************************
    const Node* find_node(const Node* position, const key_value_parameter_t& key) const
    {
      const Node* found = nullptr;
      while (position)
      {
        // Downcast found to Data_Node class for comparison and other operations
        const Data_Node& data_node = imultimap::data_cast(*position);
        // Compare the node value to the current position value
        if (node_comp(key, data_node))
        {
          // Keep searching for the node on the left
          position = position->children[kLeft];
        }
        else if (node_comp(data_node, key))
        {
          // Keep searching for the node on the right
          position = position->children[kRight];
        }
        else
        {
          // We found one, keep looking for more on the left
          found = position;
          position = position->children[kLeft];
        }
      }

      // Return the node found (might be nullptr)
      return found;
    }

    //*************************************************************************
    /// Find the node whose key is not considered to go before the key provided
    //*************************************************************************
    Node* find_lower_node(Node* position, const key_value_parameter_t& key) const
    {
      // Something at this position? keep going
      Node* lower_node = nullptr;
      while (position)
      {
        // Downcast lower node to Data_Node reference for key comparisons
        Data_Node& data_node = imultimap::data_cast(*position);
        // Compare the key value to the current lower node key value
        if (node_comp(key, data_node))
        {
          lower_node = position;
          if (position->children[kLeft])
          {
            position = position->children[kLeft];
          }
          else
          {
            // Found lowest node
            break;
          }
        }
        else if (node_comp(data_node, key))
        {
          position = position->children[kRight];
        }
        else
        {
          // Make note of current position, but keep looking to left for more
          lower_node = position;
          position = position->children[kLeft];
        }
      }

      // Return the lower_node position found
      return lower_node;
    }

    //*************************************************************************
    /// Find the node whose key is considered to go after the key provided
    //*************************************************************************
    Node* find_upper_node(Node* position, const key_value_parameter_t& key) const
    {
      // Keep track of parent of last upper node
      Node* upper_node = nullptr;
      // Has an equal node been found? start with no
      bool found = false;
      while (position)
      {
        // Downcast position to Data_Node reference for key comparisons
        Data_Node& data_node = imultimap::data_cast(*position);
        // Compare the key value to the current upper node key value
        if (node_comp(data_node, key))
        {
          position = position->children[kRight];
        }
        else if (node_comp(key, data_node))
        {
          upper_node = position;
          // If a node equal to key hasn't been found go left
          if (!found && position->children[kLeft])
          {
            position = position->children[kLeft];
          }
          else
          {
            break;
          }
        }
        else
        {
          // We found an equal item, break on next bigger item
          found = true;
          next_node(position);
        }
      }

      // Return the upper node position found (might be nullptr)
      return upper_node;
    }

    //*************************************************************************
    /// Insert a node.
    //*************************************************************************
    Node* insert_node(Node*& position, Data_Node& node)
    {
      // Find the location where the node belongs
      Node* found = position;

      // Was position provided not empty? then find where the node belongs
      if (position)
      {
        // Find the critical parent node (default to nullptr)
        Node* critical_parent_node = nullptr;
        Node* critical_node = root_node;

        while (found)
        {
          // Search for critical weight node (all nodes whose weight factor
          // is set to kNeither (balanced)
          if (kNeither != found->weight)
          {
            critical_node = found;
          }

          // Downcast found to Data_Node class for comparison and other operations
          Data_Node& found_data_node = imultimap::data_cast(*found);

          // Is the node provided to the left of the current position?
          if (node_comp(node, found_data_node))
          {
            // Update direction taken to insert new node in parent node
            found->dir = kLeft;
          }
          // Is the node provided to the right of the current position?
          else if (node_comp(found_data_node, node))
          {
            // Update direction taken to insert new node in parent node
            found->dir = kRight;
          }
          else
          {
            // Update direction taken to insert new node in parent (and
            // duplicate) node to the right.
            found->dir = kRight;
          }

          // Is there a child of this parent node?
          if (found->children[found->dir])
          {
            // Will this node be the parent of the next critical node whose
            // weight factor is set to kNeither (balanced)?
            if (kNeither != found->children[found->dir]->weight)
            {
              critical_parent_node = found;
            }

            // Keep looking for empty spot to insert new node
            found = found->children[found->dir];
          }
          else
          {
            // Attach node as a child of the parent node found
            attach_node(found, found->children[found->dir], node);

            // Return newly added node
            found = found->children[found->dir];

            // Exit loop
            break;
          }
        }

        // Was a critical node found that should be checked for balance?
        if (critical_node)
        {
          if (critical_parent_node == nullptr && critical_node == root_node)
          {
            balance_node(root_node);
          }
          else if (critical_parent_node == nullptr && critical_node == position)
          {
            balance_node(position);
          }
          else
          {
            balance_node(critical_parent_node->children[critical_parent_node->dir]);
          }
        }
      }
      else
      {
        // Attach node to current position (which is assumed to be root)
        attach_node(nullptr, position, node);

        // Return newly added node at current position
        found = position;
      }

      // Return the node found (might be nullptr)
      return found;
    }

    //*************************************************************************
    /// Remove the node specified from somewhere starting at the position
    /// provided
    //*************************************************************************
    void remove_node(Node* node)
    {
      // If valid found node was provided then proceed with steps 1 through 5
      if (node)
      {
        // Downcast found node provided to Data_Node class
        Data_Node& data_node = imultimap::data_cast(*node);

        // Keep track of node as found node
        Node* found = node;

        // Step 1: Mark path from node provided back to the root node using the
        // internal temporary dir member value and using the parent pointer. This
        // will allow us to avoid recursion in finding the node in a tree that
        //might contain duplicate keys to be found.
        while (node)
        {
          if (node->parent)
          {
            // Which direction does parent use to get to this node?
            node->parent->dir =
              node->parent->children[kLeft] == node ? kLeft : kRight;

            // Make this nodes parent the next node
            node = node->parent;
          }
          else
          {
            // Root node found - break loop
            break;
          }
        }

        // Step 2: Follow the path provided above until we reach the node
        // provided and look for the balance node to start rebalancing the tree
        // from (up to the replacement node that will be found in step 3)
        Node* balance = root_node;
        while (node)
        {
          // Did we reach the node provided originally (found) then go to step 3
          if (node == found)
          {
            // Update the direction towards a replacement node at the found node
            node->dir = node->children[kLeft] ? kLeft : kRight;

            // Exit loop and proceed with step 3
            break;
          }
          else
          {
            // If this nodes weight is kNeither or we are taking the shorter path
            // to the next node and our sibling (on longer path) is balanced then
            // we need to update the balance node to this node but all our
            // ancestors will not require rebalancing
            if ((node->weight == kNeither) ||
              (node->weight == (1 - node->dir) &&
              node->children[1 - node->dir]->weight == kNeither))
            {
              // Update balance node to this node
              balance = node;
            }

            // Keep searching for found in the direction provided in step 1
            node = node->children[node->dir];
          }
        }
        // The value for node should not be nullptr at this point otherwise
        // step 1 failed to provide the correct path to found. Step 5 will fail
        // (probably subtly) if node should be nullptr at this point

        // Step 3: Find the node (node should be equal to found at this point)
        // to replace found with (might end up equal to found) while also
        // continuing to update balance the same as in step 2 above.
        while (node)
        {
          // Replacement node found if its missing a child in the replace->dir
          // value set at the end of step 2 above
          if (node->children[node->dir] == nullptr)
          {
            // Exit loop once node to replace found is determined
            break;
          }

          // If this nodes weight is kNeither or we are taking the shorter path
          // to the next node and our sibling (on longer path) is balanced then
          // we need to update the balance node to this node but all our
          // ancestors will not require rebalancing
          if ((node->weight == kNeither) ||
            (node->weight == (1 - node->dir) &&
            node->children[1 - node->dir]->weight == kNeither))
          {
            // Update balance node to this node
            balance = node;
          }

          // Keep searching for replacement node in the direction specified above
          node = node->children[node->dir];

          // Downcast node to Data_Node class for comparison operations
          Data_Node& replace_data_node = imultimap::data_cast(*node);

          // Compare the key provided to the replace data node key
          if (node_comp(data_node, replace_data_node))
          {
            // Update the direction to the replace node
            node->dir = kLeft;
          }
          else if (node_comp(replace_data_node, data_node))
          {
            // Update the direction to the replace node
            node->dir = kRight;
          }
          else
          {
            // Update the direction to the replace node
            node->dir = node->children[kLeft] ? kLeft : kRight;
          }
        } // while(node)

        // Step 4: Update weights from balance to parent of node determined
        // in step 3 above rotating (2 or 3 node rotations) as needed.
        while (balance)
        {
          // Break when balance node reaches the parent of replacement node
          if (balance->children[balance->dir] == nullptr)
          {
            break;
          }

          // If balance node is balanced already (kNeither) then just imbalance
          // the node in the opposite direction of the node being removed
          if (balance->weight == kNeither)
          {
            balance->weight = 1 - balance->dir;
          }
          // If balance node is imbalanced in the opposite direction of the
          // node being removed then the node now becomes balanced
          else if (balance->weight == balance->dir)
          {
            balance->weight = kNeither;
          }
          // Otherwise a rotation is required at this node
          else
          {
            int weight = balance->children[1 - balance->dir]->weight;
            // Perform a 3 node rotation if weight is same as balance->dir
            if (weight == balance->dir)
            {
              // Is the root node being rebalanced (no parent)
              if (balance->parent == nullptr)
              {
                rotate_3node(root_node, 1 - balance->dir,
                  balance->children[1 - balance->dir]->children[balance->dir]->weight);
              }
              else
              {
                rotate_3node(balance->parent->children[balance->parent->dir], 1 - balance->dir,
                  balance->children[1 - balance->dir]->children[balance->dir]->weight);
              }
            }
            // Already balanced, rebalance and make it heavy in opposite
            // direction of the node being removed
            else if (weight == kNeither)
            {
              // Is the root node being rebalanced (no parent)
              if (balance->parent == nullptr)
              {
                rotate_2node(root_node, 1 - balance->dir);
                root_node->weight = balance->dir;
              }
              else
              {
                // Balance parent might change during rotate, keep local copy
                // to old parent so its weight can be updated after the 2 node
                // rotate is completed
                Node* old_parent = balance->parent;
                rotate_2node(balance->parent->children[balance->parent->dir], 1 - balance->dir);
                old_parent->children[old_parent->dir]->weight = balance->dir;
              }
              // Update balance node weight in opposite direction of node removed
              balance->weight = 1 - balance->dir;
            }
            // Rebalance and leave it balanced
            else
            {
              // Is the root node being rebalanced (no parent)
              if (balance->parent == nullptr)
              {
                rotate_2node(root_node, 1 - balance->dir);
              }
              else
              {
                rotate_2node(balance->parent->children[balance->parent->dir], 1 - balance->dir);
              }
            }
          }

          // Next balance node to consider
          balance = balance->children[balance->dir];
        } // while(balance)

        // Step 5: Swap found with node (replacement)
        if (found->parent)
        {
          // Handle traditional case
          detach_node(found->parent->children[found->parent->dir],
            node->parent->children[node->parent->dir]);
        }
        // Handle root node removal
        else
        {
          // Valid replacement node for root node being removed?
          if (node->parent)
          {
            detach_node(root_node, node->parent->children[node->parent->dir]);
          }
          else
          {
            // Found node and replacement node are both root node
            detach_node(root_node, root_node);
          }
        }

        // One less.
        --current_size;

        // Destroy the node detached above
        destroy_data_node(data_node);
      } // if(found)
    }

    // Disable copy construction.
    imultimap(const imultimap&);
  };
}

//***************************************************************************
/// Equal operator.
///\param lhs Reference to the first lookup.
///\param rhs Reference to the second lookup.
///\return <b>true</b> if the arrays are equal, otherwise <b>false</b>
///\ingroup lookup
//***************************************************************************
template <typename TKey, typename TMapped, typename TKeyCompare>
bool operator ==(const etl::imultimap<TKey, TMapped, TKeyCompare>& lhs, const etl::imultimap<TKey, TMapped, TKeyCompare>& rhs)
{
  return (lhs.size() == rhs.size()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

//***************************************************************************
/// Not equal operator.
///\param lhs Reference to the first lookup.
///\param rhs Reference to the second lookup.
///\return <b>true</b> if the arrays are not equal, otherwise <b>false</b>
///\ingroup lookup
//***************************************************************************
template <typename TKey, typename TMapped, typename TKeyCompare>
bool operator !=(const etl::imultimap<TKey, TMapped, TKeyCompare>& lhs, const etl::imultimap<TKey, TMapped, TKeyCompare>& rhs)
{
  return !(lhs == rhs);
}

//*************************************************************************
/// Less than operator.
///\param lhs Reference to the first list.
///\param rhs Reference to the second list.
///\return <b>true</b> if the first list is lexicographically less than the
/// second, otherwise <b>false</b>.
//*************************************************************************
template <typename TKey, typename TMapped, typename TKeyCompare>
bool operator <(const etl::imultimap<TKey, TMapped, TKeyCompare>& lhs, const etl::imultimap<TKey, TMapped, TKeyCompare>& rhs)
{
  return std::lexicographical_compare(lhs.begin(),
                                      lhs.end(),
                                      rhs.begin(),
                                      rhs.end());
}

//*************************************************************************
/// Greater than operator.
///\param lhs Reference to the first list.
///\param rhs Reference to the second list.
///\return <b>true</b> if the first list is lexicographically greater than the
/// second, otherwise <b>false</b>.
//*************************************************************************
template <typename TKey, typename TMapped, typename TKeyCompare>
bool operator >(const etl::imultimap<TKey, TMapped, TKeyCompare>& lhs, const etl::imultimap<TKey, TMapped, TKeyCompare>& rhs)
{
  return (rhs < lhs);
}

//*************************************************************************
/// Less than or equal operator.
///\param lhs Reference to the first list.
///\param rhs Reference to the second list.
///\return <b>true</b> if the first list is lexicographically less than or equal
/// to the second, otherwise <b>false</b>.
//*************************************************************************
template <typename TKey, typename TMapped, typename TKeyCompare>
bool operator <=(const etl::imultimap<TKey, TMapped, TKeyCompare>& lhs, const etl::imultimap<TKey, TMapped, TKeyCompare>& rhs)
{
  return !(lhs > rhs);
}

//*************************************************************************
/// Greater than or equal operator.
///\param lhs Reference to the first list.
///\param rhs Reference to the second list.
///\return <b>true</b> if the first list is lexicographically greater than or
/// equal to the second, otherwise <b>false</b>.
//*************************************************************************
template <typename TKey, typename TMapped, typename TKeyCompare>
bool operator >=(const etl::imultimap<TKey, TMapped, TKeyCompare>& lhs, const etl::imultimap<TKey, TMapped, TKeyCompare>& rhs)
{
  return !(lhs < rhs);
}

#ifdef ETL_COMPILER_MICROSOFT
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#undef __ETL_IN_IMULTIMAP_H__

#endif
