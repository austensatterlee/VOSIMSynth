#include "VOSIMComponent.h"
#include "UIUnitContainer.h"
#include "VoiceManager.h"
#include "UITabComponent.h"
#include "UILayout.h"
#include "UIStackedComponent.h"
#include <string>

synui::VOSIMComponent::VOSIMComponent(MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_unitFactory): 
	UIComponent(a_window), 
	m_vm(a_vm), 
	m_unitFactory(a_unitFactory) 
{
	m_rightPanel = new UITabWidget(m_window);
	m_rightPanel->setLayout(new GroupLayout());
	m_leftPanel = new UITabWidget(m_window);
	m_leftPanel->setLayout(new GroupLayout());

	m_unitSelector = new UIUnitSelector(m_window, m_unitFactory);
	m_controlPanel = new UIControlPanel(m_window,m_leftPanel);

	m_circuitPanel = new UICircuitPanel(m_window, m_vm, m_unitFactory, m_unitSelector, m_controlPanel);
	addChild(m_leftPanel);
	m_leftPanel->addTab("Unit", m_unitSelector);
	m_leftPanel->addTab("Edit", m_controlPanel);
	m_leftPanel->setActiveTab(0);
	m_leftPanel->m_onMouseDown = [](UIComponent* self, const UICoord& a_loc, bool a_isDblClick)->UIComponent*
	{
		Vector2i click = a_loc.localCoord(self);
		if(abs(click.x() - self->size().x()) < 5) {
			return self;
		}
		return nullptr;
	};
	m_leftPanel->m_draw = [](UIComponent* self, NVGcontext* a_nvg)
	{
		const UIStackedComponent& content = static_cast<UITabWidget*>(self)->content();

		Vector2i globalClick = self->window()->cursorPos();
		Vector2i click = UICoord(globalClick).localCoord(self);
		Vector2i size = self->size();

		if (abs(click.x() - size.x()) < 5) {
			float glow_width = 2;
			Color inner(1.0f, 0.3f);
			Color outer(1.0f, 0.0f);

			Vector2i glow_pos(size.x() - glow_width/2.0f, content.getRelPos().y());
			Vector2i glow_size( glow_width, content.size().y());
			nvgBeginPath(a_nvg);
			NVGpaint glow = nvgBoxGradient(a_nvg, glow_pos.x(), glow_pos.y(), glow_size.x(), glow_size.y(), 4*glow_width, 4*glow_width, inner, outer);
			nvgRect(a_nvg, glow_pos.x()- 4*glow_width, glow_pos.y(), glow_size.x()+8*glow_width, glow_size.y());
			nvgFillPaint(a_nvg, glow);
			nvgFill(a_nvg);
		}
	};
	m_leftPanel->m_onMouseDrag = [](UIComponent* self, const UICoord& a_loc, const Vector2i& a_dloc)
	{
		self->grow({ a_dloc.x(), 0 });
		return true;
	};

	addChild(m_rightPanel);
	setZOrder(m_rightPanel, 1);
	m_rightPanel->addTab("Circuit Editor", m_circuitPanel);
	m_rightPanel->setActiveTab(0);
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
	m_rightPanel->setRelPos({ m_leftPanel->size().x() + 5, 5 });
	m_rightPanel->setSize({ size().x() - 5 - m_rightPanel->getRelPos().x(), size().y() - 5 });
}

void synui::VOSIMComponent::draw(NVGcontext* a_nvg) {
}

void synui::VOSIMComponent::_onResize() {
	m_leftPanel->setRelPos({ 5,5 });
	m_leftPanel->setMinSize({syn::MAX(m_leftPanel->minSize().x(),190), size().y()-5 });

	m_rightPanel->setRelPos({ m_leftPanel->size().x()+5, 5 });
	m_rightPanel->setSize({ size().x() - 5 - m_rightPanel->getRelPos().x(), size().y()-5 });
};
