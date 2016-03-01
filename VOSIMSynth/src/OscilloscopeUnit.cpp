#include "OscilloscopeUnit.h"
#include <DSPMath.h>

namespace syn {
	void OscilloscopeUnit::onParamChange_(int a_paramId) {
		if(a_paramId==m_pBufferSize) {
			int newBufferSize = getParameter(m_pBufferSize).getInt();
			int oldBufferSize = m_bufferSize;
			m_bufferIndex = MIN(m_bufferIndex,newBufferSize);
			m_bufferSize = newBufferSize;
			if (m_bufferSize > oldBufferSize) {
				for (int i = 0; i < m_nBuffers; i++) {
					m_buffer[i].resize(m_bufferSize);
				}
			}
		}
	}

	void OscilloscopeUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		for (int i = 0; i < m_nBuffers; i++) {
			m_buffer[i][m_bufferIndex] = a_inputs.getValue(i);
		}
		m_bufferIndex = WRAP(m_bufferIndex+1, m_bufferSize);
	}
}
