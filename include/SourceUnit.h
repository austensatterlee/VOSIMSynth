#ifndef __SourceUnit__
#define __SourceUnit__
#include "Unit.h"
#include "GallantSignal.h"

using Gallant::Signal0;

namespace syn
{
	class SourceUnit :
		public Unit
	{
	public:
		SourceUnit(string name) :
			Unit(name), 
			m_lastSyncIndex(0)
		{
		};

		virtual ~SourceUnit()
		{
		};

		UNIT_TYPE getUnitType() const override {
			return SOURCE_UNIT;
		}

		virtual void noteOn(int pitch, int vel) = 0;
		virtual void noteOff(int pitch, int vel) = 0;
		virtual int getSamplesPerPeriod() const = 0;
		virtual bool isActive() const = 0;

		int getLastSyncIndex() const
		{
			return m_lastSyncIndex;
		};

	protected:
		int m_lastSyncIndex;

		void beginProcessing() override
		{
			m_lastSyncIndex = 0;
		}

		/// Called by child class when phase gets reset
		void updateSyncStatus() {
			m_lastSyncIndex = m_bufind;
		}
	};
}

#endif // __SourceUnit__


