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
#include <array>
#include <functional>
#include <cereal/cereal.hpp>

#if defined(_MSC_VER)
#define MSFASTCALL __fastcall
#define GCCFASTCALL
#elif defined(__GNUC__)
#define MSFASTCALL
#define GCCFASTCALL __attribute__((fastcall))
#endif

using std::array;
using std::string;

namespace syn
{
	template <typename T, int MAXSIZE>
	class NamedContainer
	{
		array<T,MAXSIZE> m_data;
		array<bool,MAXSIZE> m_existances;
		array<int, MAXSIZE> m_indices;
		array<string, MAXSIZE> m_names;
		size_t m_size;

		struct SerializableItem
		{
			int index;
			string name;
			T item;
			template <class Archive>
			void serialize(Archive& ar) {
				ar(cereal::make_nvp("index", index),
				   cereal::make_nvp("name", name),
				   cereal::make_nvp("item", item));
			}
		};

	public:
		NamedContainer() :
			m_size(0)
		{
			m_existances.fill(false);
			m_indices.fill(-1);
		}

		/**
		 * Add a named item to the container.
		 * \returns The items index in the container, or -1 if the item or name already exists, or if the container is full.
		 */
		int add(const string& a_name, const T& a_item);

		/**
		* Add a named item to the container using the requested index.
		* \returns True upon success, or false if the item or name already exists or the index is already in use, or if the container is full.
		*/
		bool add(const string& a_name, int a_index, const T& a_item);

		const T* data() const {
			return m_data.data();
		}

		/**
		 * Remove an item from the container, either by its name, index, or a reference to the item itself.
		 * \returns True upon successful removal, false if item does not exist.
		 */
		template <typename IDType>
		bool remove(const IDType& a_itemId);

		/**
		 * Retrieves an item from the container by name.
		 * \returns A reference to the item
		 */
		T& MSFASTCALL operator[](const string& a_itemName) GCCFASTCALL;
		const T& MSFASTCALL operator[](const string& a_itemName) GCCFASTCALL const;

		/**
		* Retrieves an item from the container by index.
		* \returns A reference to the item
		*/
		T& MSFASTCALL operator[](int a_itemIndex) GCCFASTCALL;
		const T& MSFASTCALL operator[](int a_itemIndex) GCCFASTCALL const;

		/**
		 * Verifies if an item is in the container, either by its name, index, or a reference to the item itself.
		 * \returns True if the item exists, false otherwise.
		 */
		template <typename IDType>
		bool contains(const IDType& a_itemId) const;

		template <typename IDType>
		int find(const IDType& a_itemId) const;

		template <typename IDType>
		const string& getItemName(const IDType& a_itemId) const;

		size_t size() const;

		const int* getIndices() const;

		const string* getNames() const;

		template<typename Archive>
		void save(Archive& ar) const {
			cereal::size_type size = m_size;
			ar(cereal::make_size_tag(size));
			for(int i=0;i<m_size;i++) {
				SerializableItem item{ m_indices[i], m_names[m_indices[i]], m_data[m_indices[i]] };
				ar(item);
			}
		}

		template<typename Archive>
		void load(Archive& ar) {
			cereal::size_type size;
			ar(cereal::make_size_tag(size));
			for(int i=0;i<size;i++) {
				SerializableItem item;
				ar(item);
				add(item.name, item.index, item.item);
			}
		}

	private:
		int getItemIndex(int a_itemId) const;

		int getItemIndex(const string& a_name) const;

		int getItemIndex(const T& a_item) const;

		int _getNextId();
	};

	template <typename T, int MAXSIZE>
	int NamedContainer<T, MAXSIZE>::add(const string& a_name, const T& a_item) {
		int item_id = _getNextId();
		if (!add(a_name, item_id, a_item))
			return -1;
		return item_id;
	}

	template <typename T, int MAXSIZE>
	bool NamedContainer<T, MAXSIZE>::add(const string& a_name, int a_index, const T& a_item) {
		if (a_index >= MAXSIZE)
			return false;
		if (contains(a_name))
			return false;
		if (contains(a_index))
			return false;
		m_data[a_index] = a_item;
		m_existances[a_index] = true;
		m_names[a_index] = a_name;

		// update index list
		m_indices[m_size] = a_index;
		std::sort(m_indices.begin(), m_indices.end(), std::greater<int>());

		m_size++;
		return true;
	}

	template <typename T, int MAXSIZE>
	template <typename IDType>
	bool NamedContainer<T, MAXSIZE>::remove(const IDType& a_itemID) {
		int index = getItemIndex(a_itemID);
		if (index == -1)
			return false;
		m_existances[index] = false;

		// update index list
		for(int i=0;i<m_size;i++) {
			if (m_indices[i] == index) {
				for(int j=i;j<m_size-1;j++) {
					m_indices[j] = m_indices[j+1];
				}
				m_indices[m_size - 1] = -1;
				break;
			}
		}
		std::sort(m_indices.begin(), m_indices.end(), std::greater<int>());

		m_size--;
		return true;
	}

	template <typename T, int MAXSIZE>
	T& NamedContainer<T, MAXSIZE>::operator[](const string& a_itemName) {
		int itemidx = getItemIndex(a_itemName);
		return m_data[itemidx];
	}

	template <typename T, int MAXSIZE>
	const T& NamedContainer<T, MAXSIZE>::operator[](const string& a_itemName) const {
		int itemidx = getItemIndex(a_itemName);
		return m_data[itemidx];
	}

	template <typename T, int MAXSIZE>
	T& NamedContainer<T, MAXSIZE>::operator[](int a_itemIndex) {
		return m_data[a_itemIndex];
	}
	
	template <typename T, int MAXSIZE>
	const T& NamedContainer<T, MAXSIZE>::operator[](int a_itemIndex) const {
		return m_data[a_itemIndex];
	}

	template <typename T, int MAXSIZE>
	template <typename IDType>
	bool NamedContainer<T, MAXSIZE>::contains(const IDType& a_itemID) const {
		return (getItemIndex(a_itemID) >= 0);
	}

	template <typename T, int MAXSIZE>
	template <typename IDType>
	int NamedContainer<T, MAXSIZE>::find(const IDType& a_itemId) const {
		return getItemIndex(a_itemId);
	}

	template <typename T, int MAXSIZE>
	size_t NamedContainer<T, MAXSIZE>::size() const {
		return m_size;
	}

	template <typename T, int MAXSIZE>
	const int* NamedContainer<T, MAXSIZE>::getIndices() const {
		return m_indices.data();
	}

	template <typename T, int MAXSIZE>
	template <typename IDType>
	const string& NamedContainer<T, MAXSIZE>::getItemName(const IDType& a_itemID) const {
		int itemidx = getItemIndex(a_itemID);
		return m_names[itemidx];
	}

	template <typename T, int MAXSIZE>
	int NamedContainer<T, MAXSIZE>::getItemIndex(int a_itemIndex) const {
		return m_existances[a_itemIndex] ? a_itemIndex : -1;
	}

	template <typename T, int MAXSIZE>
	int NamedContainer<T, MAXSIZE>::getItemIndex(const string& a_name) const {
		for (int i = 0; i < m_size; i++) {
			if (m_existances[i] && m_names[i] == a_name) 
				return i;			
		}
		return -1;
	}

	template <typename T, int MAXSIZE>
	int NamedContainer<T, MAXSIZE>::getItemIndex(const T& a_item) const {
		for (int i = 0; i < m_size; i++) {
			if (m_existances[i] && m_data[i] == a_item)
				return i;
		}
		return -1;
	}

	template <typename T, int MAXSIZE>
	string const* NamedContainer<T, MAXSIZE>::getNames() const {
		return m_names.data();
	}

	template <typename T, int MAXSIZE>
	int NamedContainer<T, MAXSIZE>::_getNextId() {
		int nextId = 0;
		while (m_existances[nextId]) 
			nextId++;
		return nextId;
	}
}

#endif //VOSIMLIB_NAMEDCONTAINER_H
