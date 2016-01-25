#pragma once
#include <random>
#include "DSPMath.h"
using namespace std;

namespace syn
{
	class RandomGenerator : public SourceUnit
	{
	public:
		void noteOn(int pitch, int vel) override { }

		void noteOff(int pitch, int vel) override { }

		int getSamplesPerPeriod() const override { return 2; }

		bool isActive() const override { return true; }

		RandomGenerator(string name)
			: SourceUnit(name), m_curr(0), m_next(0) {}

		virtual ~RandomGenerator() {}

		RandomGenerator(const RandomGenerator& other) : RandomGenerator(other.m_name) {
			m_curr = other.m_curr;
		}

	protected:
		uint32_t m_curr, m_next;

		void process(int bufind) override {
			m_curr = m_next;
			m_next = 69069 * m_next + 1;
			m_output[bufind] = m_next;
		}

		void onSampleRateChange(double newfs) override { }

	private:
		Unit* cloneImpl() const override {
			return new RandomGenerator(*this);
		};

		string getClassName() const override {
			return "RandomGenerator";
		};
	};
};

