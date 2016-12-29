#include "CircuitWidget.h"
#include "VoiceManager.h"
#include "UI.h"
#include "MainWindow.h"
#include "UnitWidget.h"
#include "MainGUI.h"
#include "UnitEditor.h"
#include "CircuitWidget.h"

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
            m_start({ -1,-1 }), m_end({ -1,-1 }),
            m_isFinal(false) { }

        virtual ~CircuitWire() { m_parentCircuit->m_grid.replaceValue({ CircuitWidget::GridCell::Wire, this }, m_parentCircuit->m_grid.getEmptyValue()); }

        std::string info() const
        {
            if (!m_isFinal)
                return "";
            const syn::Unit& unit = m_parentCircuit->m_vm->getUnit(m_outputPort.first, m_parentCircuit->m_vm->getNewestVoiceIndex());
            double value = unit.readOutput(m_outputPort.second);
            std::ostringstream os;
            os << std::setprecision(4) << value;
            return os.str();
        }

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
                m_isFinal = true;
                auto inputWidget = m_parentCircuit->m_unitWidgets[m_inputPort.first];
                auto outputWidget = m_parentCircuit->m_unitWidgets[m_outputPort.first];
                Eigen::Vector2i inputPos = inputWidget->getInputPortAbsPosition(m_inputPort.second) - m_parentCircuit->absolutePosition();
                Eigen::Vector2i outputPos = outputWidget->getOutputPortAbsPosition(m_outputPort.second) - m_parentCircuit->absolutePosition();
                startPos = outputPos;
                endPos = inputPos;
            }
            else if (m_outputPort.first >= 0)
            {
                auto outputWidget = m_parentCircuit->m_unitWidgets[m_outputPort.first];
                startPos = outputWidget->getOutputPortAbsPosition(m_outputPort.second) - m_parentCircuit->absolutePosition();
                endPos = a_endPt;
            }
            else if (m_inputPort.first >= 0)
            {
                auto inputWidget = m_parentCircuit->m_unitWidgets[m_inputPort.first];
                startPos = inputWidget->getInputPortAbsPosition(m_inputPort.second) - m_parentCircuit->absolutePosition();
                endPos = a_endPt;
            }

            m_start = startPos = m_parentCircuit->fixToGrid(startPos);
            m_end = endPos = m_parentCircuit->fixToGrid(endPos);
        }

        /**
         * \brief Weighting function used for pathfinding.
         *
         * Weights straight paths higher than jagged ones, and prefers crossing wires instead over going through units.
         */
        template <typename CellType>
        struct weight_func
        {
            int operator()(const Grid2D<CellType>& grid, const Grid2DPoint& prev, const Grid2DPoint& curr, const Grid2DPoint& next) const
            {
                int score = 0;
                // Weight jagged paths more than straight ones
                if ((prev.array() > -1).all())
                {
                    if (((curr - prev).array() != (next - curr).array()).any())
                        score += 1;
                }

                // Prefer empty over a wire, and prefer a wire over a unit.
                switch (grid.get(next).state)
                {
                case CircuitWidget::GridCell::Empty:
                    score += 1;
                    break;
                case CircuitWidget::GridCell::Wire:
                    score += 5;
                    break;
                case CircuitWidget::GridCell::Unit:
                    score += 55;
                    break;
                default:
                    score += 1;
                    break;
                }
                return score;
            }
        };

        /**
         * \brief Refresh the wire's path.
         */
        void updatePath()
        {
            updateStartAndEndPositions(m_end);

            auto startCell = m_parentCircuit->m_grid.fromPixel(m_start, m_parentCircuit->gridSpacing());
            auto endCell = m_parentCircuit->m_grid.fromPixel(m_end, m_parentCircuit->gridSpacing());

            m_path = m_parentCircuit->m_grid.findPath<manhattan_distance, weight_func>(startCell, endCell);

            // Update grid to reflect new location
            m_parentCircuit->m_grid.replaceValue({ CircuitWidget::GridCell::Wire, this }, m_parentCircuit->m_grid.getEmptyValue());
            for (auto cell : m_path)
            {
                if (!m_parentCircuit->m_grid.isOccupied(cell))
                    m_parentCircuit->m_grid.get(cell) = { CircuitWidget::GridCell::Wire, this };
            }
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

        operator json() const
        {
            json j;
            j["input"] = { m_inputPort.first, m_inputPort.second };
            j["output"] = { m_outputPort.first, m_outputPort.second };
            return j;
        }

        CircuitWire* load(const json& j)
        {
            setInputPort({ j["input"][0], j["input"][1] });
            setOutputPort({ j["output"][0], j["output"][1] });
            updatePath();
            return this;
        }

        bool highlight;
    private:
        CircuitWidget* m_parentCircuit;
        CircuitWidget::Port m_inputPort;
        CircuitWidget::Port m_outputPort;
        Eigen::Vector2i m_start;
        Eigen::Vector2i m_end;
        std::list<Grid2DPoint> m_path;
        bool m_isFinal;
    };
}

synui::CircuitWidget::CircuitWidget(nanogui::Widget* a_parent, synui::MainWindow* a_mainWindow, synui::UnitEditorHost* a_unitEditorHost, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf) :
    Widget(a_parent),
    m_window(a_mainWindow),
    m_uf(a_uf),
    m_vm(a_vm),
    m_grid{ {0,0}, GridCell{GridCell::Empty, nullptr} },
    m_gridSpacing(10),
    m_state(State::Uninitialized),
    m_placingUnitState{ 0,nullptr },
    m_drawingWireState{ false,nullptr },
    m_highlightedWire{ nullptr },
    m_unitEditorHost(a_unitEditorHost)
{
    registerUnitWidget<syn::InputUnit>([](CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new InputUnitWidget(parent, a_vm, unitId); });
    registerUnitWidget<syn::OutputUnit>([](CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new OutputUnitWidget(parent, a_vm, unitId); });
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

    Grid2DPoint cell = m_grid.fromPixel(p, m_gridSpacing);

    if (m_grid.contains(cell) && m_grid.get(cell).state == GridCell::Wire)
    {
        if (m_grid.get(cell).ptr != m_highlightedWire)
        {
            if (m_highlightedWire)
            {
                m_highlightedWire->highlight = false;
            }
            m_wireHighlightTime = glfwGetTime();
            m_highlightedWire = static_cast<CircuitWire*>(m_grid.get(cell).ptr);
            m_highlightedWire->highlight = true;
        }
        return true;
    }
    else if (m_highlightedWire)
    {
        m_highlightedWire->highlight = false;
        m_highlightedWire = nullptr;
    }
    return false;
}

void synui::CircuitWidget::draw(NVGcontext* ctx)
{
    /* Perform first draw init */
    if (m_state == State::Uninitialized)
    {
        resizeGrid(m_gridSpacing);
        createInputOutputUnits_();
        m_state = State::Idle;
    }

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());

    /* Draw background */
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
    nvgFillColor(ctx, mTheme->mWindowFillUnfocused);
    nvgFill(ctx);

    /* Draw grid */
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

    /* Handle mouse hover */
    if (mMouseFocus)
    {
        Vector2i mousePos = m_window->getGUI()->getScreen()->mousePos() - absolutePosition();
        // Draw unit widget ghost if one is being placed
        if (m_state == State::PlacingUnit)
        {
            UnitWidget* w = m_placingUnitState.widget;
            m_placingUnitState.isValid = checkUnitPos_(m_placingUnitState.widget, mousePos);
            nvgBeginPath(ctx);
            if (m_placingUnitState.isValid)
                nvgStrokeColor(ctx, nanogui::Color(0.7f, 0.7f, 0.7f, 0.75f));
            else
                nvgStrokeColor(ctx, nanogui::Color(0.7f, 0.0f, 0.0f, 0.75f));
            nvgRect(ctx, mousePos[0], mousePos[1], w->width(), w->height());
            nvgStroke(ctx);
        }
        // Draw wire ghost if one is being placed
        else if (m_state == State::DrawingWire)
        {
            m_drawingWireState.wire->updateStartAndEndPositions(mousePos);
            m_drawingWireState.wire->draw(ctx);
        }
        else if (m_state == State::Idle)
        {
            double elapsed = glfwGetTime() - m_wireHighlightTime;
            if (m_highlightedWire)
            {
                if (elapsed > 0.5)
                {
                    nvgSave(ctx);
                    drawTooltip(ctx, { mousePos.x(),mousePos.y() + 25 }, m_highlightedWire->info(), elapsed);
                    nvgRestore(ctx);
                }
            }
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
}

void synui::CircuitWidget::resizeGrid(int a_newGridSpacing)
{
    Vector2i newGridShape = m_grid.fromPixel(size(), a_newGridSpacing);
    if (m_gridSpacing == a_newGridSpacing && (newGridShape.array() == m_grid.getShape().array()).all())
        return;
    m_gridSpacing = a_newGridSpacing;
    m_grid.resize(m_grid.fromPixel(size(), m_gridSpacing));

    for (auto unit : m_unitWidgets)
        unit.second->updateRowSizes_();

    performScreenLayout_();

    m_grid.reset();
    for (auto unit : m_unitWidgets)
        updateUnitPos_(unit.second, unit.second->position(), true);
}

synui::CircuitWidget::operator json() const
{
    json j;
    j["grid_spacing"] = m_gridSpacing;
    json& wires = j["wires"] = json();
    for (CircuitWire* w : m_wires) { wires.push_back(w->operator json()); }
    json& units = j["units"] = json();
    for (auto& u : m_unitWidgets) { units[std::to_string(u.first)] = u.second->operator json(); }
    return j;
}

synui::CircuitWidget* synui::CircuitWidget::load(const json& j)
{
    /* Reset to clean state */
    reset();

    /* Load grid spacing */
    resizeGrid(j["grid_spacing"].get<int>());

    /* Load new unit widgets */
    const json& units = j["units"];
    const syn::Circuit* circ = m_vm->getPrototypeCircuit();
    for (json::const_iterator it = units.cbegin(); it != units.cend(); ++it)
    {
        int unitId = stoi(it.key());
        unsigned classId = circ->getUnit(unitId).getClassIdentifier();
        const json& unit = it.value();
        Vector2i pos{ unit["x"].get<int>(), unit["y"].get<int>() };
        UnitWidget* widget = createUnitWidget_(classId, unitId);
        updateUnitPos_(widget, pos);
        m_unitWidgets[unitId]->load(j);
    }

    /* Load new wires */
    const json& wires = j["wires"];
    for (const json& w : wires)
    {
        CircuitWire* newWire = new CircuitWire(this);
        m_wires.push_back(newWire->load(w));
    }

    performScreenLayout_();

    for (CircuitWire* cw : m_wires) { cw->updatePath(); }

    return this;
}

void synui::CircuitWidget::reset()
{
    /* Clear the circuit widget of all wires and unit widgets */
    while (!m_wires.empty()) { deleteWireWidget_(m_wires.back()); }
    while (!m_unitWidgets.empty())
    {
        auto it = m_unitWidgets.begin();
        deleteUnitWidget_(it->second);
    }
    m_grid.getBlock({ 0,0 }, m_grid.getShape()).fill(m_grid.getEmptyValue());
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
    m_placingUnitState.widget = createUnitWidget_(a_classId, a_unitId);
    m_placingUnitState.widget->setVisible(false);
    m_placingUnitState.widget->setEnabled(false);
}

synui::UnitWidget* synui::CircuitWidget::createUnitWidget_(unsigned a_classId, int a_unitId)
{
    UnitWidget* widget;
    if (m_registeredUnitWidgets.find(a_classId) == m_registeredUnitWidgets.end())
        widget = new UnitWidget(this, m_vm, a_unitId);
    else
        widget = m_registeredUnitWidgets[a_classId](this, m_vm, a_unitId);

    widget->setEditorCallback([&](unsigned classId, int unitId) { m_unitEditorHost->activateEditor(classId, unitId); });
    // Performing the layout is necesarry so we know what size the widget will be.
    performScreenLayout_();
    m_unitWidgets[a_unitId] = widget;
    return widget;
}

bool synui::CircuitWidget::endPlaceUnit_(const Eigen::Vector2i& a_pos)
{
    if (m_state != State::PlacingUnit)
        return false;
    // Place the unit if the position is valid
    if (checkUnitPos_(m_placingUnitState.widget, a_pos))
    {
        m_state = State::Idle;
        m_placingUnitState.widget->setVisible(true);
        m_placingUnitState.widget->setEnabled(true);
        updateUnitPos_(m_placingUnitState.widget, a_pos);
        return true;
    }
    else
    {
        m_placingUnitState.isValid = false;
        return false;
    }
}

void synui::CircuitWidget::deleteUnit_(int a_unitId)
{
    // Disallow removing the input and output units.
    if (a_unitId == m_vm->getPrototypeCircuit()->getInputUnitId() || a_unitId == m_vm->getPrototypeCircuit()->getOutputUnitId())
        return;

    deleteUnitWidget_(m_unitWidgets[a_unitId]);

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
            deleteWireWidget_(wire);
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
            deleteWireWidget_(wire);
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

void synui::CircuitWidget::updateUnitPos_(UnitWidget* a_unitWidget, const Eigen::Vector2i& a_newPos, bool a_force)
{
    /* Compute the grid cells of the unit's boundaries  */
    Grid2DPoint topLeftCell = m_grid.fromPixel(a_newPos, m_gridSpacing);
    Grid2DPoint bottomRightCell = m_grid.fromPixel(a_newPos + a_unitWidget->size(), m_gridSpacing) + Vector2i::Ones();

    /* Abort if unit widget already exists at the requested position */
    bool positionMatch = (m_grid.toPixel(topLeftCell, m_gridSpacing).array() == a_unitWidget->position().array()).all();
    if (!a_force && positionMatch)
        return;

    auto blk = m_grid.forceGetBlock(topLeftCell, bottomRightCell);
    /* Abort if grid contents already match the specified the unit */
    bool gridMatch = blk.unaryExpr([a_unitWidget](const GridCell& x) { return x.state == GridCell::Unit && x.ptr == a_unitWidget ? x : GridCell{ GridCell::Empty, nullptr }; }).array().all();
    if (!a_force && gridMatch)
        return;

    std::ostringstream os;
    os << "Updating unit position: " << a_unitWidget->getName() << " (" << a_newPos.x() << ", " << a_newPos.y() << ") " << (a_force ? "(forcing)" : "");
    Logger::instance().log("info", os.str());

    /* Erase unit from grid */
    m_grid.replaceValue({ GridCell::Unit, a_unitWidget }, m_grid.getEmptyValue());

    /* Place the unit and reroute the wires */
    std::set<CircuitWire*> wires;
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
    m_grid.forceSetBlock(topLeftCell, bottomRightCell, { GridCell::Unit, a_unitWidget }, false);
    a_unitWidget->setPosition(m_grid.toPixel(topLeftCell, m_gridSpacing));
    // Reroute wires
    for (auto wire : wires)
        wire->updatePath();
    for (auto wire : m_wires)
    {
        if (wires.find(wire) != wires.end())
            continue;
        if (wire->getInputPort().first == a_unitWidget->getUnitId() || wire->getOutputPort().first == a_unitWidget->getUnitId())
            wire->updatePath();
    }
}

bool synui::CircuitWidget::checkUnitPos_(UnitWidget* a_unitWidget, const Eigen::Vector2i& a_newPos)
{
    // Compute the grid cells of the unit's boundaries
    Grid2DPoint topLeftCell = m_grid.fromPixel(a_newPos, m_gridSpacing);
    Vector2i bottomRightPixel = a_newPos + a_unitWidget->size();
    Grid2DPoint bottomRightCell = m_grid.fromPixel(bottomRightPixel, m_gridSpacing);
    bottomRightCell += Vector2i::Ones();

    // Check that the unit would not intersect any other units
    auto blk = m_grid.forceGetBlock(topLeftCell, bottomRightCell);
    auto condition = [a_unitWidget](const GridCell& x) { return x.state != GridCell::Unit || x.ptr == a_unitWidget ? GridCell{ GridCell::Empty, nullptr } : x; };
    return !blk.unaryExpr(condition).array().any();
}

void synui::CircuitWidget::deleteWireWidget_(CircuitWire* wire)
{
    if (!wire)
        return;
    m_wires.erase(std::find(m_wires.begin(), m_wires.end(), wire));
    if (m_highlightedWire == wire)
        m_highlightedWire = nullptr;
    delete wire;
}

void synui::CircuitWidget::deleteUnitWidget_(UnitWidget* widget)
{
    int unitId = widget->getUnitId();

    // Delete the wire widgets connected to this unit
    for (int i = 0; i < m_wires.size(); i++)
    {
        auto wire = m_wires[i];
        if (wire->getInputPort().first == unitId || wire->getOutputPort().first == unitId)
        {
            deleteWireWidget_(wire);
            i--;
        }
    }

    // Delete the unit editor widget
    m_unitEditorHost->removeEditor(unitId);

    // Delete the unit widget
    m_grid.replaceValue({ GridCell::Unit, m_unitWidgets[unitId] }, m_grid.getEmptyValue());
    removeChild(m_unitWidgets[unitId]);
    m_unitWidgets.erase(unitId);
}