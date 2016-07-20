#include "VOSIMComponent.h"
#include "UIUnitContainer.h"
#include "VoiceManager.h"

synui::VOSIMComponent::VOSIMComponent(MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_unitFactory): UIComponent(a_window), m_vm(a_vm), m_unitFactory(a_unitFactory) {
	m_unitSelector = new UIUnitSelector(m_window, m_unitFactory);
	m_controlPanel = new UIControlPanel(m_window);
	m_controlPanel->setSize({ -1,200 });

	m_circuitPanel = new UICircuitPanel(m_window, m_vm, m_unitFactory, m_unitSelector, m_controlPanel);
	addChild(m_unitSelector);
	setZOrder(m_unitSelector, 1);
	addChild(m_controlPanel);
	setZOrder(m_controlPanel, 0);
	addChild(m_circuitPanel);
	setZOrder(m_circuitPanel, 2);
}

void synui::VOSIMComponent::save(ByteChunk* a_data) const {
	const vector<UIUnitContainer*>& units = m_circuitPanel->getUnits();
	int nUnits = units.size() - 2; // skip input/output units
	a_data->Put<int>(&nUnits);
	for (int i = 0; i < nUnits; i++) {
		int unitId = units[i + 2]->getUnitId();
		Vector2i pos = units[i + 2]->getRelPos();
		a_data->Put<int>(&unitId);
		a_data->Put<Vector2i>(&pos);
		// reserved
		int reservedBytes = 128;
		a_data->Put<int>(&reservedBytes);
		for (int j = 0; j < reservedBytes; j++) {
			char tmp = 0;
			a_data->Put<char>(&tmp);
		}
	}
}

int synui::VOSIMComponent::load(ByteChunk* a_data, int startPos) const {
	/* Reset CircuitPanel */
	m_circuitPanel->reset();

	/* Load CircuitPanel */
	// Add units
	int nUnits;
	startPos = a_data->Get<int>(&nUnits, startPos);
	for (int i = 0; i < nUnits; i++) {
		int unitId;
		Vector2i pos;
		startPos = a_data->Get<int>(&unitId, startPos);
		startPos = a_data->Get<Vector2i>(&pos, startPos);
		// reserved
		int reservedBytes;
		startPos = a_data->Get<int>(&reservedBytes, startPos);
		startPos += reservedBytes;

		if (m_vm->getPrototypeCircuit()->hasUnit(unitId)) {
			m_circuitPanel->onAddUnit_(m_vm->getPrototypeCircuit()->getUnit(unitId).getClassIdentifier(), unitId);
			m_circuitPanel->findUnitContainer(unitId)->setRelPos(pos);
		}
	}
	// Add wires
	const vector<syn::ConnectionRecord>& records = m_vm->getPrototypeCircuit()->getConnections();
	for (int i = 0; i < records.size(); i++) {
		const syn::ConnectionRecord& rec = records[i];
		if (m_vm->getPrototypeCircuit()->hasUnit(rec.from_id) && m_vm->getPrototypeCircuit()->hasUnit(rec.to_id))
			m_circuitPanel->onAddConnection_(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
	}
	return startPos;
}

void synui::VOSIMComponent::notifyChildResized(UIComponent* a_child) {
	_onResize();
}

void synui::VOSIMComponent::draw(NVGcontext* a_nvg) {
}

void synui::VOSIMComponent::_onResize() {
	m_controlPanel->setSize({ size().x() - 4, -1 });
	m_controlPanel->setRelPos({ 2, size().y() - 5 - m_controlPanel->size().y() });

	m_unitSelector->setRelPos({ 0,0 });
	m_unitSelector->setSize({ -1,m_controlPanel->getRelPos().y() - 1 });

	m_circuitPanel->setRelPos({ m_unitSelector->size().x()+1, 0 });
	m_circuitPanel->setSize({ size().x() - m_circuitPanel->getRelPos().x(), m_controlPanel->getRelPos().y()-1 });
};
