#include "UICircuitPanel.h"
#include <VoiceManager.h>
#include <UIWindow.h>
#include <UIDefaultUnitControl.h>

syn::UICircuitPanel::UICircuitPanel(VOSIMWindow* a_window, VoiceManager* a_vm, UnitFactory* a_unitFactory):
	UIComponent{a_window},
	m_vm(a_vm),
	m_unitFactory(a_unitFactory) 
{
	m_unitSelector = new UIUnitSelector{ m_window, m_unitFactory };
	UIWindow* unitSelectorWindow = new UIWindow{ m_window,"Unit Selector" };
	unitSelectorWindow->setRelPos({ 200,200 });
	m_unitSelector->setRelPos({ 5, theme()->mWindowHeaderHeight });
	addChild(unitSelectorWindow);
	unitSelectorWindow->addChild(m_unitSelector);

	const Circuit& circ = m_vm->getCircuit();
	onAddUnit_(circ.getUnit(circ.getInputUnitId()).getClassIdentifier(), circ.getInputUnitId());
	m_inputs = m_unitControls.back();
	const vector<UIUnitPort*>& inputPorts = m_inputs->getInPorts();
	for(UIUnitPort* port : inputPorts) {
		port->setVisible(false);
	}
	onAddUnit_(circ.getUnit(circ.getOutputUnitId()).getClassIdentifier(), circ.getOutputUnitId());
	m_outputs = m_unitControls.back();
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

bool syn::UICircuitPanel::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (UIComponent::onMouseDown(a_relCursor, a_diffCursor))
		return true;
	if (m_unitSelector->selectedGroup()>=0 && m_unitSelector->selectedPrototype()>=0) {
		unsigned classId = m_unitFactory->getClassId(m_unitSelector->selectedPrototypeName());
		requestAddUnit_(classId);
		m_unitSelector->setSelectedPrototype(-1);
		return true;
	}
	return false;
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
		int fromUnit, fromPort, toUnit, toPort;
		int pos = 0;
		pos = a_data->Get<int>(&fromUnit, pos);
		pos = a_data->Get<int>(&fromPort, pos);
		pos = a_data->Get<int>(&toUnit, pos);
		pos = a_data->Get<int>(&toPort, pos);
		a_circuit->connectInternal(fromUnit, fromPort, toUnit, toPort);
	};

	msg->data.Put<int>(&a_fromUnit);
	msg->data.Put<int>(&a_fromPort);
	msg->data.Put<int>(&a_toUnit);
	msg->data.Put<int>(&a_toPort);
	m_vm->queueAction(msg);
}

void syn::UICircuitPanel::onResize() {
	m_inputs->parent()->setRelPos({ 50,m_size[1] / 2 });
	m_outputs->parent()->setRelPos({ m_size[0] - 100,m_size[1] / 2 });
}

void syn::UICircuitPanel::draw(NVGcontext* a_nvg) {
	const Circuit& circ = m_vm->getCircuit();
	const vector<ConnectionRecord>& connRecs = circ.getConnectionRecords();

	nvgSave(a_nvg);
	for(ConnectionRecord connRec : connRecs) {
		Vector2i fromPos = findUnit(connRec.from_id)->getOutPorts()[connRec.from_port]->getAbsCenter();
		Vector2i toPos = findUnit(connRec.to_id)->getInPorts()[connRec.to_port]->getAbsCenter();
		nvgBeginPath(a_nvg);
		nvgStrokeColor(a_nvg,Color(Vector3f{ 0.0f,0.0f,0.0f }));
		nvgMoveTo(a_nvg, fromPos[0], fromPos[1]);
		nvgLineTo(a_nvg, toPos[0], toPos[1]);
		nvgStroke(a_nvg);
	}
	nvgRestore(a_nvg);
}

void syn::UICircuitPanel::onAddUnit_(unsigned a_classId, int a_unitId) {
	UIUnitControl* unitctrl = new DefaultUnitControl(m_window, m_vm, a_unitId);
	UIUnitControlContainer* unitctrlcontainer = new UIUnitControlContainer(m_window, m_vm, a_unitId, unitctrl);
	unitctrlcontainer->setRelPos({ 0,theme()->mWindowHeaderHeight });
	UIWindow* unitCtrlWin = new UIWindow(m_window, m_vm->getUnit(a_unitId).getName());
	unitCtrlWin->addChild(unitctrlcontainer);
	unitCtrlWin->setRelPos(m_window->cursorPos() - m_pos);
	addChild(unitCtrlWin);
	m_unitControls.push_back(unitctrlcontainer);
}

void syn::UICircuitPanel::requestAddUnit_(unsigned a_classId) {
	ActionMessage* msg = new ActionMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* data)
		{
			UICircuitPanel* self;
			UnitFactory* unitFactory;
			unsigned classId;
			int pos = 0;
			pos = data->Get<UICircuitPanel*>(&self,pos);
			pos = data->Get<UnitFactory*>(&unitFactory, pos);
			pos = data->Get<unsigned>(&classId, pos);
			int unitId = a_circuit->addUnit(shared_ptr<Unit>(unitFactory->createUnit(classId)));
			if (a_isLast)
				self->onAddUnit_(classId, unitId);
		};

	UICircuitPanel* self = this;
	msg->data.Put<UICircuitPanel*>(&self);
	msg->data.Put<UnitFactory*>(&m_unitFactory);
	msg->data.Put<unsigned>(&a_classId);	
	m_vm->queueAction(msg);
}
