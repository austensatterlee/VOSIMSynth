#include "OscilloscopeWidget.h"
#include "DSPMath.h"

namespace synui
{
	OscilloscopeUnit::OscilloscopeUnit(const string& a_name) :
		Unit(a_name),
		m_widget(nullptr),
		m_bufferIndex(0),
		m_numBuffers(1),
		m_lastPhase(0.0),
		m_lastSync(0),
		m_syncCount(0)
	{
		addParameter_(pBufferSize, syn::UnitParameter("buffer size", 16, 96000, 256));
		addParameter_(pNumPeriods, syn::UnitParameter("periods", 1, 16, 1));
		m_bufferSize = param(pBufferSize).getInt();
		m_buffers.resize(m_numBuffers);
		for(int i=0;i<m_numBuffers;i++) {
			addInput_("in"+std::to_string(i));
			m_buffers[i].resize(param(pBufferSize).getMax());
		}
		m_iPhase = addInput_("ph");
	}

	int OscilloscopeUnit::getNumBuffers() const
	{
		return m_buffers.size();
	}

	const Eigen::VectorXf& OscilloscopeUnit::getBuffer(int a_bufIndex) const {
		return m_buffers[a_bufIndex];
	}

	int OscilloscopeUnit::getBufferSize(int a_bufIndex) const {
		return inputSource(a_bufIndex) ? m_bufferSize : 0;
	}

	void OscilloscopeUnit::onParamChange_(int a_paramId) {
		if (a_paramId == pBufferSize) {
			int newBufferSize = param(pBufferSize).getInt();
			m_bufferIndex = syn::WRAP(m_bufferIndex, newBufferSize);
			m_bufferSize = newBufferSize;
		}
	}

	void OscilloscopeUnit::process_() {
		BEGIN_PROC_FUNC
			double phase = READ_INPUT(m_iPhase);
			if (phase - m_lastPhase < -0.5) {
				_sync();
			}
			m_lastSync++;
			m_lastPhase = phase;
			for (int i = 0; i < m_buffers.size(); i++) {
				m_buffers[i][m_bufferIndex] = READ_INPUT(i);
			}
			m_bufferIndex = syn::WRAP(m_bufferIndex + 1, m_bufferSize);
		END_PROC_FUNC
	}


	void OscilloscopeUnit::_sync() {
		m_syncCount++;
		if (m_syncCount >= param(pNumPeriods).getInt()) {
			setParam(pBufferSize, m_lastSync);
			m_bufferIndex = 0;
			m_lastSync = 0;
			m_syncCount = 0;
		}
	}

	void OscilloscopeWidget::draw(NVGcontext* ctx) {
		auto circuit = m_vm->getPrototypeCircuit();
		m_unitId = -1;
		for(int i=0; i<circuit->getNumUnits(); i++) {
			int unitId = circuit->getUnitId(i);
			auto unitClassId = circuit->getUnit(unitId).getClassIdentifier();
			if(unitClassId==OscilloscopeUnit::classIdentifier())
				m_unitId = unitId;
		}
		if(m_unitId>=0){
			const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
			const Eigen::VectorXf& buf = unit->getBuffer(0);
			int viewSize = unit->getBufferSize(0);
			Eigen::Map<const Eigen::VectorXf> bufView(buf.data(), viewSize);
			setValues(((bufView.array()+1.0)*0.5).matrix());
		}
		Graph::draw(ctx);
	}
}