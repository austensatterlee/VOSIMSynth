#include "UICircuitPanel.h"
#include "VoiceManager.h"
#include "UnitFactory.h"
#include "UIWindow.h"
#include "UIDefaultUnitControl.h"
#include "UIUnitControlContainer.h"
#include "UIUnitPort.h"
#include "UIWire.h"
#include "UICell.h"
#include "UIUnitSelector.h"
#include "UIButton.h"
#include "Theme.h"


syn::UICircuitPanel::UICircuitPanel(VOSIMWindow* a_window, VoiceManager* a_vm, UnitFactory* a_unitFactory) :
	UIComponent{ a_window },
	m_vm(a_vm),
	m_unitFactory(a_unitFactory)
{
	m_unitSelector = new UIUnitSelector{ m_window, m_unitFactory };
	UIWindow* unitSelectorWindow = new UIWindow{ m_window,"Unit Selector" };
	unitSelectorWindow->setRelPos({ 5,45 });
	addChild(unitSelectorWindow);
	unitSelectorWindow->addChildToBody(m_unitSelector);

	const Circuit* circ = m_vm->getPrototypeCircuit();
	onAddUnit_(circ->getUnit(circ->getInputUnitId()).getClassIdentifier(), circ->getInputUnitId());
	m_inputs = m_unitControls.back();
	m_inputs->m_headerRow->removeChild(m_inputs->m_closeButton);
	m_inputs->m_closeButton = nullptr;
	vector<UIUnitPort*>& inputPorts = m_inputs->m_inPorts;
	inputPorts.clear();
	m_inputs->m_bodyRow->removeChild(m_inputs->m_cols[0]);

	onAddUnit_(circ->getUnit(circ->getOutputUnitId()).getClassIdentifier(), circ->getOutputUnitId());
	m_outputs = m_unitControls.back();
	m_outputs->m_headerRow->removeChild(m_outputs->m_closeButton);
	m_outputs->m_closeButton = nullptr;
	vector<UIUnitPort*>& outputPorts = m_outputs->m_outPorts;
	outputPorts.clear();
	m_outputs->m_bodyRow->removeChild(m_outputs->m_cols[1]);

	setMinSize({ 400, 400 });
}

syn::UIUnitControlContainer* syn::UICircuitPanel::getUnit(const UICoord& a_absPt) const {
	UIUnitControlContainer* found = dynamic_cast<UIUnitControlContainer*>(findChild(a_absPt,"units"));
	return found;
}

syn::UIUnitControlContainer* syn::UICircuitPanel::findUnit(int a_unitId) const {
	for (UIUnitControlContainer* unitCtrlContainer : m_unitControls) {
		if (unitCtrlContainer->getUnitId() == a_unitId)
			return unitCtrlContainer;
	}
	return nullptr;
}

void syn::UICircuitPanel::requestAddConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	// remove old wire
	for (UIWire* wire : m_wires) {
		if(wire->toUnit()==a_toUnit && wire->toPort()==a_toPort) {
			onDeleteConnection_(wire->fromUnit(), wire->fromPort(), wire->toUnit(), wire->toPort());
			break;
		}
	}

	// send the new connection request to the real-time thread
	RTMessage* msg = new RTMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		UICircuitPanel* self;
		int fromUnit, fromPort, toUnit, toPort;
		GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
		a_circuit->connectInternal(fromUnit, fromPort, toUnit, toPort);

		// Queue return message
		if (a_isLast) {
			GUIMessage* msg = new GUIMessage;
			msg->action = [](VOSIMWindow* a_win, ByteChunk* a_data)
			{
				int fromUnit, fromPort, toUnit, toPort;
				GetArgs(a_data, 0, fromUnit, fromPort, toUnit, toPort);
				a_win->getCircuitPanel()->onAddConnection_(fromUnit, fromPort, toUnit, toPort);
			};
			PutArgs(&msg->data, fromUnit, fromPort, toUnit, toPort);
			self->m_window->queueExternalMessage(msg);
		}
	};

	UICircuitPanel* self = this;
	PutArgs(&msg->data, self, a_fromUnit, a_fromPort, a_toUnit, a_toPort);
	m_vm->queueAction(msg);
}

void syn::UICircuitPanel::requestDeleteConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	RTMessage* msg = new RTMessage();
	UICircuitPanel* self = this;
	PutArgs(&msg->data, self, a_fromUnit, a_fromPort, a_toUnit, a_toPort);

	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		UICircuitPanel* self;
		int fromUnit, fromPort, toUnit, toPort;
		GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
		a_circuit->disconnectInternal(fromUnit, fromPort, toUnit, toPort);

		// Queue return message
		if (a_isLast) {
			GUIMessage* msg = new GUIMessage;
			PutArgs(&msg->data, fromUnit, fromPort, toUnit, toPort);
			msg->action = [](VOSIMWindow* a_win, ByteChunk* a_data)
			{
				int fromUnit, fromPort, toUnit, toPort;
				GetArgs(a_data, 0, fromUnit, fromPort, toUnit, toPort);
				a_win->getCircuitPanel()->onDeleteConnection_(fromUnit, fromPort, toUnit, toPort);
			};
			self->m_window->queueExternalMessage(msg);
		}
	};

	m_vm->queueAction(msg);
}

void syn::UICircuitPanel::requestMoveConnection(UIWire* a_wire, int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	if (a_wire->originVector() == Vector2i{ a_fromUnit, a_fromPort } && a_wire->destVector() == Vector2i{ a_toUnit, a_toPort })
		return;
	requestDeleteConnection(a_wire->fromUnit(), a_wire->fromPort(), a_wire->toUnit(), a_wire->toPort());
	requestAddConnection(a_fromUnit, a_fromPort, a_toUnit, a_toPort);
}

void syn::UICircuitPanel::requestDeleteUnit(int a_unitId) {
	// Delete the corresponding UIUnitControlContainer
	UIUnitControlContainer* unitCtrlCont = findUnit(a_unitId);
	m_window->clearFocus();
	removeChild(unitCtrlCont);

	m_unitControls.erase(find(m_unitControls.begin(), m_unitControls.end(), unitCtrlCont));
	// Delete the corresponding wires
	for (int i = 0; i < m_wires.size(); i++) {
		UIWire* wire = m_wires[i];
		if (wire->fromUnit() == a_unitId || wire->toUnit() == a_unitId) {
			removeChild(wire);
			m_wires.erase(m_wires.begin() + i);
			i--;
		}
	}
	// Delete the unit from the circuit
	RTMessage* msg = new RTMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		int unitId;
		GetArgs(a_data, 0, unitId);
		a_circuit->removeUnit(unitId);
	};

	PutArgs(&msg->data, a_unitId);
	m_vm->queueAction(msg);
}

syn::UIWire* syn::UICircuitPanel::getSelectedWire() {
	UICoord mousePt(m_window->cursorPos());
	// Return nullptr if pt is over a port
	UIUnitControlContainer* ctrlContainer = m_window->getCircuitPanel()->getUnit(mousePt);
	if (ctrlContainer && (ctrlContainer->getSelectedInPort(mousePt) || ctrlContainer->getSelectedOutPort(mousePt)))
		return nullptr;

	make_heap(m_wireSelectionQueue.begin(), m_wireSelectionQueue.end(), UIWire::compareByHoverDistance());
	return m_wireSelectionQueue.empty() ? nullptr : m_wireSelectionQueue.front();
}

void syn::UICircuitPanel::_onResize() {
	m_inputs->setRelPos({ 5, size()[1] / 2 - m_inputs->size()[1] / 2 });
	m_outputs->setRelPos({ size()[0] - m_outputs->size()[0] - 5, size()[1] / 2 - m_outputs->size()[1] / 2 });
}

void syn::UICircuitPanel::setSelectedWire(UIWire* a_wire) {
	if (find(m_wireSelectionQueue.begin(), m_wireSelectionQueue.end(), a_wire) == m_wireSelectionQueue.end())
		m_wireSelectionQueue.push_back(a_wire);
}

void syn::UICircuitPanel::clearSelectedWire(UIWire* a_wire) {
	auto it = find(m_wireSelectionQueue.begin(), m_wireSelectionQueue.end(), a_wire);
	if (it != m_wireSelectionQueue.end())
		m_wireSelectionQueue.erase(it);
}

void syn::UICircuitPanel::reset() {
	for (int i = 0; i < m_wires.size(); i++) {
		removeChild(m_wires[i]);
	}
	for (int i = 0; i < m_unitControls.size(); i++) {
		if (m_unitControls[i] != m_inputs && m_unitControls[i] != m_outputs) // don't allow deletion of input or output units
			removeChild(m_unitControls[i]);
	}
	m_unitControls.clear();
	m_unitControls.push_back(m_inputs);
	m_unitControls.push_back(m_outputs);
	m_wires.clear();
	m_wireSelectionQueue.clear();
}

syn::UIComponent* syn::UICircuitPanel::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (child)
		return child;

	if (m_unitSelector->selectedPrototype() >= 0) {
		unsigned classId = m_unitFactory->getClassId(m_unitSelector->selectedPrototypeName());
		requestAddUnit_(classId);
		m_unitSelector->setSelectedPrototype(-1);
		return this;
	}
	return nullptr;
}

bool syn::UICircuitPanel::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
}

void syn::UICircuitPanel::draw(NVGcontext* a_nvg) {}

void syn::UICircuitPanel::setChildrenStyles(NVGcontext* a_nvg) {
	nvgIntersectScissor(a_nvg, 0.0f, 0.0f, size()[0], size()[1]);
}

void syn::UICircuitPanel::onAddConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	for(UIWire* wire : m_wires) {
		if(wire->toPort()==a_toPort && wire->toUnit()==a_toUnit) {
			onDeleteConnection_(wire->fromUnit(), wire->fromPort(), wire->toUnit(), wire->toPort());
			break;
		}
	}
	UIWire* wire = new UIWire(m_window, a_fromUnit, a_fromPort, a_toUnit, a_toPort);
	m_wires.push_back(wire);
	addChild(wire,"wires");
	setZOrder(wire, -1);
}

void syn::UICircuitPanel::onDeleteConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	for (int i = 0; i < m_wires.size(); i++) {
		UIWire* wire = m_wires[i];
		if (wire->fromUnit() == a_fromUnit && wire->fromPort() == a_fromPort && wire->toUnit() == a_toUnit && wire->toPort() == a_toPort) {
			m_wires.erase(m_wires.cbegin() + i);
			m_window->forfeitFocus(wire);
			clearSelectedWire(wire);
			removeChild(wire);
			return;
		}
	}
}

void syn::UICircuitPanel::onAddUnit_(unsigned a_classId, int a_unitId) {
	UIUnitControl* unitctrl = m_window->createUnitControl(a_classId, a_unitId);
	UIUnitControlContainer* unitctrlcontainer = new UIUnitControlContainer(m_window, m_vm, a_unitId, unitctrl);
	unitctrlcontainer->setRelPos(m_window->cursorPos() - m_pos - unitctrlcontainer->size()/2);
	addChild(unitctrlcontainer, "units");
	m_unitControls.push_back(unitctrlcontainer);
}

void syn::UICircuitPanel::requestAddUnit_(unsigned a_classId) {
	RTMessage* msg = new RTMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		UICircuitPanel* self;
		UnitFactory* unitFactory;
		Unit* unit;
		GetArgs(a_data, 0, self, unitFactory, unit);
		int unitId = a_circuit->addUnit(unit->clone());
		// Queue return message
		if (a_isLast) {
			GUIMessage* msg = new GUIMessage;
			msg->action = [](VOSIMWindow* a_win, ByteChunk* a_data) {
				unsigned classId;
				int unitId;
				GetArgs(a_data, 0, classId, unitId);
				a_win->getCircuitPanel()->onAddUnit_(classId, unitId);
			};
			unsigned classId = unit->getClassIdentifier();
			PutArgs(&msg->data, classId, unitId);
			self->m_window->queueExternalMessage(msg);
			delete unit;
		}
	};

	UICircuitPanel* self = this;
	Unit* unit = m_window->getUnitFactory()->createUnit(a_classId);
	PutArgs(&msg->data, self, m_unitFactory, unit);
	m_vm->queueAction(msg);
}