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

#ifndef __NAMEDCONTAINER__
#define __NAMEDCONTAINER__
#include <vector>
#include <stdexcept>
#include <unordered_map>

#if defined(_MSC_VER)
#define MSFASTCALL __fastcall
#define GCCFASTCALL 
#elif defined(__GNUC__)
#define MSFASTCALL
#define GCCFASTCALL __attribute__((fastcall))
#endif

using std::vector;
using std::pair;
using std::make_pair;
using std::string;
using std::unordered_map;


namespace syn {

    template<typename T>
    class NamedContainer {
		typedef int key_t;
		unordered_map<key_t,T> m_data;
		vector<pair<string, key_t> > m_names;
		size_t m_size;
		key_t m_nextid;
    public:
		NamedContainer() :
			NamedContainer(16)
		{
		}

		explicit NamedContainer(size_t a_reservesize) :
			m_size(0),
			m_nextid(0)
		{
			m_data.reserve(a_reservesize);
		}

        virtual ~NamedContainer(){ m_names.clear(); m_data.clear(); }

        /**
         * Add a named item to the container.
         * \returns The items index in the container, or -1 if the item or name already exists.
         */
        int add(const string& a_name, const T& a_item);

		/**
		* Add a named item to the container using the requested id.
		* \returns True upon success, or false if the item or name already exists or the index is already in use.
		*/
		bool add(const string& a_name, int a_id, const T& a_item);

		const unordered_map<key_t, T>& data() const { return m_data; }

        /**
         * Remove an item from the container, either by its name, index, or a reference to the item itself.
         * \returns True upon successful removal, false if item does not exist.
         */
        template<typename ID>
        bool remove(const ID& a_itemID);

        /**
         * Retrieves an item from the container, either by its name, index, or a reference to the item itself.
         * \returns A reference to the item
         */
        template<typename ID>
        T& MSFASTCALL operator[](const ID& a_itemID) GCCFASTCALL;

		T& MSFASTCALL operator[](key_t a_itemID) GCCFASTCALL;

        template<typename ID>
        const T& MSFASTCALL operator[](const ID& a_itemID) const GCCFASTCALL;

		const T& MSFASTCALL operator[](key_t a_itemID) const GCCFASTCALL;

        /**
         * Verifies if an item is in the container, either by its name, index, or a reference to the item itself.
         * \returns True if the item exists, false otherwise.
         */
        template<typename ID>
        bool find(const ID& a_itemID) const;

        template<typename ID>
        string getItemName(const ID& a_itemID) const;

        size_t MSFASTCALL size() GCCFASTCALL const;

		int getItemIndex(key_t a_itemId) const { return m_data.find(a_itemId)!=m_data.end() ? a_itemId : -1; }

		int getItemIndex(const string& a_name) const;

		int getItemIndex(const T& a_item) const;

	    vector<int> getIds() const;

	    vector<string> getNames() const;
    private:
		key_t _getNextId();
    };

    template<typename T>
    int NamedContainer<T>::add(const string& a_name, const T& a_item){        
		int item_id = _getNextId();
		if (!add(a_name, item_id, a_item))
			return -1;
        return item_id;
    }

	template <typename T>
	bool NamedContainer<T>::add(const string& a_name, int a_id, const T& a_item) {
		if (find(a_name))
			return false;
		if (find(a_id))
			return false;
		m_data[a_id] = a_item;
		m_names.push_back(make_pair(a_name, a_id));
		m_size = m_data.size();
		return true;
    }

	template<typename T>
    template<typename ID>
    bool NamedContainer<T>::remove(const ID& a_itemID){
        int itemidx = getItemIndex(a_itemID);
        // Check if item was named and, if so, remove its name from the list
        for(unsigned i=0;i<m_names.size();i++){
            if(m_names[i].second==itemidx){
                m_names.erase(m_names.begin()+i);
				break;
            }
        }
        int nerased = m_data.erase(itemidx);
		m_size = m_data.size();
        return nerased==1;
    }

    template<typename T>
    template<typename ID>
    T& NamedContainer<T>::operator[](const ID& a_itemID) {
        int itemidx = getItemIndex(a_itemID);
        return m_data[itemidx];
    }

	template <typename T>
	T& NamedContainer<T>::operator[](key_t a_itemID)
    {
		return m_data[a_itemID];
    }

	template<typename T>
    template<typename ID>
    const T& NamedContainer<T>::operator[](const ID& a_itemID) const {
        int itemidx = getItemIndex(a_itemID);
        return m_data[itemidx];
    }

	template <typename T>
	const T& NamedContainer<T>::operator[](key_t a_itemID) const
    {
		return m_data.at(a_itemID);
    }

	template<typename T>
    template<typename ID>
    bool NamedContainer<T>::find(const ID& a_itemID) const{
        return (getItemIndex(a_itemID) >= 0);
    }

    template<typename T>
    size_t NamedContainer<T>::size() const{
        return m_size;
    }

    template<typename T>
    template<typename ID>
    string NamedContainer<T>::getItemName(const ID& a_itemID) const{
        int itemidx = getItemIndex(a_itemID);
		// Check if item was named and, if so, grab its name from the list
        for(int i=0;i<m_names.size();i++){
            if(m_names[i].second==itemidx){
                return m_names[i].first;
            }
        }
		throw std::domain_error("Item does not exist in NamedContainer");
    }

    template<typename T>
    int NamedContainer<T>::getItemIndex(const string& a_name) const{
		int nNames = m_names.size();
        for(int i=0;i<nNames;i++){
            if(m_names[i].first == a_name){
                return m_names[i].second;
            }
        }
        return -1;
    }

    template<typename T>
    int NamedContainer<T>::getItemIndex(const T& a_item) const{
		for(auto it = m_data.begin();it!=m_data.end(); ++it) {
			if (it->second == a_item)
				return it->first;
		}
        return -1;
    }

	template <typename T>
	vector<int> NamedContainer<T>::getIds() const {
		vector<int> ids(m_data.size());
		int i = 0;
		for (auto it = m_data.begin(); it != m_data.end(); ++it) {
			ids[i] = it->first;
			i++;
		}
		return ids;
	}

	template <typename T>
	vector<string> NamedContainer<T>::getNames() const {
		vector<string> names(m_names.size());
		for (int i = 0; i < m_names.size(); i++) {
			names[i] = m_names[i].first;
		}
		return names;
	}

	template <typename T>
	typename NamedContainer<T>::key_t NamedContainer<T>::_getNextId() {
		while (find(m_nextid))
			m_nextid++;
		return m_nextid;
    }
}


#endif //VOSIMLIB_NAMEDCONTAINER_H
