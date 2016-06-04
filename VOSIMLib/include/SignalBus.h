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

#ifndef __SIGNALBUS__
#define __SIGNALBUS__

#include "NamedContainer.h"
#include <vector>
using std::vector;
using std::string;

namespace syn
{
	class Signal
	{
	public:
		/// Type of accumulation to perform on signal
		enum ChannelAccType
		{
			EAdd = 0,
			EMul = 1
		};

	public:
		Signal();
		bool operator==(const Signal& a_rhs) const;

		double MSFASTCALL get() const GCCFASTCALL;
		void MSFASTCALL accumulate(const Signal& a_sig) GCCFASTCALL;
		void MSFASTCALL set(const Signal& a_newVal) GCCFASTCALL;
		void MSFASTCALL set(const double& a_newVal) GCCFASTCALL;
		void setDefault(double a_default);
		void setChannelAccType(ChannelAccType a_accType);

		void MSFASTCALL clear() GCCFASTCALL;
	private:
		double m_value;
		double m_default;
		ChannelAccType m_accType;
	};

	class SignalBus
	{
	public:
		SignalBus()
			: m_numChannels(0) {}

		int addChannel();
		int addChannel(const string& a_name);
		int MSFASTCALL getNumChannels() const GCCFASTCALL;

		void MSFASTCALL set(SignalBus& a_other) GCCFASTCALL;

		template <typename ID>
		double MSFASTCALL getValue(const ID& a_identifier) const GCCFASTCALL;

		template <typename ID>
		const Signal& MSFASTCALL getChannel(const ID& a_identifier) const GCCFASTCALL;

		template <typename ID>
		Signal& MSFASTCALL getChannel(const ID& a_identifier) GCCFASTCALL;

		template <typename ID, typename T>
		void MSFASTCALL setChannel(const ID& a_identifier, const T& a_newVal) GCCFASTCALL;

		template <typename ID>
		string getChannelName(const ID& a_identifier) const;

		void MSFASTCALL clear() GCCFASTCALL;
	private:
		NamedContainer<Signal> m_signals;
		int m_numChannels;
	};

	template <typename ID>
	const Signal& SignalBus::getChannel(const ID& a_identifier) const {
		return m_signals[a_identifier];
	}

	template <typename ID>
	Signal& SignalBus::getChannel(const ID& a_identifier) {
		return m_signals[a_identifier];
	}

	template <typename ID, typename T>
	void SignalBus::setChannel(const ID& a_identifier, const T& a_newVal) {
		Signal& sig = m_signals[a_identifier];
		sig.set(a_newVal);
	}

	template <typename ID>
	double SignalBus::getValue(const ID& a_identifier) const {
		const Signal& signal = m_signals[a_identifier];
		return signal.get();
	}

	template <typename ID>
	string SignalBus::getChannelName(const ID& a_identifier) const {
		return m_signals.getItemName(a_identifier);
	}
}

#endif //VOSIMLIB_SIGNALBUS_H
