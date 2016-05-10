#include "UICircuitPanel.h"
#include <VoiceManager.h>
#include <UIWindow.h>
#include <UIDefaultUnitControl.h>
#include <eigen/src/Core/util/ForwardDeclarations.h>
#include <eigen/src/Core/util/ForwardDeclarations.h>

bool syn::UIWire::contains(const Vector2i& a_pt) {
	NDPoint<2, int> pt{ a_pt[0],a_pt[1] };
	NDPoint<2, int> frompt{ fromPt()[0], fromPt()[1] };
	NDPoint<2, int> topt{ toPt()[0], toPt()[1] };
	double distance = pointLineDistance(pt, frompt, topt);
	return distance < 5;
}

void syn::UIWire::onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) {
	UIComponent::onMouseEnter(a_relCursor, a_diffCursor, a_isEntering);

	if(a_isEntering)
		m_window->getCircuitPanel()->setSelectedWire(this);
	else
		m_window->getCircuitPanel()->clearSelectedWire(this);
}

bool syn::UIWire::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	UIUnitControlContainer* selectedUnit = m_window->getCircuitPanel()->getUnit(m_window->cursorPos());
	m_isDragging = false;
	if (selectedUnit && selectedUnit != m_parent) {
		if (!m_isDraggingInput) {
			UIUnitPort* port = selectedUnit->getSelectedOutPort(m_window->cursorPos());
			if (port && !(port->getUnitId() == m_fromUnit && port->getPortId() == m_fromPort))
				m_window->getCircuitPanel()->requestAddConnection(port->getUnitId(), port->getPortId(), m_toUnit, m_toPort);
			else
				return true;
		}
		else {
			UIUnitPort* port = selectedUnit->getSelectedInPort(m_window->cursorPos());
			if (port && !(port->getUnitId()==m_toUnit && port->getPortId()==m_toPort))
				m_window->getCircuitPanel()->requestAddConnection(m_fromUnit, m_fromPort, port->getUnitId(), port->getPortId());
			else
				return true;
		}
	}
	m_window->getCircuitPanel()->requestDeleteConnection(m_fromUnit, m_fromPort, m_toUnit, m_toPort);
	return true;
}

syn::UIComponent* syn::UIWire::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	Vector2i frompt = fromPt();
	Vector2i topt = toPt();

	int distanceToInput = (a_relCursor - topt).squaredNorm();
	int distanceToOutput = (a_relCursor - frompt).squaredNorm();
	m_isDragging = true;
	m_isDraggingInput = distanceToInput < distanceToOutput;
	return this;
}

Eigen::Vector2i syn::UIWire::fromPt() const {
	Vector2i fromPos;
	if (m_isDragging && !m_isDraggingInput) {
		fromPos = m_window->cursorPos();
	}
	else {
		fromPos = m_window->getCircuitPanel()->findUnit(m_fromUnit)->getOutPorts()[m_fromPort]->getAbsCenter();
	}
	return fromPos;
}

Eigen::Vector2i syn::UIWire::toPt() const {
	Vector2i toPos;
	if(m_isDragging && m_isDraggingInput) {
		toPos = m_window->cursorPos();
	} else {
		toPos = m_window->getCircuitPanel()->findUnit(m_toUnit)->getInPorts()[m_toPort]->getAbsCenter();
	}
	return toPos;
}

void syn::UIWire::draw(NVGcontext* a_nvg) {
	Vector2i fromPos = fromPt();
	Vector2i toPos = toPt();

	nvgBeginPath(a_nvg);
	if (m_window->getCircuitPanel()->getSelectedWire()==this) {
		nvgStrokeColor(a_nvg, Color(Vector3f{ 1.0f,0.0f,0.0f }));
	}
	else {
		nvgStrokeColor(a_nvg, Color(Vector3f{ 0.0f,0.0f,0.0f }));
	}
	nvgMoveTo(a_nvg, fromPos[0], fromPos[1]);
	nvgLineTo(a_nvg, toPos[0], toPos[1]);

	nvgStroke(a_nvg);
}

syn::UICircuitPanel::UICircuitPanel(VOSIMWindow* a_window, VoiceManager* a_vm, UnitFactory* a_unitFactory) :
	UIComponent{a_window},
	m_vm(a_vm),
	m_unitFactory(a_unitFactory),
	m_selectedWire(nullptr)
{
	m_unitSelector = new UIUnitSelector{ m_window, m_unitFactory };
	UIWindow* unitSelectorWindow = new UIWindow{ m_window,"Unit Selector" };
	unitSelectorWindow->setRelPos({ 5,45 });
	m_unitSelector->setRelPos({ 5, theme()->mWindowHeaderHeight });
	addChild(unitSelectorWindow);
	unitSelectorWindow->addChild(m_unitSelector);

	const Circuit& circ = m_vm->getCircuit();
	onAddUnit_(circ.getUnit(circ.getInputUnitId()).getClassIdentifier(), circ.getInputUnitId());
	m_inputs = m_unitControls.back();
	m_inputs->m_headerRow->removeChild(m_inputs->m_closeButton);
	m_inputs->m_closeButton = nullptr;
	const vector<UIUnitPort*>& inputPorts = m_inputs->getInPorts();
	for(UIUnitPort* port : inputPorts) {
		port->setVisible(false);
	}
	onAddUnit_(circ.getUnit(circ.getOutputUnitId()).getClassIdentifier(), circ.getOutputUnitId());
	m_outputs = m_unitControls.back();
	m_outputs->m_headerRow->removeChild(m_outputs->m_closeButton);
	m_outputs->m_closeButton = nullptr;
	const vector<UIUnitPort*>& outputPorts = m_outputs->getOutPorts();
	for (UIUnitPort* port : outputPorts) {
		port->setVisible(false);
	}
}

bool syn::UICircuitPanel::onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (UIComponent::onMouseMove(a_relCursor, a_diffCursor))
		return true;
	return false;
}

syn::UIComponent* syn::UICircuitPanel::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor);
	if (child) return child;

	if (m_unitSelector->selectedGroup()>=0 && m_unitSelector->selectedPrototype()>=0) {
		unsigned classId = m_unitFactory->getClassId(m_unitSelector->selectedPrototypeName());
		requestAddUnit_(classId);
		m_unitSelector->setSelectedPrototype(-1);
		return this;
	}
	return nullptr;
}

bool syn::UICircuitPanel::onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) {
	if (UIComponent::onMouseScroll(a_relCursor, a_diffCursor, a_scrollAmt))
		return true;
	return false;
}

syn::UIUnitControlContainer* syn::UICircuitPanel::getUnit(const Vector2i& a_pt) const {
	for(UIUnitControlContainer* unitCtrlContainer : m_unitControls) {
		Vector2i relPos = a_pt - unitCtrlContainer->parent()->getAbsPos();
		if (unitCtrlContainer->contains(relPos)) {
			return unitCtrlContainer;
		}
	}
	return nullptr;
}

syn::UIUnitControlContainer* syn::UICircuitPanel::findUnit(int a_unitId) const {
	for (UIUnitControlContainer* unitCtrlContainer : m_unitControls) {
		if (unitCtrlContainer->getUnitId() == a_unitId)
			return unitCtrlContainer;
	}
	return nullptr;
}

void syn::UICircuitPanel::requestAddConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort)
{
	ActionMessage* msg = new ActionMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
	{
		UICircuitPanel* self;
		int fromUnit, fromPort, toUnit, toPort;
		int pos = 0;
		pos = a_data->Get<UICircuitPanel*>(&self, pos);
		pos = a_data->Get<int>(&fromUnit, pos);
		pos = a_data->Get<int>(&fromPort, pos);
		pos = a_data->Get<int>(&toUnit, pos);
		pos = a_data->Get<int>(&toPort, pos);
		a_circuit->connectInternal(fromUnit, fromPort, toUnit, toPort);
		if (a_isLast)
			self->onAddConnection_(fromUnit, fromPort, toUnit, toPort);
	};

	UICircuitPanel* self = this;
	msg->data.Put<UICircuitPanel*>(&self);
	msg->data.Put<int>(&a_fromUnit);
	msg->data.Put<int>(&a_fromPort);
	msg->data.Put<int>(&a_toUnit);
	msg->data.Put<int>(&a_toPort);
	m_vm->queueAction(msg);
}

void syn::UICircuitPanel::requestDeleteConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	ActionMessage* msg = new ActionMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
	{
		UICircuitPanel* self;
		int fromUnit, fromPort, toUnit, toPort;
		int pos = 0;
		pos = a_data->Get<UICircuitPanel*>(&self, pos);
		pos = a_data->Get<int>(&fromUnit, pos);
		pos = a_data->Get<int>(&fromPort, pos);
		pos = a_data->Get<int>(&toUnit, pos);
		pos = a_data->Get<int>(&toPort, pos);
		a_circuit->disconnectInternal(fromUnit, fromPort, toUnit, toPort);
		if (a_isLast)
			self->onDeleteConnection_(fromUnit, fromPort, toUnit, toPort);
	};

	UICircuitPanel* self = this;
	msg->data.Put<UICircuitPanel*>(&self);
	msg->data.Put<int>(&a_fromUnit);
	msg->data.Put<int>(&a_fromPort);
	msg->data.Put<int>(&a_toUnit);
	msg->data.Put<int>(&a_toPort);
	m_vm->queueAction(msg);
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
	ActionMessage* msg = new ActionMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
	{
		UICircuitPanel* self;
		UnitFactory* unitFactory;
		int unitId;
		int pos = 0;
		pos = a_data->Get<UICircuitPanel*>(&self, pos);
		pos = a_data->Get<UnitFactory*>(&unitFactory, pos);
		pos = a_data->Get<int>(&unitId, pos);
		a_circuit->removeUnit(unitId);
	};

	UICircuitPanel* self = this;
	msg->data.Put<UICircuitPanel*>(&self);
	msg->data.Put<UnitFactory*>(&m_unitFactory);
	msg->data.Put<int>(&a_unitId);
	m_vm->queueAction(msg);
}

void syn::UICircuitPanel::_onResize() {
	m_inputs->setRelPos({ 5, size()[1] / 2 - m_inputs->size()[1]/2 });
	m_outputs->setRelPos({ size()[0] - m_outputs->size()[0] - 5, size()[1] / 2 - m_outputs->size()[1]/2 });
}

void syn::UICircuitPanel::setSelectedWire(UIWire* a_wire) {
	m_selectedWire = a_wire;
}

void syn::UICircuitPanel::clearSelectedWire(UIWire* a_wire) {
	if (m_selectedWire == a_wire)
		m_selectedWire = nullptr;
}

void syn::UICircuitPanel::reset() {
	for (int i = 0; i < m_unitControls.size(); i++) {
		if(m_unitControls[i]!=m_inputs && m_unitControls[i]!=m_outputs) // don't allow deletion of input or output units
			removeChild(m_unitControls[i]);
	}
	for (int i = 0; i < m_wires.size(); i++) {
		removeChild(m_wires[i]);
	}
	m_unitControls.clear();
	m_unitControls.push_back(m_inputs);
	m_unitControls.push_back(m_outputs);
	m_wires.clear();
	m_selectedWire = nullptr;
}

void syn::UICircuitPanel::draw(NVGcontext* a_nvg) {
}

void syn::UICircuitPanel::onAddConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	UIWire* wire = new UIWire(m_window, a_fromUnit, a_fromPort, a_toUnit, a_toPort);
	m_wires.push_back(wire);
	addChild(wire);
}

void syn::UICircuitPanel::onDeleteConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) {
	for (int i = 0; i < m_wires.size();i++) {
		UIWire* wire = m_wires[i];
		if(wire->fromUnit() == a_fromUnit && wire->fromPort()==a_fromPort && wire->toUnit()==a_toUnit && wire->toPort()==a_toPort) {
			removeChild(m_wires[i]);
			m_wires.erase(m_wires.cbegin() + i);
			return;
		}
	}
}

void syn::UICircuitPanel::onAddUnit_(unsigned a_classId, int a_unitId) {
	UIUnitControl* unitctrl = m_window->createUnitControl(a_classId, a_unitId);
	UIUnitControlContainer* unitctrlcontainer = new UIUnitControlContainer(m_window, m_vm, a_unitId, unitctrl);
	unitctrlcontainer->setRelPos(m_window->cursorPos() - m_pos);
	addChild(unitctrlcontainer);
	m_unitControls.push_back(unitctrlcontainer);
}

void syn::UICircuitPanel::requestAddUnit_(unsigned a_classId) {
	ActionMessage* msg = new ActionMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
		{
			UICircuitPanel* self;
			UnitFactory* unitFactory;
			Unit* unit;
			int pos = 0;
			pos = a_data->Get<UICircuitPanel*>(&self,pos);
			pos = a_data->Get<UnitFactory*>(&unitFactory, pos);
			pos = a_data->Get<Unit*>(&unit, pos);
			int unitId = a_circuit->addUnit(shared_ptr<Unit>(unit->clone()));
			if (a_isLast)
				self->onAddUnit_(unit->getClassIdentifier(), unitId);
		};

	UICircuitPanel* self = this;
	Unit* unit = m_window->getUnitFactory()->createUnit(a_classId);
	msg->data.Put<UICircuitPanel*>(&self);
	msg->data.Put<UnitFactory*>(&m_unitFactory);
	msg->data.Put<Unit*>(&unit);	
	m_vm->queueAction(msg);
}
