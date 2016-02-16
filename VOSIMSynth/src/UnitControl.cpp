#include "UnitControl.h"
#include <DSPMath.h>

namespace syn
{
	UnitControl::UnitControl(): m_unitId(0), m_plug(nullptr) {
		
	}

	UnitControl::UnitControl(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y):
		m_voiceManager(a_vm),
		m_unitId(a_unitId),
		m_pos(a_x,a_y),
		m_size(0,0),
		m_plug(a_plug) {
		move({ a_x,a_y });
	}

	bool UnitControl::isHit(int a_x, int a_y) const {
		return m_pos[0] < a_x && m_pos[1] < a_y && m_pos[0] + m_size[0] > a_x && m_pos[1] + m_size[1] > a_y;
	}

	void UnitControl::move(NDPoint<2, int> a_newPos) {
		m_pos = a_newPos;
		onChangeRect_();
	}

	UnitControl* UnitControl::construct(IPlugBase* a_mPlug, shared_ptr<VoiceManager> a_voiceManager, int a_uid, int a_x, int a_y) const {
		return _construct(a_mPlug, a_voiceManager, a_uid, a_x, a_y);
	}

	void UnitControl::resize(NDPoint<2, int> a_newSize) {
		m_size[0] = MAX(m_minSize[0], a_newSize[0]);
		m_size[1] = MAX(m_minSize[1], a_newSize[1]);
		onChangeRect_();
	}

	NDPoint<2, int> UnitControl::getMinSize() const { return m_minSize; }

	void UnitControl::setUnitId(int a_newUnitId) { m_unitId = a_newUnitId; onSetUnitId_(); }

	void UnitControl::updateMinSize_(NDPoint<2, int> a_newMinSize) {
		m_minSize[0] = MAX(m_minSize[0], a_newMinSize[0]);
		m_minSize[1] = MAX(m_minSize[1], a_newMinSize[1]);
	}

	void UnitControl::resetMinSize_() {
		m_minSize = { 0,0 };
	}

	int UnitControl::getSelectedParam(int a_x, int a_y) { return -1; }
}

