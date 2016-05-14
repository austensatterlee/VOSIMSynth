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

#ifndef __ATOMICCONTAINERS__
#define __ATOMICCONTAINERS__

#include <mutex>
#include <atomic>
#include <stdexcept>

namespace syn
{
	/**
	* Rounds the given uint32 to the next power of 2.
	* If doing so would cause overflow, the highest power of 2 is returned instead.
	*/
	inline unsigned next_power_of_2(unsigned x) {
		if (x == 0xFFFFFFFF)
			return 0x10000000;
		x--;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		x++;
		return x;
	}

	/**
	 * Atomic single-reader single-writer ring buffer.
	 * Allows constant time access to any element, and constant time popping and pushing to either side.
	 * Allows linear time removal of any element, and linear time searching for an element.
	 * Sizes are limited to powers of two, up to 128.
	 */
	template <typename T>
	class AtomicQueue
	{
	public:
		AtomicQueue();
		explicit AtomicQueue(unsigned a_maxsize);
		virtual ~AtomicQueue();

		bool push_left(const T& a_item);
		bool push_right(const T& a_item);

		bool pop_left(T& a_item);
		bool pop_right(T& a_item);
		bool pop(T& a_item, unsigned a_offset);

		bool peek_left(T& a_item) const;
		bool peek_right(T& a_item) const;
		bool peek(T& a_item, unsigned a_offset = 0) const;

		int find(const T& a_item) const;
		bool remove(const T& a_item);

		bool empty() const;

		const T* getData() const {
			return m_data;
		}

		unsigned max_size() const {
			unsigned size = m_maxsize.load(std::memory_order_seq_cst);
			return size + 1;
		}

		/**
		 * Resizes the maximum capacity of the queue. Should not be called during concurrent execution.
		 */
		bool unsafe_resize(unsigned a_maxsize);
	private:
		typedef uint8_t qindex_t;
		typedef uint8_t qsize_t;
		std::atomic<qsize_t> m_maxsize;
		std::atomic<qindex_t> m_left_index; /// Elements exit via the front of the queue
		std::atomic<qindex_t> m_right_index; /// Elements enter via the back of the queue
		T* m_data;
		std::mutex m_resizeMutex;
	};

	template <typename T>
	AtomicQueue<T>::AtomicQueue() :
		AtomicQueue(2) {}

	template <typename T>
	AtomicQueue<T>::AtomicQueue(unsigned a_maxsize):
		m_left_index(0),
		m_right_index(0),
		m_data(nullptr) {
		unsafe_resize(a_maxsize);
	}

	template <typename T>
	AtomicQueue<T>::~AtomicQueue() {

		if (m_data) {
			free(m_data);
		}
		m_data = nullptr;
	}

	template <typename T>
	bool AtomicQueue<T>::push_left(const T& a_item) {
		// do nothing if full
		unsigned oldright = m_right_index.load(std::memory_order_seq_cst);
		unsigned oldleft = m_left_index.load(std::memory_order_seq_cst);
		// check if adding a new a_item will cause the right and exit to meet.
		// we can wrap around m_maxsize like this because we are guaranteed it is one less than a power of 2 (e.g. 0xFF=0x100-0x01).
		unsigned newleft = (oldleft - 1) & m_maxsize;
		if (newleft == oldright)
			return false;
		// store value
		m_data[newleft] = a_item;
		// store new right
		m_left_index.store(newleft, std::memory_order_seq_cst);
		return true;
	}

	template <typename T>
	bool AtomicQueue<T>::push_right(const T& a_item) {
		// do nothing if full
		unsigned oldright = m_right_index.load(std::memory_order_seq_cst);
		unsigned oldleft = m_left_index.load(std::memory_order_seq_cst);
		// check if adding a new a_item will cause the right and exit to meet.
		unsigned newright = (oldright + 1) & m_maxsize;
		if (newright == oldleft)
			return false;
		// store value
		m_data[oldright] = a_item;
		// store new right
		m_right_index.store(newright, std::memory_order_seq_cst);
		return true;
	}

	template <typename T>
	bool AtomicQueue<T>::pop_left(T& a_item) {
		// do nothing if empty
		unsigned oldright = m_right_index.load(std::memory_order_seq_cst);
		unsigned oldleft = m_left_index.load(std::memory_order_seq_cst);
		if (oldleft == oldright)
			return false;
		// retrieve value
		a_item = m_data[oldleft];
		// store new exit
		m_left_index.store((oldleft + 1) & m_maxsize, std::memory_order_seq_cst);
		return true;
	}

	template <typename T>
	bool AtomicQueue<T>::pop_right(T& a_item) {
		// do nothing if empty
		unsigned oldright = m_right_index.load(std::memory_order_seq_cst);
		unsigned oldleft = m_left_index.load(std::memory_order_seq_cst);
		if (oldleft == oldright)
			return false;
		unsigned newright = (oldright - 1) & m_maxsize;
		// retrieve value
		a_item = m_data[newright];
		// store new exit
		m_right_index.store(newright, std::memory_order_seq_cst);
		return true;
	}

	template <typename T>
	bool AtomicQueue<T>::pop(T& a_item, unsigned a_offset) {
		AtomicQueue<T> tmpq(a_offset);
		T tmpblock;
		bool success = true;
		for (int i = 0; i < a_offset - 1; i++) {
			if (!pop_left(tmpblock)) {
				success = false;
				break;
			}
			tmpq.push_right(tmpblock);
		}
		if (success) {
			pop_left(a_item);
		}
		while (tmpq.pop_left(tmpblock)) {
			push_right(tmpblock);
		}
		return success;
	}

	template <typename T>
	bool AtomicQueue<T>::peek_left(T& a_item) const {
		return peek(a_item, 0);
	}


	template <typename T>
	bool AtomicQueue<T>::peek_right(T& a_item) const {
		unsigned right = m_right_index.load(std::memory_order_seq_cst);
		unsigned left = m_left_index.load(std::memory_order_seq_cst);
		unsigned offset;
		if (right > left) {
			offset = right - 1 - left;
		} else if (right < left) {
			unsigned size = m_maxsize.load(std::memory_order_seq_cst);
			offset = right + size - left;
		} else {
			return false;
		}
		return peek(a_item, offset);
	}

	template <typename T>
	bool AtomicQueue<T>::peek(T& a_item, unsigned a_offset) const {
		unsigned right = m_right_index.load(std::memory_order_seq_cst);
		unsigned left = m_left_index.load(std::memory_order_seq_cst);
		a_offset = (a_offset + left) & m_maxsize;
		// ensure the offset refers to a non-empty block
		if (right > left && (a_offset >= left && a_offset < right)) {
			a_item = m_data[a_offset];
			return true;
		}
		if (right < left && (a_offset >= left || a_offset < right)) {
			a_item = m_data[a_offset];
			return true;
		}
		return false;
	}

	template <typename T>
	bool AtomicQueue<T>::empty() const {
		unsigned right = m_right_index.load(std::memory_order_seq_cst);
		unsigned left = m_left_index.load(std::memory_order_seq_cst);
		return right == left;
	}

	template <typename T>
	int AtomicQueue<T>::find(const T& a_item) const {
		T tmp;
		unsigned i = 0;
		while (peek(tmp, i)) {
			if (tmp == a_item)
				return i;
		}
		return -1;
	}

	template <typename T>
	bool AtomicQueue<T>::remove(const T& a_item) {
		AtomicQueue<T> tmpq(m_maxsize);
		T tmpblock;
		bool success = false;
		// "uncover" requested item by temporarily storing the data above it on another queue
		while (pop_left(tmpblock)) {
			if (tmpblock == a_item) {
				success = true;
				break;
			}
			tmpq.push_right(tmpblock);
		}
		// Restore original state (sans the removed block, if successful)
		while (tmpq.pop_left(tmpblock)) {
			push_left(tmpblock);
		}
		return success;
	}

	template <typename T>
	bool AtomicQueue<T>::unsafe_resize(unsigned a_maxsize) {
		std::lock_guard<std::mutex> lock(m_resizeMutex);
		if (!a_maxsize)
			return false;

		a_maxsize = next_power_of_2(a_maxsize + 1);
		a_maxsize = a_maxsize >= 256 ? 256 : a_maxsize;
		uint8_t newsize = static_cast<uint8_t>(a_maxsize);
		m_maxsize.store(newsize - 1, std::memory_order_seq_cst);

		if (!m_data) {
			m_data = static_cast<T*>(calloc(newsize, sizeof(T)));
		} else {
			m_data = static_cast<T*>(realloc(m_data, newsize * sizeof(T)));
		}
		if (!m_data) {
			throw std::runtime_error("Unable to allocate memory for AtomicQueue");
		}
		return true;
	}
}
#endif
