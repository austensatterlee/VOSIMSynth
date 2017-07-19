/*
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
along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2016, Austen Satterlee
*/

#pragma once

#include "vosimsynth/Signal.h"
#include <iostream>

namespace synui {
	// A Property encapsulates a value and may inform
	// you on any changes applied to this value.

	template <typename T>
	class Property {

	public:
		typedef T value_type;

		// Properties for built-in types are automatically
		// initialized to 0. See template specializations
		// at the bottom of this file
		Property() {}

		Property(const T& val)
			: value_(val) {}

		Property(T&& val)
			: value_(std::move(val)) {}

		Property(const Property<T>& to_copy)
			: value_(to_copy.value_) {}

		Property(Property<T>&& to_copy) noexcept
		    : value_(std::move(to_copy.value_)) {}

		// returns a Signal which is fired when the internal value
		// has been changed. The new value is passed as parameter.
		const Signal<T>& on_change() const {
			return onChange;
		}

		// sets the Property to a new value.
		// on_change() will be emitted.
		void set(const T& value) {
			if (value != value_) {
				value_ = value;
                onChange.emit(value_);
			}
		}

		// returns the internal value
		const T& get() const { return value_; }

		// if there are any Properties connected to this Property,
		// they won't be notified of any further changes
		void disconnect_auditors() {
            onChange.disconnect_all();
		}

		// returns the value of this Property
		const T& operator()() const {
			return Property<T>::get();
		}

	public:
		Signal<T> onChange;

	private:
		T value_;
	};

	// specialization for built-in default contructors
	template<> inline Property<double>::Property()
        : value_(0.0) {}

	template<> inline Property<float>::Property()
		: value_(0.f) {}

	template<> inline Property<short>::Property()
		: value_(0) {}

	template<> inline Property<int>::Property()
		: value_(0) {}

	template<> inline Property<char>::Property()
		: value_(0) {}

	template<> inline Property<unsigned>::Property()
		: value_(0) {}

	template<> inline Property<bool>::Property()
		: value_(false) {}

	// stream operators
	template<typename T>
	std::ostream& operator<<(std::ostream& str, const Property<T>& val) {
		str << val.get();
		return str;
	}

	template<typename T>
	std::istream& operator>>(std::istream& str, Property<T>& val) {
		T tmp;
		str >> tmp;
		val.set(tmp);
		return str;
	}
}