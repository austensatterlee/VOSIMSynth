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
#include "vosimlib/IntMap.h"

namespace syn {

    template <typename T, int MAXSIZE>
    class VOSIMLIB_API StrMap : public IntMap<T, MAXSIZE> {
        array<std::string, MAXSIZE> m_names;
    public:

        StrMap()
            : IntMap<T, MAXSIZE>() {}        

        using IntMap<T, MAXSIZE>::operator[];

        /**
         * Retrieve a list of the names stored in the container. The size of the list is the same as the value
         * returned by size().
         */
        string const* names() const {
            return m_names.data();
        }

        /**
         * Retrieve an item by name.
         * \returns A reference to the item
         */
        T& operator[](const string& a_name) ;
        const T& operator[](const string& a_name) const;

        /**
         * Add a named item to the container.
         * \returns The items index in the container, or -1 if the item or name already exists, or if the container is full.
         */
        int add(const string& a_name, const T& a_item);

        /**
        * Add a named item to the container using the requested index.
        * \returns True upon success, or false if the item or name already exists or the index is already in use, or if the container is full.
        */
        bool add(const string& a_name, int a_id, const T& a_item);

        int getIndexFromName(const string& a_name) const {
            for (int i = 0; i < IntMap<T, MAXSIZE>::m_size; i++) {
                if (m_names[IntMap<T, MAXSIZE>::m_ids[i]] == a_name)
                    return i;
            }
            return -1;
        }

        /**
         * Remove the item with the specified name from the container.
         * \returns True upon successful removal, false if item does not exist.
         */
        bool removeByName(const string& a_name) {
            int id = _getIdFromName(a_name);
            return IntMap<T, MAXSIZE>::_remove(id);
        }

        bool containsName(const string& a_name) const { return _getIdFromName(a_name) >= 0; }

        /**
         * Get the name of the item with the specified id.
         */
        const string& getNameFromId(const typename IntMap<T, MAXSIZE>::id_type& a_id) const {
            return m_names[a_id];
        }

        /**
         * Get the name of the specified item.
         */
        const string& getNameFromItem(typename IntMap<T, MAXSIZE>::const_item_type& a_item) const {
            int id = _getIdFromItem(a_item);
            return m_names[id];
        }

        /**
         * Find the id of the item with the specified name.
         * \returns -1 if the item does not exist
         */
        typename IntMap<T, MAXSIZE>::id_type getIdFromName(const string& a_name) const {
            return _getIdFromName(a_name);
        }

    protected:
        int _getIdFromName(const string& a_name) const {
            for (int i = 0; i < IntMap<T, MAXSIZE>::m_size; i++) {
                if (m_names[IntMap<T, MAXSIZE>::m_ids[i]] == a_name)
                    return IntMap<T, MAXSIZE>::m_ids[i];
            }
            return -1;
        }
   
        int add(const T& a_item) { return IntMap<T, MAXSIZE>::add(a_item); }

        bool add(int a_index, const T& a_item) { return IntMap<T, MAXSIZE>::add(a_index, a_item); }
    };

    template <typename T, int MAXSIZE>
    T& StrMap<T, MAXSIZE>::operator[](const string& a_name) {
        int id = _getIdFromName(a_name);
        return IntMap<T, MAXSIZE>::m_data[id];
    }

    template <typename T, int MAXSIZE>
    const T& StrMap<T, MAXSIZE>::operator[](const string& a_name) const {
        int id = _getIdFromName(a_name);
        return IntMap<T, MAXSIZE>::m_data[id];
    }

    template <typename T, int MAXSIZE>
    int StrMap<T, MAXSIZE>::add(const string& a_name, const T& a_item) {
        if(_getIdFromName(a_name)>=0)
            return -1;
        int id = add(a_item);
        if(id>=0)
            m_names[id] = a_name;
        return id;
    }

    template <typename T, int MAXSIZE>
    bool StrMap<T, MAXSIZE>::add(const string& a_name, int a_id, const T& a_item) {
        if(_getIdFromName(a_name)>=0)
            return false;
        bool success = add(a_id, a_item);
        if(success)
            m_names[a_id] = a_name;
        return success;
    }
}