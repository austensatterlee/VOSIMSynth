#include "CircuitWidget.h"
#include "VoiceManager.h"
#include "UI.h"
#include "MainWindow.h"
#include "UnitWidget.h"
#include "MainGUI.h"

namespace synui
{
	class CircuitWire
	{
	public:

		CircuitWire(bool a_startFromOutput)
			: m_inputUnit(nullptr),
			  m_inputPort(-1),
			  m_outputUnit(nullptr),
			  m_outputPort(-1),
			  m_startFromOutput(a_startFromOutput) {}

		void draw(NVGcontext* ctx, const Eigen::Vector2i& endPt = {0,0})
		{
			// Determine start/end points
			Eigen::Vector2i startPos;
			Eigen::Vector2i endPos;
			if (m_inputUnit && m_outputUnit)
			{
				Eigen::Vector2i inputPos = m_inputUnit->getInputPortPosition(m_inputPort);
				Eigen::Vector2i outputPos = m_outputUnit->getOutputPortPosition(m_outputPort);
				startPos = m_startFromOutput ? outputPos : inputPos;
				endPos = m_startFromOutput ? inputPos : outputPos;
			}else if(m_startFromOutput)
			{
				startPos = m_outputUnit->getOutputPortPosition(m_outputPort);
				endPos = endPt;				
			}else
			{
				startPos = m_inputUnit->getInputPortPosition(m_inputPort);
				endPos = endPt;					
			}

			// Draw lines
			nvgSave(ctx);
			nvgBeginPath(ctx);
			nvgMoveTo(ctx, startPos.x(), startPos.y());
			for (auto pt : m_pts) { nvgLineTo(ctx, pt.x(), pt.y()); }
			nvgLineTo(ctx, endPos.x(), endPos.y());
			nvgStroke(ctx);
			nvgRestore(ctx);
		}

		void addPoint(const Eigen::Vector2i& a_pt)
		{
			if (std::find(m_pts.begin(), m_pts.end(), a_pt) == m_pts.end())
				m_pts.push_back(a_pt);
		}

		void setInputPort(synui::UnitWidget* a_inputUnit, int a_inputPort)
		{
			m_inputUnit = a_inputUnit;
			m_inputPort = a_inputPort;
		}

		void setOutputPort(synui::UnitWidget* a_outputUnit, int a_outputPort)
		{
			m_outputUnit = a_outputUnit;
			m_outputPort = a_outputPort;
		}

	private:
		synui::UnitWidget* m_inputUnit;
		int m_inputPort;
		synui::UnitWidget* m_outputUnit;
		int m_outputPort;
		bool m_startFromOutput;
		std::vector<Eigen::Vector2i> m_pts;
	};
}

synui::CircuitWidget::CircuitWidget(nanogui::Widget* a_parent, synui::MainWindow* a_mainWindow, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf) :
	Widget(a_parent),
	m_window(a_mainWindow),
	m_uf(a_uf),
	m_vm(a_vm),
	m_gridSize(15),
	m_state(State::Idle),
	m_placingUnitState({0,nullptr}),
	m_drawingWireState{{0,0},{0,0},false,nullptr} { }

bool synui::CircuitWidget::mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers)
{
	if (Widget::mouseButtonEvent(p, button, down, modifiers))
		return true;

	if (m_state == State::PlacingUnit)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && down)
		{
			m_placingUnitState.widget->setVisible(true);
			m_placingUnitState.widget = nullptr;
			m_state = State::Idle;
			return false;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && down)
		{
			m_unitWidgets.erase(m_placingUnitState.unitId);
			removeChild(m_placingUnitState.widget);
			m_placingUnitState.widget = nullptr;
			m_state = State::Idle;
			return false;
		}
	}

	if (m_state == State::DrawingWire && !down)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && !down)
		{
			// Force wire creation to fail when the user lets the mouse up over a blank space
			endWireDraw_(0, 0, m_drawingWireState.startedFromOutput);
		}
	}
	return false;
}

void synui::CircuitWidget::draw(NVGcontext* ctx)
{
	// Draw grid
	nvgSave(ctx);
	nvgTranslate(ctx, mPos.x(), mPos.y());
	int gX = 0, gY = 0;
	nvgStrokeWidth(ctx, 1.0f);
	nvgStrokeColor(ctx, nanogui::Color{1.0f,0.1f});
	while (gX <= mSize.x())
	{
		nvgBeginPath(ctx);
		nvgMoveTo(ctx, gX, 0);
		nvgLineTo(ctx, gX, mSize.y());
		nvgStroke(ctx);
		gX += m_gridSize;
	}
	while (gY <= mSize.y())
	{
		nvgBeginPath(ctx);
		nvgMoveTo(ctx, 0, gY);
		nvgLineTo(ctx, mSize.x(), gY);
		nvgStroke(ctx);
		gY += m_gridSize;
	}

	if (mMouseFocus)
	{
		Vector2i mousePos = m_window->gui()->screen()->mousePos() - absolutePosition();
		// Draw unit widget ghost if one is being placed
		if (m_state == State::PlacingUnit)
		{
			m_placingUnitState.widget->setPosition(fixToGrid(mousePos));
			m_placingUnitState.widget->draw(ctx);
		}
		// Draw wire widget ghost if one is being placed
		else
		if (m_state == State::DrawingWire)
		{
			m_drawingWireState.wire->addPoint(fixToGrid(mousePos));
			m_drawingWireState.wire->draw(ctx, mousePos);
		}
	}

	// Draw wires
	for (auto wire : m_wires) { wire->draw(ctx); }
	nvgRestore(ctx);

	Widget::draw(ctx);
}

void synui::CircuitWidget::loadPrototype(const std::string& a_unitPrototype) { createUnit_(a_unitPrototype); }

Eigen::Vector2i synui::CircuitWidget::fixToGrid(const Eigen::Vector2i& a_pixelLocation) const
{
	Eigen::Vector2i gridLocation{round(a_pixelLocation.x() * (1.0 / m_gridSize)), round(a_pixelLocation.y() * (1.0 / m_gridSize))};
	return gridLocation * m_gridSize;
}

void synui::CircuitWidget::createUnit_(const std::string& a_unitPrototypeName)
{
	syn::RTMessage* msg = new syn::RTMessage();
	msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
			{
				CircuitWidget* self;
				syn::UnitFactory* unitFactory;
				syn::Unit* unit;
				GetArgs(a_data, 0, self, unitFactory, unit);
				int unitId = a_circuit->addUnit(unit->clone());
				// Queue return message
				if (a_isLast)
				{
					GUIMessage* msg = new GUIMessage;
					msg->action = [](MainWindow* a_win, ByteChunk* a_data)
							{
								CircuitWidget* self;
								int unitId;
								GetArgs(a_data, 0, self, unitId);
								self->onUnitCreated_(unitId);
							};
					PutArgs(&msg->data, self, unitId);
					self->m_window->queueExternalMessage(msg);
					delete unit;
				}
			};

	auto self = this;
	auto unit = m_uf->createUnit(a_unitPrototypeName);
	PutArgs(&msg->data, self, m_uf, unit);
	m_vm->queueAction(msg);
}

void synui::CircuitWidget::onUnitCreated_(int a_unitId)
{
	m_state = State::PlacingUnit;
	m_placingUnitState.unitId = a_unitId;
	m_placingUnitState.widget = new UnitWidget(this, a_unitId);
	m_window->gui()->screen()->performLayout();
	m_placingUnitState.widget->setVisible(false);
	m_unitWidgets[a_unitId] = m_placingUnitState.widget;
}

void synui::CircuitWidget::createConnection_(const Port& a_inputPort, const Port& a_outputPort)
{
	// send the new connection request to the real-time thread
	syn::RTMessage* msg = new syn::RTMessage();
	msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
			{
				CircuitWidget* self;
				int fromUnit, fromPort, toUnit, toPort;
				GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
				a_circuit->connectInternal(fromUnit, fromPort, toUnit, toPort);

				// Queue return message
				if (a_isLast)
				{
					GUIMessage* msg = new GUIMessage;
					msg->action = [](MainWindow* a_win, ByteChunk* a_data)
							{
								CircuitWidget* self;
								int fromUnit, fromPort, toUnit, toPort;
								GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
								self->onConnectionCreated_({fromUnit, fromPort}, {toUnit, toPort});
							};
					PutArgs(&msg->data, self, fromUnit, fromPort, toUnit, toPort);
					self->m_window->queueExternalMessage(msg);
				}
			};

	CircuitWidget* self = this;
	PutArgs(&msg->data, self, a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);
	m_vm->queueAction(msg);
}

void synui::CircuitWidget::onConnectionCreated_(const Port& a_inputPort, const Port& a_outputPort) { m_state = State::Idle; }

void synui::CircuitWidget::startWireDraw_(int a_unitId, int a_portId, bool a_isOutput)
{
	m_state = State::DrawingWire;
	m_drawingWireState.startedFromOutput = a_isOutput;
	m_drawingWireState.wire = new CircuitWire(a_isOutput);

	if (a_isOutput){
		m_drawingWireState.outputPort = {a_unitId, a_portId};
		m_drawingWireState.wire->setOutputPort(m_unitWidgets[a_unitId], a_portId);
	}
	else{
		m_drawingWireState.inputPort = {a_unitId, a_portId};
		m_drawingWireState.wire->setInputPort(m_unitWidgets[a_unitId], a_portId);
	}
}

void synui::CircuitWidget::endWireDraw_(int a_unitId, int a_portId, bool a_isOutput)
{
	if (m_state != State::DrawingWire)
		return;
	m_state = State::Idle;
	// Reset state if connection is invalid
	if (a_isOutput == m_drawingWireState.startedFromOutput)
	{
		delete m_drawingWireState.wire;
		m_drawingWireState.wire = nullptr;
		return;
	}

	createConnection_(m_drawingWireState.inputPort, m_drawingWireState.outputPort);
	if(a_isOutput)
		m_drawingWireState.wire->setOutputPort(m_unitWidgets[a_unitId], a_portId);
	else
		m_drawingWireState.wire->setInputPort(m_unitWidgets[a_unitId], a_portId);
	
	m_wires.push_back(m_drawingWireState.wire);
	m_drawingWireState.wire = nullptr;
}
