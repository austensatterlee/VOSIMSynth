/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "vosimlib/common.h"
#include <array>

using std::array;
using std::string;

namespace syn {
    /**
     * Pre-allocated container that allows fast access to items via an integer id or index.
     *
     * This container works best when storing a relatively small number of items that are looked up frequently
     * and need to be accessed by a persistent id. In such cases, it will often perform better than something
     * like std::map<int>.
     * 
     * Advantages:
     *  - Items can be assigned ids automatically upon insertion.
     *  - Removing an item does not change any other item's id.
     *  - Constant time lookup.
     *  - O(n log(n)) insertion and removal of items by id, where n is the maximum size of the container.
     *  - Pre-allocated storage.
     *
     * Disadvantages:
     *  - Maximum size must be determined at compile time.
     *  - Ids must be small (less than the maximum size of the container).
     *
     * Note that ids are often not contiguous. Indexes are always contiguous, but the item pointed to by any
     * given index may change. To iterate over the container, either use a range-based for loop, or use one of
     * the index access methods.
     *
     * \tparam T Type of item to be stored in the container.
     * \tparam MAXSIZE Maximum number of items that can be stored in the container.
     */
    template <typename T, int MAXSIZE>
    class VOSIMLIB_API IntMap {
        // Used to correctly add constness when the template argument is a pointer.
        template<typename value_type>
        struct add_const { typedef const value_type type; };

        template<typename value_type>
        struct add_const<value_type*> { typedef const value_type* const type; };

    public:
        static const size_t max_size = MAXSIZE;
        using item_type = T;
        using const_item_type = typename add_const<item_type>::type;
        using reference = item_type&;
        using const_reference = const T&;
        using pointer = item_type*;
        using const_pointer = const T*;

        typedef int id_type;

    protected:
        T m_data[MAXSIZE];
        array<bool, MAXSIZE> m_existances;
        array<int, MAXSIZE> m_ids;
        int m_size;

        template <typename _T>
        struct id_comparator {
            typedef _T first_argument_type;
            typedef _T second_argument_type;
            typedef bool result_type;

            constexpr bool operator()(const _T& _Left, const _T& _Right) const {
                return (_Left >= 0) && (_Right<0 || _Left < _Right);
            }
        };

    public:
        IntMap()
            :
            m_size(0) {
            m_existances.fill(false);
            m_ids.fill(-1);
        }

        class iterator : public std::iterator<std::bidirectional_iterator_tag, item_type> {
        public:

            iterator()
                : m_nc(nullptr),
                  m_item_num(0) { }

            iterator(IntMap& nc)
                : m_nc(&nc),
                  m_item_num(0) { }

            iterator(IntMap& nc, int offset)
                : m_nc(&nc),
                  m_item_num(offset) { }

            iterator(const iterator& other)
                : m_nc(other.m_nc),
                  m_item_num(other.m_item_num) { }

            ~iterator() { }

            iterator& operator ++() {
                m_item_num = m_nc->next_offset(m_item_num);
                return *this;
            }

            iterator operator ++(int) {
                iterator temp(*this);
                temp.m_item_num = m_nc->next_offset(m_item_num);
                return temp;
            }

            iterator& operator --() {
                m_item_num = m_nc->prev_offset(m_item_num);
                return *this;
            }

            iterator operator --(int) {
                iterator temp(*this);
                temp.m_item_num = m_nc->prev_offset(m_item_num);
                return temp;
            }

            iterator operator =(const iterator& other) {
                m_nc = other.m_nc;
                m_item_num = other.m_item_num;
                return *this;
            }

            reference operator *() {
                return m_nc->getByIndex(m_item_num);
            }

            const_reference operator *() const {
                return m_nc->getByIndex(m_item_num);
            }

            pointer operator &() {
                return &(m_nc->getByIndex(m_item_num));
            }

            const_pointer operator &() const {
                return &(m_nc->getByIndex(m_item_num));
            }

            pointer operator ->() {
                return &(m_nc->getByIndex(m_item_num));
            }

            const_pointer operator ->() const {
                return &(m_nc->getByIndex(m_item_num));
            }

            friend bool operator ==(const iterator& lhs, const iterator& rhs) {
                return lhs.m_nc == rhs.m_nc && lhs.m_item_num == rhs.m_item_num;
            }

            friend bool operator !=(const iterator& lhs, const iterator& rhs) {
                return !(lhs == rhs);
            }

        private:
            IntMap* m_nc;
            int m_item_num;
        };

        typedef typename std::iterator_traits<iterator>::difference_type difference_type;
        typedef std::reverse_iterator<iterator> reverse_iterator;

        /**
         * Increment the given index and check if it is out of bounds.
         * \return The incremented offset, or -1 if it is out of bounds.
         */
        static int next_offset(int a_offset) {
            a_offset++;
            return a_offset >= MAXSIZE ? -1 : a_offset;
        }

        /**
         * Decrement the given index and check if it is out of bounds.
         * \return The decremented offset, or -1 if it is out of bounds.
         */
        static int prev_offset(int a_offset) {
            a_offset--;
            return a_offset < 0 ? -1 : a_offset;
        }

        iterator begin() {
            return iterator(*this, 0);
        }

        iterator cbegin() const {
            return iterator(*this, 0);
        }

        iterator end() {
            return iterator(*this, m_size);
        }

        iterator cend() const {
            return iterator(*this, m_size);
        }

        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        reverse_iterator crbegin() const {
            return reverse_iterator(end());
        }

        reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        reverse_iterator crend() const {
            return reverse_iterator(begin());
        }

        /**
         * Add an item to the container.
         * \returns The items index in the container, or -1 if the item already exists, or if the container is full.
         */
        int add(const T& a_item);

        /**
        * Add an item to the container using the requested index.
        * \returns True upon success, or false if the item already exists or the index is already in use, or if the container is full.
        */
        bool add(int a_index, const T& a_item);

        const T* data() const {
            return m_data;
        }

        /**
         * Remove the specified item from the container.
         * \returns True upon successful removal, false if item does not exist.
         */    
        bool removeByItem(const_item_type& a_item) {            
            int id = _getIdFromItem(a_item);
            return _remove(id);
        }
          
        /**
         * Remove the item with the specified id from the container.
         * \returns True upon successful removal, false if item does not exist.
         */   
        bool removeById(const id_type& a_id) {
            return _remove(a_id);
        }

        /**
         * Retrieve the item with the specified id.
         * \returns A reference to the item
         */
        T& operator[](int a_id) ;
        const T& operator[](int a_id) const;
        
        /**
         * Retrieve the item at the specified index.
         * \returns A reference to the item
         */
        T& getByIndex(int a_index) ;
        const T& getByIndex(int a_index) const;

        /**
         * Find the id of the specified item.
         * \returns -1 if the item does not exist
         */
        id_type getIdFromItem(const_item_type& a_item) const {
            return _getIdFromItem(a_item);
        }

        /**
         * Find the id of the item at the specified index. 
         * 
         * For example, getIdFromIndex(0) returns the first id, getIdFromIndex(1) returns the second id, etc.
         * 
         * \returns -1 if the item does not exist
         */
        id_type getIdFromIndex(int a_index) const {
            if(a_index<0 || a_index>=m_size)
                return -1;
            return m_ids[a_index];
        }

        int getIndexFromId(id_type a_id) const;
        int getIndexFromItem(const_item_type& a_item) const;

        bool containsItem(const_item_type& a_item) const { return _getIdFromItem(a_item)>=0; }
        bool containsId(id_type a_id) const { return _checkId(a_id)>=0; }

        int size() const;

        bool empty() const;

        /**
         * Retrieve a list of all the ids stored in the container. The size of the list is the same as the
         * value returned by size().
         */
        const id_type* ids() const;

        id_type getUnusedId() const;

    protected:
        int _checkId(int a_id) const;

        int _getIdFromItem(const_item_type& a_item) const;

        bool _remove(int a_index);
    };

    template <typename T, int MAXSIZE>
    int IntMap<T, MAXSIZE>::add(const T& a_item) {
        int item_id = getUnusedId();
        if (!add(item_id, a_item))
            return -1;
        return item_id;
    }

    template <typename T, int MAXSIZE>
    bool IntMap<T, MAXSIZE>::add(int a_id, const T& a_item) {
        if (a_id >= MAXSIZE)
            return false;
        if (_checkId(a_id)>=0)
            return false;
        m_data[a_id] = a_item;
        m_existances[a_id] = true;

        // update index list
        m_ids[m_size] = a_id;
        std::sort(std::begin(m_ids), std::end(m_ids), id_comparator<int>());

        m_size++;
        return true;
    }

    template <typename T, int MAXSIZE>
    T& IntMap<T, MAXSIZE>::operator[](int a_id) 
    {
        return m_data[a_id];
    }

    template <typename T, int MAXSIZE>
    const T& IntMap<T, MAXSIZE>::operator[](int a_id) const 
    {
        return m_data[a_id];
    }

    template <typename T, int MAXSIZE>
    T& IntMap<T, MAXSIZE>::getByIndex(int a_index) {
        return m_data[m_ids[a_index]];
    }

    template <typename T, int MAXSIZE>
    const T& IntMap<T, MAXSIZE>::getByIndex(int a_index) const {
        return m_data[m_ids[a_index]];
    }

    template <typename T, int MAXSIZE>
    int IntMap<T, MAXSIZE>::getIndexFromId(id_type a_id) const {
        auto loc = std::find(m_ids.begin(), m_ids.end(), a_id);
        if (loc != m_ids.end())
            return loc - m_ids.begin();
        return -1;
    }

    template <typename T, int MAXSIZE>
    int IntMap<T, MAXSIZE>::getIndexFromItem(const_item_type& a_item) const {
        for (int i = 0; i < m_size; i++) {
            if (m_data[m_ids[i]] == a_item)
                return i;
        }
        return -1;
    }

    template <typename T, int MAXSIZE>
    int IntMap<T, MAXSIZE>::size() const { return m_size; }

    template <typename T, int MAXSIZE>
    bool IntMap<T, MAXSIZE>::empty() const { return !m_size; }

    template <typename T, int MAXSIZE>
    const typename IntMap<T, MAXSIZE>::id_type* IntMap<T, MAXSIZE>::ids() const { return m_ids.data(); }

    template <typename T, int MAXSIZE>
    int IntMap<T, MAXSIZE>::_checkId(int a_id) const {
        return a_id>=0 && m_existances[a_id] ? a_id : -1;
    }

    template <typename T, int MAXSIZE>
    int IntMap<T, MAXSIZE>::_getIdFromItem(const_item_type& a_item) const {
        for (int i = 0; i < m_size; i++) {
            if (m_data[m_ids[i]] == a_item)
                return m_ids[i];
        }
        return -1;
    }

    template <typename T, int MAXSIZE>
    bool IntMap<T, MAXSIZE>::_remove(int a_index) {
        if (a_index == -1)
            return false;
        m_existances[a_index] = false;

        // update index list
        for (int i = 0; i < m_size; i++) {
            if (m_ids[i] == a_index) {
                for (int j = i; j < m_size - 1; j++) {
                    m_ids[j] = m_ids[j + 1];
                }
                m_ids[m_size - 1] = -1;
                break;
            }
        }
        std::sort(std::begin(m_ids), std::end(m_ids), id_comparator<int>());

        m_size--;
        return true;
    }

    template <typename T, int MAXSIZE>
    typename IntMap<T, MAXSIZE>::id_type IntMap<T, MAXSIZE>::getUnusedId() const {
        int nextId = 0;
        while (m_existances[nextId]) {
            nextId++;
            // container is full
            if (nextId >= MAXSIZE)
                return -1;
        }
        return nextId;
    }
}
