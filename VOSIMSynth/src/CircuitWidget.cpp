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

        explicit CircuitWire(CircuitWidget* a_parentCircuit)
            : highlight(false),
            m_parentCircuit(a_parentCircuit),
            m_inputPort({ -1,-1 }),
            m_outputPort({ -1,-1 }),
            m_start({ -1,-1 }), m_end({ -1,-1 }) { }

        virtual ~CircuitWire() { m_parentCircuit->m_grid.replaceValue({ CircuitWidget::GridCell::Wire, this }, m_parentCircuit->m_grid.getEmptyValue()); }

        void draw(NVGcontext* ctx)
        {
            nvgSave(ctx);
            nvgBeginPath(ctx);
            // If wire is incomplete (still being drawn), draw a "ghost"
            if (m_inputPort.first < 0 || m_outputPort.first < 0)
            {
                nvgStrokeWidth(ctx, 2.0f);
                nvgStrokeColor(ctx, nanogui::Color(1.0f, 0.0f, 0.0f, 0.3f));
            }
            else
            {
                nvgStrokeWidth(ctx, 2.0f);
                if (highlight)
                    nvgStrokeColor(ctx, nanogui::Color(1.0f, 1.0f, 1.0f, 0.55f));
                else
                    nvgStrokeColor(ctx, nanogui::Color(1.0f, 0.0f, 0.0f, 0.55f));
            }
            nvgMoveTo(ctx, m_start.x(), m_start.y());
            for (auto pt : m_path)
            {
                pt = m_parentCircuit->m_grid.toPixel(pt, m_parentCircuit->gridSpacing());
                nvgLineTo(ctx, pt.x(), pt.y());
            }
            nvgLineTo(ctx, m_end.x(), m_end.y());
            nvgStroke(ctx);

            nvgRestore(ctx);
        }

        /**
         * \brief Determine and update the start and end points of the wire.
         * If both the input and output port have been set, then the start and end points are simply the
         * location of those ports. If only one of them has been set, the location of that port is the start
         * point, and the `a_endPt` argument is used to set the end point.
         * \param a_endPt End point to use in the case that one of the ports has not been set.
         */
        void updateStartAndEndPositions(const Eigen::Vector2i& a_endPt = { -1,-1 })
        {
            // Determine start/end points
            Eigen::Vector2i startPos;
            Eigen::Vector2i endPos = m_end;
            if (m_inputPort.first >= 0 && m_outputPort.first >= 0)
            {
                auto inputWidget = m_parentCircuit->m_unitWidgets[m_inputPort.first];
                auto outputWidget = m_parentCircuit->m_unitWidgets[m_outputPort.first];
                Eigen::Vector2i inputPos = inputWidget->getInputPortPosition(m_inputPort.second);
                Eigen::Vector2i outputPos = outputWidget->getOutputPortPosition(m_outputPort.second);
                startPos = outputPos;
                endPos = inputPos;
            }
            else if (m_outputPort.first >= 0)
            {
                auto outputWidget = m_parentCircuit->m_unitWidgets[m_outputPort.first];
                startPos = outputWidget->getOutputPortPosition(m_outputPort.second);
                endPos = a_endPt;
            }
            else if (m_inputPort.first >= 0)
            {
                auto inputWidget = m_parentCircuit->m_unitWidgets[m_inputPort.first];
                startPos = inputWidget->getInputPortPosition(m_inputPort.second);
                endPos = a_endPt;
            }

            m_start = startPos = m_parentCircuit->fixToGrid(startPos);
            m_end = endPos = m_parentCircuit->fixToGrid(endPos);
        }

        /**
         * \brief Refresh the wire's path.
         */
        void updatePath()
        {
            struct weight_func
            {
                int operator()(const Grid2DEdge& edge, const CircuitWidget::GridCell& fromValue, const CircuitWidget::GridCell& toValue) const
                {
                    switch (toValue.state)
                    {
                    case CircuitWidget::GridCell::Empty:
                        return 1;
                        break;
                    case CircuitWidget::GridCell::Wire:
                        return 15;
                        break;
                    case CircuitWidget::GridCell::Unit:
                        return 20;
                        break;
                    default:
                        return 1;
                        break;
                    }
                }
            };

            updateStartAndEndPositions(m_end);

            auto startCell = m_parentCircuit->m_grid.fromPixel(m_start, m_parentCircuit->gridSpacing());
            auto endCell = m_parentCircuit->m_grid.fromPixel(m_end, m_parentCircuit->gridSpacing());

            m_path = m_parentCircuit->m_grid.findPath<Grid2D<CircuitWidget::GridCell>::manhattan_distance, weight_func>(startCell, endCell);

            // Update grid to reflect new location
            m_parentCircuit->m_grid.replaceValue({ CircuitWidget::GridCell::Wire, this }, m_parentCircuit->m_grid.getEmptyValue());
            for (auto cell : m_path) { m_parentCircuit->m_grid.get(cell) = { CircuitWidget::GridCell::Wire, this }; }
        }

        void setInputPort(const CircuitWidget::Port& a_port)
        {
            m_inputPort = a_port;
            updateStartAndEndPositions();
        }

        void setOutputPort(const CircuitWidget::Port& a_port)
        {
            m_outputPort = a_port;
            updateStartAndEndPositions();
        }

        CircuitWidget::Port getInputPort() const { return m_inputPort; }
        CircuitWidget::Port getOutputPort() const { return m_outputPort; }

        bool highlight;
    private:
        CircuitWidget* m_parentCircuit;
        CircuitWidget::Port m_inputPort;
        CircuitWidget::Port m_outputPort;
        Eigen::Vector2i m_start;
        Eigen::Vector2i m_end;
        std::list<Grid2DPoint> m_path;
    };
}

synui::CircuitWidget::CircuitWidget(nanogui::Widget* a_parent, synui::MainWindow* a_mainWindow, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf) :
    Widget(a_parent),
    m_window(a_mainWindow),
    m_uf(a_uf),
    m_vm(a_vm),
    m_grid{ {1,1}, GridCell{GridCell::Empty, nullptr} },
    m_gridSpacing(20),
    m_state(State::Uninitialized),
    m_placingUnitState({ 0,nullptr }),
    m_drawingWireState{ false,nullptr },
    m_highlightedWire{ nullptr }
{
    registerUnitWidget<syn::InputUnit>([](CircuitWidget* parent, int unitId) { return new InputUnitWidget(parent, unitId); });
    registerUnitWidget<syn::OutputUnit>([](CircuitWidget* parent, int unitId) { return new OutputUnitWidget(parent, unitId); });
}

bool synui::CircuitWidget::mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers)
{
    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;

    if (m_state == State::Idle)
    {
        if (m_highlightedWire)
        {
            if (button == GLFW_MOUSE_BUTTON_RIGHT && !down)
            {
                deleteConnection_(m_highlightedWire->getInputPort(), m_highlightedWire->getOutputPort());
                m_highlightedWire = nullptr;
                return true;
            }
        }
    }

    if (m_state == State::PlacingUnit)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && down)
        {
            endPlaceUnit_(p - position());
            return true;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && down)
        {
            m_unitWidgets.erase(m_placingUnitState.unitId);
            removeChild(m_placingUnitState.widget);
            m_placingUnitState.widget = nullptr;
            m_state = State::Idle;
            return true;
        }
    }

    if (m_state == State::DrawingWire)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && !down)
        {
            // Force wire creation to fail when the user lets the mouse up over a blank space
            endWireDraw_(0, 0, m_drawingWireState.startedFromOutput);
            return true;
        }
    }
    return false;
}

bool synui::CircuitWidget::mouseMotionEvent(const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers)
{
    if (Widget::mouseMotionEvent(p, rel, button, modifiers))
        return true;
    if (m_highlightedWire)
        m_highlightedWire->highlight = false;
    Grid2DPoint cell = m_grid.fromPixel(p, m_gridSpacing);
    if (m_grid.contains(cell) && m_grid.get(cell).state == GridCell::Wire)
    {
        m_highlightedWire = static_cast<CircuitWire*>(m_grid.get(cell).ptr);
        m_highlightedWire->highlight = true;
        return true;
    }
    return false;
}

void synui::CircuitWidget::draw(NVGcontext* ctx)
{
    // Perform first draw init
    if (m_state == State::Uninitialized)
    {
        createInputOutputUnits_();
        m_state = State::Idle;
    }
    // Draw grid
    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());
    for (int i = 0; i < m_grid.getSize(); i++)
    {
        auto pt = m_grid.unravel_index(i);
        auto pixel = m_grid.toPixel(pt, m_gridSpacing);
        if (m_grid.get(pt).state == GridCell::Empty)
            nvgFillColor(ctx, nanogui::Color{ 1.0f,0.125f });
        else if (m_grid.get(pt).state == GridCell::Unit)
            nvgFillColor(ctx, nanogui::Color{ 0.0f,0.125f });
        else if (m_grid.get(pt).state == GridCell::Wire)
            nvgFillColor(ctx, nanogui::Color{ 0.0f,0.0f,1.0f,0.125f });

        nvgBeginPath(ctx);
        nvgCircle(ctx, pixel.x(), pixel.y(), 1.0f);
        nvgFill(ctx);
    }

    if (mMouseFocus)
    {
        Vector2i mousePos = m_window->gui()->screen()->mousePos() - absolutePosition();
        // Draw unit widget ghost if one is being placed
        if (m_state == State::PlacingUnit)
        {
            UnitWidget* w = m_placingUnitState.widget;
            //m_placingUnitState.widget->setPosition(fixToGrid(mousePos));
            nvgBeginPath(ctx);
            nvgStrokeColor(ctx, nanogui::Color(0.7f, 0.7f, 0.7f, 0.75f));
            nvgRect(ctx, mousePos[0], mousePos[1], w->width(), w->height());
            nvgStroke(ctx);
        }
        // Draw wire ghost if one is being placed
        else if (m_state == State::DrawingWire)
        {
            m_drawingWireState.wire->updateStartAndEndPositions(mousePos);
            m_drawingWireState.wire->draw(ctx);
        }
    }

    // Draw wires
    for (auto wire : m_wires) { wire->draw(ctx); }
    nvgRestore(ctx);

    Widget::draw(ctx);
}

void synui::CircuitWidget::loadPrototype(const std::string& a_unitPrototype) { createUnit_(a_unitPrototype); }

Eigen::Vector2i synui::CircuitWidget::fixToGrid(const Eigen::Vector2i& a_pixelLocation) const { return m_grid.toPixel(m_grid.fromPixel(a_pixelLocation, m_gridSpacing), m_gridSpacing); }

void synui::CircuitWidget::performLayout(NVGcontext* ctx)
{
    Widget::performLayout(ctx);
    m_grid.resize(m_grid.fromPixel(size(), m_gridSpacing)); // TODO: Ensure resize occurs if size is changed outside of performLayout.
}

void synui::CircuitWidget::createInputOutputUnits_()
{
    const syn::Circuit* circ = m_vm->getPrototypeCircuit();
    int inputUnitId = circ->getInputUnitId();
    unsigned inputClassId = circ->getUnit(inputUnitId).getClassIdentifier();
    int outputUnitId = circ->getOutputUnitId();
    unsigned outputClassId = circ->getUnit(outputUnitId).getClassIdentifier();
    onUnitCreated_(inputClassId, inputUnitId);
    endPlaceUnit_({ 0,height() * 0.5 - m_placingUnitState.widget->height() * 0.5 });

    onUnitCreated_(outputClassId, outputUnitId);
    endPlaceUnit_({ width() - 1.5 * m_placingUnitState.widget->width(),height() * 0.5 - m_placingUnitState.widget->height() * 0.5 });
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
        unsigned classId = unit->getClassIdentifier();
        // Queue return message
        if (a_isLast)
        {
            GUIMessage* msg = new GUIMessage;
            msg->action = [](MainWindow* a_win, ByteChunk* a_data)
            {
                CircuitWidget* self;
                unsigned classId;
                int unitId;
                GetArgs(a_data, 0, self, classId, unitId);
                self->onUnitCreated_(classId, unitId);
            };
            PutArgs(&msg->data, self, classId, unitId);
            self->m_window->queueExternalMessage(msg);
            delete unit;
        }
    };

    auto self = this;
    auto unit = m_uf->createUnit(a_unitPrototypeName);
    PutArgs(&msg->data, self, m_uf, unit);
    m_vm->queueAction(msg);
}

void synui::CircuitWidget::onUnitCreated_(unsigned a_classId, int a_unitId)
{
    m_state = State::PlacingUnit;
    m_placingUnitState.unitId = a_unitId;
    if (m_registeredUnitWidgets.find(a_classId) == m_registeredUnitWidgets.end())
        m_placingUnitState.widget = new UnitWidget(this, a_unitId);
    else
        m_placingUnitState.widget = m_registeredUnitWidgets[a_classId](this, a_unitId);
    m_placingUnitState.widget->setVisible(false);
    m_placingUnitState.widget->setEnabled(false);
    m_unitWidgets[a_unitId] = m_placingUnitState.widget;
    m_window->gui()->screen()->performLayout();
}

void synui::CircuitWidget::endPlaceUnit_(const Eigen::Vector2i& a_pos)
{
    if (m_state != State::PlacingUnit)
        return;
    m_placingUnitState.widget->setVisible(true);
    m_placingUnitState.widget->setEnabled(true);
    bool result = updateUnitPos_(m_placingUnitState.unitId, a_pos);
    if (!result)
    {
        m_unitWidgets.erase(m_placingUnitState.unitId);
        removeChild(m_placingUnitState.widget);
    }
    m_placingUnitState.widget = nullptr;
    m_state = State::Idle;
}

void synui::CircuitWidget::deleteUnit_(int a_unitId)
{
    // Disallow removing the input and output units.
    if (a_unitId == m_vm->getPrototypeCircuit()->getInputUnitId() || a_unitId == m_vm->getPrototypeCircuit()->getOutputUnitId())
        return;

    // Delete the wire widgets connected to this unit
    for (int i = 0; i < m_wires.size(); i++)
    {
        auto wire = m_wires[i];
        if (wire->getInputPort().first == a_unitId || wire->getOutputPort().first == a_unitId)
        {
            deleteWire_(wire);
            i--;
        }
    }

    // Delete the corresponding widget
    m_grid.replaceValue({ GridCell::Unit, m_unitWidgets[a_unitId] }, m_grid.getEmptyValue());
    removeChild(m_unitWidgets[a_unitId]);
    m_unitWidgets.erase(a_unitId);

    // Delete the unit from the circuit
    syn::RTMessage* msg = new syn::RTMessage();
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
    {
        int unitId;
        GetArgs(a_data, 0, unitId);
        a_circuit->removeUnit(unitId);
    };

    PutArgs(&msg->data, a_unitId);
    m_vm->queueAction(msg);
}

void synui::CircuitWidget::deleteConnection_(const Port& a_inputPort, const Port& a_outputPort)
{
    // Delete the wire widget
    bool foundWire = false;
    for (int i = 0; i < m_wires.size(); i++)
    {
        auto wire = m_wires[i];
        if (wire->getInputPort() == a_inputPort && wire->getOutputPort() == a_outputPort)
        {
            deleteWire_(wire);
            foundWire = true;
            break;
        }
    }

    // Abort action if wire is not found
    if (!foundWire)
        return;

    // Send message to RT thread to delete the connection
    syn::RTMessage* msg = new syn::RTMessage();
    CircuitWidget* self = this;
    PutArgs(&msg->data, self, a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);

    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
    {
        CircuitWidget* self;
        int fromUnit, fromPort, toUnit, toPort;
        GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
        a_circuit->disconnectInternal(fromUnit, fromPort, toUnit, toPort);
    };

    m_vm->queueAction(msg);
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
                self->onConnectionCreated_({ toUnit, toPort }, { fromUnit, fromPort });
            };
            PutArgs(&msg->data, self, fromUnit, fromPort, toUnit, toPort);
            self->m_window->queueExternalMessage(msg);
        }
    };

    CircuitWidget* self = this;
    PutArgs(&msg->data, self, a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);
    m_vm->queueAction(msg);
}

void synui::CircuitWidget::onConnectionCreated_(const Port& a_inputPort, const Port& a_outputPort)
{
    // Remove any wire widgets that already point to that unit input
    for (int i = 0; i < m_wires.size(); i++)
    {
        auto wire = m_wires[i];
        if (wire->getInputPort() == a_inputPort)
        {
            deleteWire_(wire);
            i--;
        }
    }

    m_state = State::Idle;
    m_drawingWireState.wire->updatePath();
    m_wires.push_back(m_drawingWireState.wire);
    m_drawingWireState.wire = nullptr;
}

void synui::CircuitWidget::startWireDraw_(int a_unitId, int a_portId, bool a_isOutput)
{
    if (m_state != State::Idle)
        return;
    m_state = State::DrawingWire;
    m_drawingWireState.startedFromOutput = a_isOutput;
    m_drawingWireState.wire = new CircuitWire(this);

    if (a_isOutput)
        m_drawingWireState.wire->setOutputPort({ a_unitId, a_portId });
    else
        m_drawingWireState.wire->setInputPort({ a_unitId, a_portId });
}

void synui::CircuitWidget::endWireDraw_(int a_unitId, int a_portId, bool a_isOutput)
{
    if (m_state != State::DrawingWire)
        return;
    CircuitWire* wire = m_drawingWireState.wire;
    // If connection is invalid, delete the wire and reset state.
    if (a_isOutput == m_drawingWireState.startedFromOutput ||
        wire->getInputPort().first == a_unitId || wire->getOutputPort().first == a_unitId)
    {
        m_state = State::Idle;
        delete wire;
        m_drawingWireState.wire = nullptr;
    }
    else
    {
        if (a_isOutput)
            wire->setOutputPort({ a_unitId, a_portId });
        else
            wire->setInputPort({ a_unitId, a_portId });
        createConnection_(wire->getInputPort(), wire->getOutputPort());
    }
}

bool synui::CircuitWidget::updateUnitPos_(int a_unitId, const Eigen::Vector2i& a_newPos)
{
    UnitWidget* unitWidget = m_unitWidgets[a_unitId];
    // save current grid in case operation fails
    m_grid.save();
    // erase unit from grid
    m_grid.replaceValue({ GridCell::Unit, unitWidget }, m_grid.getEmptyValue());
    // compute the grid cells of the unit's boundaries 
    auto topLeftCell = m_grid.fromPixel(a_newPos, m_gridSpacing);
    auto bottomRightCell = m_grid.fromPixel(a_newPos + unitWidget->size(), m_gridSpacing) + Vector2i::Ones();

    // Check that the unit's boundaries don't extend off the grid
    if (m_grid.contains(topLeftCell, bottomRightCell)) {
        // If there are only wires in the way, place the unit and reroute the wires
        auto blk = m_grid.getBlock(topLeftCell, bottomRightCell);
        if (!blk.unaryExpr([](const GridCell& x) { return x.state != GridCell::Unit ? GridCell{ GridCell::Empty, nullptr } : x; }).array().any())
        {
            std::unordered_set<CircuitWire*> wires;
            // Record any wires we may be overwriting
            for (int r = 0; r < blk.rows(); r++)
            {
                for (int c = 0; c < blk.cols(); c++)
                {
                    const GridCell& cellContents = blk(r, c);
                    if (cellContents.state == GridCell::Wire)
                        wires.insert(static_cast<CircuitWire*>(cellContents.ptr));
                }
            }
            // Place the unit
            m_grid.setBlock(topLeftCell, bottomRightCell, { GridCell::Unit, unitWidget }, true);
            unitWidget->setPosition(m_grid.toPixel(topLeftCell, m_gridSpacing));
            // Reroute wires
            for (auto wire : wires)
                wire->updatePath();
            for (auto wire : m_wires)
            {
                if (wires.find(wire) != wires.end())
                    continue;
                if (wire->getInputPort().first == a_unitId || wire->getOutputPort().first == a_unitId)
                    wire->updatePath();
            }
            return true;
        }
    }
    m_grid.restore();
    return false;
}

void synui::CircuitWidget::deleteWire_(CircuitWire* wire)
{
    if (!wire)
        return;
    m_wires.erase(std::find(m_wires.begin(), m_wires.end(), wire));
    if (m_highlightedWire == wire)
        m_highlightedWire = nullptr;
    delete wire;
}
