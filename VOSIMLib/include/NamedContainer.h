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

namespace syn
{
	template <typename T>
	class NamedContainer
	{
		vector<T> m_data;
		vector<int> m_ids;
		vector<int> m_indices; /// Maps IDs to indices (a value of -1 at position 'k' means the ID 'k' is not in the container)
		vector<pair<string, int>> m_names;
		size_t m_size;
		int m_nextid;
	public:
		NamedContainer() :
			NamedContainer(16) { }

		explicit NamedContainer(size_t a_reservesize) :
			m_size(0),
			m_nextid(0) {
			m_data.reserve(a_reservesize);
			m_ids.reserve(a_reservesize);
			m_names.reserve(a_reservesize);
			m_indices.reserve(a_reservesize);
		}

		virtual ~NamedContainer() {
		}

		/**
		 * Add a named item to the container.
		 * \returns The items index in the container, or -1 if the item or name already exists.
		 */
		int add(const string& a_name, const T& a_item);

		/**
		* Add a named item to the container using the requested id.
		* Don't use exceedingly high IDs. The memory usage of this object is proportional to the highest ID.
		* \returns True upon success, or false if the item or name already exists or the index is already in use.
		*/
		bool add(const string& a_name, int a_id, const T& a_item);

		const vector<T>& data() const {
			return m_data;
		}

		/**
		 * Remove an item from the container, either by its name, index, or a reference to the item itself.
		 * \returns True upon successful removal, false if item does not exist.
		 */
		template <typename ID>
		bool remove(const ID& a_itemID);

		/**
		 * Retrieves an item from the container, either by its name, index, or a reference to the item itself.
		 * \returns A reference to the item
		 */
		template <typename ID>
		T& MSFASTCALL operator[](const ID& a_itemID) GCCFASTCALL;

		T& MSFASTCALL operator[](const int& a_itemID) GCCFASTCALL;

		T& MSFASTCALL getItemByIndex(int a_itemIdx) GCCFASTCALL;

		template <typename ID>
		const T& MSFASTCALL operator[](const ID& a_itemID) const GCCFASTCALL;

		const T& MSFASTCALL getItemByIndex(int a_itemIdx) const GCCFASTCALL;

		/**
		 * Verifies if an item is in the container, either by its name, index, or a reference to the item itself.
		 * \returns True if the item exists, false otherwise.
		 */
		template <typename ID>
		bool find(const ID& a_itemID) const;

		template <typename ID>
		string getItemName(const ID& a_itemID) const;

		template <typename ID>
		int getItemId(const ID& a_itemID) const;

		size_t MSFASTCALL size() GCCFASTCALL const;

		const vector<int>& getIds() const;

		vector<string> getNames() const;

	private:
		int getItemIndex(int a_itemId) const;

		int getItemIndex(const string& a_name) const;

		int getItemIndex(const T& a_item) const;

		int _getNextId();
	};

	template <typename T>
	int NamedContainer<T>::add(const string& a_name, const T& a_item) {
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
		m_data.push_back(a_item);
		m_ids.push_back(a_id);
		m_names.push_back(make_pair(a_name, a_id));
		if (a_id >= m_indices.size())
			m_indices.resize(a_id + 1, -1);
		m_indices[a_id] = m_data.size() - 1;
		m_size = m_data.size();
		return true;
	}

	template <typename T>
	template <typename ID>
	bool NamedContainer<T>::remove(const ID& a_itemID) {
		int itemidx = getItemIndex(a_itemID);
		if (itemidx == -1)
			return false;
		int itemid = m_ids[itemidx];
		// Check if item was named and, if so, remove its name from the list
		for (unsigned i = 0; i < m_names.size(); i++) {
			if (m_names[i].second == itemid) {
				m_names.erase(m_names.begin() + i);
				break;
			}
		}
		m_data.erase(m_data.begin() + itemidx);
		m_ids.erase(m_ids.begin() + itemidx);
		// Update the index map for elements that were moved
		for (unsigned i = itemidx; i < m_data.size(); i++) {
			int tmpid = m_ids[i];
			m_indices[tmpid]--;
		}
		m_size = m_data.size();
		m_indices[itemid] = -1;
		m_nextid = itemid;
		return true;
	}

	template <typename T>
	template <typename ID>
	T& NamedContainer<T>::operator[](const ID& a_itemID) {
		int itemidx = getItemIndex(a_itemID);
		return m_data[itemidx];
	}

	template <typename T>
	T& NamedContainer<T>::operator[](const int& a_itemID) {
		int itemidx = getItemIndex(a_itemID);
		return m_data[itemidx];
	}

	template <typename T>
	T& NamedContainer<T>::getItemByIndex(int a_itemIdx) {
		return m_data[a_itemIdx];
	}

	template <typename T>
	template <typename ID>
	const T& NamedContainer<T>::operator[](const ID& a_itemID) const {
		int itemidx = getItemIndex(a_itemID);
		return m_data[itemidx];
	}

	template <typename T>
	const T& NamedContainer<T>::getItemByIndex(int a_itemIdx) const {
		return m_data[a_itemIdx];
	}

	template <typename T>
	template <typename ID>
	bool NamedContainer<T>::find(const ID& a_itemID) const {
		return (getItemIndex(a_itemID) >= 0);
	}

	template <typename T>
	size_t NamedContainer<T>::size() const {
		return m_size;
	}

	template <typename T>
	int NamedContainer<T>::getItemIndex(int a_itemId) const {
		return a_itemId >= m_indices.size() ? -1 : m_indices[a_itemId];
	}

	template <typename T>
	template <typename ID>
	string NamedContainer<T>::getItemName(const ID& a_itemID) const {
		int itemidx = getItemIndex(a_itemID);
		int itemid = m_ids[itemidx];
		// Check if item was named and, if so, grab its name from the list
		for (int i = 0; i < m_names.size(); i++) {
			if (m_names[i].second == itemid) {
				return m_names[i].first;
			}
		}
		throw std::domain_error("Item does not exist in NamedContainer");
	}

	template <typename T>
	template <typename ID>
	int NamedContainer<T>::getItemId(const ID& a_itemID) const {
		return m_ids[getItemIndex(a_itemID)];
	}

	template <typename T>
	int NamedContainer<T>::getItemIndex(const string& a_name) const {
		int nNames = m_names.size();
		for (int i = 0; i < nNames; i++) {
			if (m_names[i].first == a_name) {
				return getItemIndex(m_names[i].second);
			}
		}
		return -1;
	}

	template <typename T>
	int NamedContainer<T>::getItemIndex(const T& a_item) const {
		for (int i = 0; i < m_data.size(); i++) {
			if (m_data[i] == a_item)
				return i;
		}
		return -1;
	}

	template <typename T>
	const vector<int>& NamedContainer<T>::getIds() const {
		return m_ids;
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
	int NamedContainer<T>::_getNextId() {
		m_nextid = 0;
		while (find(m_nextid))
			m_nextid++;
		return m_nextid;
	}
}

#endif //VOSIMLIB_NAMEDCONTAINER_H
