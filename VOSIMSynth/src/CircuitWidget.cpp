#include "CircuitWidget.h"
#include "UI.h"
#include "MainWindow.h"
#include "UnitWidget.h"
#include "widgets/DefaultUnitWidget.h"
#include "MainGUI.h"
#include "UnitEditor.h"
#include "ContextMenu.h"
#include "Logger.h"
#include <VoiceManager.h>
#include <units/MathUnits.h>
#include <unordered_set>
#include <set>

namespace synui {
    class CircuitWire {
    public:

        explicit CircuitWire(CircuitWidget* a_parentCircuit)
            :
            highlight(None),
            m_parentCircuit(a_parentCircuit),
            m_inputPort({-1,-1}),
            m_outputPort({-1,-1}),
            m_start({-1,-1}),
            m_end({-1,-1}),
            m_cost(0),
            m_isFinal(false) { }

        ~CircuitWire() { m_parentCircuit->m_grid.replaceValue({CircuitWidget::GridCell::Wire, this}, m_parentCircuit->m_grid.getEmptyValue()); }

        string info() const {
            if (!m_isFinal)
                return "";

            std::ostringstream os;
            /// @DEBUG Show wire cost
            os << "Cost: " << m_cost << std::endl;

            // List current output value for this wire held by each active voice
            vector<int> activeVoiceIndices = m_parentCircuit->m_vm->getActiveVoiceIndices();
            for (int vind : activeVoiceIndices) {
                const syn::Unit& unit = m_parentCircuit->m_vm->getUnit(m_outputPort.first, vind);
                double value = unit.readOutput(m_outputPort.second, 0);
                os << "Voice " << vind << ": " << std::setprecision(4) << value << std::endl;
            }
            return os.str();
        }

        void draw(NVGcontext* ctx) {
            nvgSave(ctx);
            nvgBeginPath(ctx);
            nanogui::Color wireColor;
            // If wire is incomplete (still being drawn), draw a "ghost"
            if (m_inputPort.first < 0 || m_outputPort.first < 0) {
                nvgStrokeWidth(ctx, 2.0f);
                wireColor = nanogui::Color(1.0f, 0.0f, 0.0f, 1.0f);
            } else {
                nvgStrokeWidth(ctx, 1.5f);
                switch (highlight) {
                case Incoming:
                    wireColor = nanogui::Color(0.45f, 0.45f, 1.00f, 0.55f);
                    break;
                case Outgoing:
                    wireColor = nanogui::Color(0.25f, 1.0f, 0.25f, 0.55f);
                    break;
                case Selected:
                    nvgStrokeWidth(ctx, 2.0f);
                    wireColor = nanogui::Color(0.75f, 0.75f, 0.75f, 0.75f);
                    break;
                case None:
                default:
                    wireColor = nanogui::Color(0.85f, 0.20f, 0.10f, 0.65f);
                }
            }
            nvgStrokeColor(ctx, wireColor);
            nvgLineJoin(ctx, NVG_ROUND);

            nvgMoveTo(ctx, m_start.x(), m_start.y());
            if (m_path.empty())
                nvgLineTo(ctx, m_end.x(), m_end.y());

            int gs = m_parentCircuit->gridSpacing();
            for (int i = 0; i < m_path.size(); i += 1) {
                Grid2DPoint& currGridPt = m_path[i];
                Grid2DPoint& nextGridPt = i + 1 < m_path.size() ? m_path[i + 1] : m_path.back();
                Grid2DPoint& nextGridPt2 = i + 2 < m_path.size() ? m_path[i + 2] : m_path.back();
                Grid2DPoint& nextGridPt3 = i + 3 < m_path.size() ? m_path[i + 3] : m_path.back();

                Vector2i currPixelPt = i >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(currGridPt, gs);
                nvgLineTo(ctx, currPixelPt.x(), currPixelPt.y());

                if (m_parentCircuit->wireDrawStyle() == CircuitWidget::Curved) {
                    Vector2i dCurrGridPt = nextGridPt - currGridPt;
                    Vector2i dNextGridPt = nextGridPt2 - nextGridPt;
                    Vector2i dNextGridPt2 = nextGridPt3 - nextGridPt2;
                    // Draw a curve when the path changes direction.
                    if (dNextGridPt.dot(dCurrGridPt) == 0) {
                        Vector2i nextPixelPt = i + 1 >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(nextGridPt, gs);
                        Vector2i nextPixelPt2 = i + 2 >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(nextGridPt2, gs);
                        // Handle "S-curves" and corner turns.
                        if (dNextGridPt2.dot(dNextGridPt) == 0) {
                            Vector2i nextPixelPt3 = i + 3 >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(nextGridPt3, gs);
                            nvgBezierTo(ctx, nextPixelPt.x(), nextPixelPt.y(), nextPixelPt2.x(), nextPixelPt2.y(), nextPixelPt3.x(), nextPixelPt3.y());
                            i += 2;
                        } else {
                            nvgBezierTo(ctx, currPixelPt.x(), currPixelPt.y(), nextPixelPt.x(), nextPixelPt.y(), nextPixelPt2.x(), nextPixelPt2.y());
                            i += 1;
                        }
                    }
                }

            }
            nvgStroke(ctx);


            //            nvgBeginPath(ctx);
            //            nvgFillColor(ctx, nanogui::Color(0.0f,0.0f,0.0f,0.5f));
            //            for (auto gridPt : m_crossings)
            //            {                
            //                Vector2i pixelPt = m_parentCircuit->m_grid.toPixel({gridPt.first, gridPt.second}, m_parentCircuit->getGridSpacing());      
            //                nvgCircle(ctx, pixelPt.x(), pixelPt.y(), 1.0f);
            //            }
            //            nvgFill(ctx);

            /* Draw arrow */
            if (m_path.size() > 1) {
                float noseSize = m_parentCircuit->gridSpacing() * 0.33;
                float noseAngle = 45.0;

                Vector2i pt = m_end;
                Vector2i prevPt = m_parentCircuit->m_grid.toPixel(m_path[m_path.size() - 2], m_parentCircuit->gridSpacing());

                Vector2f dir = (pt - prevPt).cast<float>();
                dir.normalize();
                float dangle = asin(dir[1]);

                Vector2f headOffset = Vector2f{0, 1} * noseSize;
                Vector2f noseOffset = Vector2f{noseSize / sin(2 * PI / 180.0 * noseAngle), 0};

                nvgBeginPath(ctx);
                nvgFillColor(ctx, wireColor);
                nvgTranslate(ctx, pt.x(), pt.y());
                nvgRotate(ctx, dangle);
                nvgTranslate(ctx, -noseSize, 0);
                nvgMoveTo(ctx, 0, 0);
                nvgLineTo(ctx, headOffset.x(), headOffset.y());
                nvgLineTo(ctx, noseOffset.x(), noseOffset.y());
                nvgLineTo(ctx, headOffset.x(), -headOffset.y());
                nvgClosePath(ctx);
                nvgFill(ctx);
            }
            nvgRestore(ctx);
        }

        /**
         * \brief Determine and update the start and end points of the wire.
         * If both the input and output port have been set, then the start and end points are simply the
         * location of those ports. If only one of them has been set, the location of that port is the start
         * point, and the `a_endPt` argument is used to set the end point.
         * \param a_endPt End point to use in the case that one of the ports has not been set.
         */
        void updateStartAndEndPositions(const Vector2i& a_endPt = {-1,-1}) {
            // Determine start/end points
            Vector2i startPos;
            Vector2i endPos = m_end;
            if (m_inputPort.first >= 0 && m_outputPort.first >= 0) {
                m_isFinal = true;
                auto inputWidget = m_parentCircuit->m_unitWidgets[m_inputPort.first];
                auto outputWidget = m_parentCircuit->m_unitWidgets[m_outputPort.first];
                Vector2i inputPos = inputWidget->getInputPortAbsPosition(m_inputPort.second) - m_parentCircuit->absolutePosition();
                Vector2i outputPos = outputWidget->getOutputPortAbsPosition(m_outputPort.second) - m_parentCircuit->absolutePosition();
                startPos = outputPos;
                endPos = inputPos;
            } else if (m_outputPort.first >= 0) {
                auto outputWidget = m_parentCircuit->m_unitWidgets[m_outputPort.first];
                startPos = outputWidget->getOutputPortAbsPosition(m_outputPort.second) - m_parentCircuit->absolutePosition();
                endPos = a_endPt;
            } else if (m_inputPort.first >= 0) {
                auto inputWidget = m_parentCircuit->m_unitWidgets[m_inputPort.first];
                startPos = inputWidget->getInputPortAbsPosition(m_inputPort.second) - m_parentCircuit->absolutePosition();
                endPos = a_endPt;
            }

            m_start = startPos;
            m_end = endPos;
        }

        /**
         * \brief Weighting function used for pathfinding.
         *
         * Weights straight paths higher than jagged ones, and prefers crossing wires instead over going through units.
         */
        template <typename CellType>
        struct weight_func {
            int operator()(const Grid2D<CellType>& grid, const Grid2DPoint& prev, const Grid2DPoint& curr, const Grid2DPoint& next) const {
                int score = 0;
                // Weight jagged paths more than straight ones
                if ((prev.array() > -1).all()) {
                    if (((curr - prev).array() != (next - curr).array()).any())
                        score += 5;
                }

                // Prefer empty over a wire, and prefer a wire over a unit.
                switch (grid.get(next).state) {
                case CircuitWidget::GridCell::Empty:
                    score += 1;
                    break;
                case CircuitWidget::GridCell::Wire:
                    score += 10;
                    break;
                case CircuitWidget::GridCell::Unit:
                    score += 100;
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
        void updatePath() {
            // Update start and end positions
            updateStartAndEndPositions(m_end);
            auto startCell = m_parentCircuit->m_grid.fromPixel(m_start, m_parentCircuit->gridSpacing());
            auto endCell = m_parentCircuit->m_grid.fromPixel(m_end, m_parentCircuit->gridSpacing());

            // Remove from grid            
            m_parentCircuit->m_grid.replaceValue({CircuitWidget::GridCell::Wire, this}, m_parentCircuit->m_grid.getEmptyValue());
            m_crossings.clear();

            // Find new shortest path between start and end cells
            auto path = m_parentCircuit->m_grid.findPath<manhattan_distance, weight_func>(startCell, endCell, &m_cost);
            m_path.resize(path.size());
            copy(path.begin(), path.end(), m_path.begin());

            // Update grid to reflect new location
            for (auto cell : m_path) {
                if (m_parentCircuit->m_grid.get(cell).state == CircuitWidget::GridCell::Wire)
                    m_crossings.insert({cell.x(), cell.y()});
                if (!m_parentCircuit->m_grid.isOccupied(cell))
                    m_parentCircuit->m_grid.get(cell) = {CircuitWidget::GridCell::Wire, this};
            }
        }

        float pathCost() const { return m_cost; }

        void setInputPort(const CircuitWidget::Port& a_port) {
            m_inputPort = a_port;
            updateStartAndEndPositions();
        }

        void setOutputPort(const CircuitWidget::Port& a_port) {
            m_outputPort = a_port;
            updateStartAndEndPositions();
        }

        CircuitWidget::Port getInputPort() const { return m_inputPort; }
        CircuitWidget::Port getOutputPort() const { return m_outputPort; }

        operator json() const {
            json j;
            j["input"] = {m_inputPort.first, m_inputPort.second};
            j["output"] = {m_outputPort.first, m_outputPort.second};
            return j;
        }

        CircuitWire* load(const json& j) {
            setInputPort({j["input"][0], j["input"][1]});
            setOutputPort({j["output"][0], j["output"][1]});
            updatePath();
            return this;
        }

        enum HighlightStyle {
            None = 0,
            Incoming,
            Outgoing,
            Selected
        } highlight;

    private:
        CircuitWidget* m_parentCircuit;
        CircuitWidget::Port m_inputPort;
        CircuitWidget::Port m_outputPort;
        Vector2i m_start;
        Vector2i m_end;
        vector<Grid2DPoint> m_path;
        std::set<std::pair<int, int>> m_crossings;
        float m_cost;
        bool m_isFinal;
    };
}

synui::CircuitWidget::CircuitWidget(Widget* a_parent, MainWindow* a_mainWindow, UnitEditorHost* a_unitEditorHost, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf)
    : Widget(a_parent),
      m_window(a_mainWindow),
      m_unitEditorHost(a_unitEditorHost),
      m_uf(a_uf),
      m_vm(a_vm),
      m_grid{{0,0}, GridCell{GridCell::Empty, nullptr}},
      m_gridSpacing(18),
      m_wireDrawStyle(Curved),
      m_state(new cwstate::IdleState()),
      m_uninitialized(true) {
    registerUnitWidget<syn::InputUnit>([](CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new InputUnitWidget(parent, a_vm, unitId); });
    registerUnitWidget<syn::OutputUnit>([](CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new OutputUnitWidget(parent, a_vm, unitId); });
}

synui::CircuitWidget::~CircuitWidget() {
    for (auto m_wire : m_wires)
        delete m_wire;
}

bool synui::CircuitWidget::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {
    return m_state->mouseButtonEvent(*this, p, button, down, modifiers);
}

bool synui::CircuitWidget::mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
    return m_state->mouseMotionEvent(*this, p, rel, button, modifiers);
}

void synui::CircuitWidget::draw(NVGcontext* ctx) {
    /* Perform first draw init */
    if (m_uninitialized) {
        resizeGrid(m_gridSpacing);
        createInputOutputUnits_();
        m_uninitialized = false;
    }

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());

    /* Draw background */
    nvgBeginPath(ctx);
    nanogui::Color backgroundColor(30, 255);
    nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
    nvgFillColor(ctx, backgroundColor);
    nvgFill(ctx);

    /* Draw grid */
    for (int i = 0; i < m_grid.getSize(); i++) {
        float ptSize = m_gridSpacing * 0.5f;
        const float strokeWidth = 1.0f;

        auto pt = m_grid.unravel_index(i);
        auto pixel = m_grid.toPixel(pt, m_gridSpacing);
        if (m_grid.get(pt).state == GridCell::Empty) {
            nvgStrokeColor(ctx, nanogui::Color{70,255});
        } else if (m_grid.get(pt).state == GridCell::Unit) {
            nvgStrokeColor(ctx, nanogui::Color{10, 255});
        } else if (m_grid.get(pt).state == GridCell::Wire) {
            nvgStrokeColor(ctx, nanogui::Color{25,25,75,255});
        }

        nvgBeginPath(ctx);
        nvgMoveTo(ctx, pixel.x() - ptSize * 0.5f, pixel.y());
        nvgLineTo(ctx, pixel.x() + ptSize * 0.5f, pixel.y());
        nvgMoveTo(ctx, pixel.x(), pixel.y() - ptSize * 0.5f);
        nvgLineTo(ctx, pixel.x(), pixel.y() + ptSize * 0.5f);
        nvgStrokeWidth(ctx, strokeWidth);
        nvgStroke(ctx);
    }

    /* Highlight wires connected to a unit in the current selection */
    for (auto wire : m_wires) {
        auto outUnit = m_unitWidgets[wire->getOutputPort().first];
        auto inUnit = m_unitWidgets[wire->getInputPort().first];
        wire->draw(ctx);
        if (outUnit->getUnitId() == m_unitEditorHost->getActiveUnitId()) {
            auto oldHighlight = wire->highlight;
            wire->highlight = CircuitWire::Outgoing;
            wire->draw(ctx);
            wire->highlight = oldHighlight;
        } else if (inUnit->getUnitId() == m_unitEditorHost->getActiveUnitId()) {
            auto oldHighlight = wire->highlight;
            wire->highlight = CircuitWire::Incoming;
            wire->draw(ctx);
            wire->highlight = oldHighlight;
        }
    }
    nvgRestore(ctx);

    Widget::draw(ctx);

    /* Handle mouse hover drawing that should be displayed in front of the unit widgets */
    nvgSave(ctx);
    // Highlight unit widgets in the current selection
    if (m_unitSelection.size() > 1) {
        nanogui::Color selectedWidgetColor{1.0f,1.0f,0.0f,0.9f};
        for (auto w : m_unitSelection) {
            const auto& p = w->position();
            const auto& s = w->size();
            nvgBeginPath(ctx);
            nvgRect(ctx, p.x() - 0.5f, p.y() - 0.5f, s.x() + 1.0f, s.y() + 1.0f);
            nvgStrokeColor(ctx, selectedWidgetColor);
            nvgStrokeWidth(ctx, 1.0f);
            nvgStroke(ctx);
        }
    }
    nvgRestore(ctx);

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());
    m_state->draw(*this, ctx);
    nvgRestore(ctx);
}

void synui::CircuitWidget::loadPrototype(syn::UnitTypeId a_classId) { createUnit_(a_classId); }

Eigen::Vector2i synui::CircuitWidget::fixToGrid(const Vector2i& a_pixelLocation) const { return m_grid.toPixel(m_grid.fromPixel(a_pixelLocation, m_gridSpacing), m_gridSpacing); }

void synui::CircuitWidget::performLayout(NVGcontext* ctx) {
    std::unordered_map<int, Vector2i> oldSizes;
    for (auto w : m_unitWidgets) {
        oldSizes[w.first] = w.second->size();
        w.second->onGridChange_();
        // reset size so it doesn't affect next layout computation
        w.second->setSize({0,0});
    }
    Widget::performLayout(ctx);
    for (auto w : m_unitWidgets) {
        if ((oldSizes[w.first].array() != w.second->size().array()).any())
            updateUnitPos(w.second, w.second->position(), true);
    }
}

void synui::CircuitWidget::resizeGrid(int a_newGridSpacing) {
    // Require at least 6 pixels per grid point.
    const int minSpacing = 6;
    // Require at least 20 rows and 20 columns (enforced only after first draw)
    const int maxSpacing = m_uninitialized ? a_newGridSpacing : syn::MIN(size().x(), size().y()) / 20;
    a_newGridSpacing = syn::MAX(a_newGridSpacing, minSpacing);
    a_newGridSpacing = syn::MIN(a_newGridSpacing, maxSpacing);
    Vector2i newGridShape = m_grid.fromPixel(size(), a_newGridSpacing);

    if (m_gridSpacing == a_newGridSpacing && (newGridShape.array() == m_grid.getShape().array()).all())
        return;
    m_gridSpacing = a_newGridSpacing;
    m_grid.resize(newGridShape);

    screen()->performLayout();

    for (auto unit : m_unitWidgets)
        updateUnitPos(unit.second, unit.second->position(), true);
}

synui::CircuitWidget::operator json() const {
    json j;
    j["grid_spacing"] = m_gridSpacing;
    json& wires = j["wires"] = json();
    for (CircuitWire* w : m_wires) { wires.push_back(w->operator json()); }
    json& units = j["units"] = json();
    for (auto& u : m_unitWidgets) { units[std::to_string(u.first)] = u.second->operator json(); }
    return j;
}

synui::CircuitWidget* synui::CircuitWidget::load(const json& j) {
    /* Reset to clean state */
    reset();

    /* Load grid spacing */
    resizeGrid(j["grid_spacing"].get<int>());

    /* Load new unit widgets */
    const json& units = j["units"];
    const syn::Circuit* circ = m_vm->getPrototypeCircuit();
    for (json::const_iterator it = units.cbegin(); it != units.cend(); ++it) {
        int unitId = stoi(it.key());
        syn::UnitTypeId classId = circ->getUnit(unitId).getClassIdentifier();
        const json& unit = it.value();
        Vector2i pos{unit["x"].get<int>(), unit["y"].get<int>()};
        UnitWidget* widget = createUnitWidget(classId, unitId)->load(unit);
        updateUnitPos(widget, pos);
        m_unitWidgets[unitId] = widget;
    }

    /* Load new wires */
    const json& wires = j["wires"];
    for (const json& w : wires) {
        CircuitWire* newWire = new CircuitWire(this);
        m_wires.push_back(newWire->load(w));
    }

    _changeState(new cwstate::IdleState());
    return this;
}

void synui::CircuitWidget::reset() {
    /* Clear the circuit widget of all wires and unit widgets */
    while (!m_wires.empty()) { deleteWireWidget(m_wires.back()); }
    while (!m_unitWidgets.empty()) {
        auto it = m_unitWidgets.begin();
        deleteUnitWidget(it->second);
    }
    m_grid.getBlock({0,0}, m_grid.getShape()).fill(m_grid.getEmptyValue());
}

void synui::CircuitWidget::createInputOutputUnits_() {
    const syn::Circuit* circ = m_vm->getPrototypeCircuit();
    int inputUnitId = circ->getInputUnitId();
    syn::UnitTypeId inputClassId = circ->getUnit(inputUnitId).getClassIdentifier();
    int outputUnitId = circ->getOutputUnitId();
    syn::UnitTypeId outputClassId = circ->getUnit(outputUnitId).getClassIdentifier();

    UnitWidget* inWidget = createUnitWidget(inputClassId, inputUnitId);
    m_unitWidgets[inputUnitId] = inWidget;
    UnitWidget* outWidget = createUnitWidget(outputClassId, outputUnitId);
    m_unitWidgets[outputUnitId] = outWidget;
    screen()->performLayout();
    updateUnitPos(inWidget, {0.5 * inWidget->width(), height() * 0.5 - inWidget->height() * 0.5});
    updateUnitPos(outWidget, {width() - 1.5 * outWidget->width(), height() * 0.5 - outWidget->height() * 0.5});
}

void synui::CircuitWidget::createUnit_(syn::UnitTypeId a_classId) {
    syn::RTMessage* msg = new syn::RTMessage();
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
                CircuitWidget* self;
                syn::UnitFactory* unitFactory;
                syn::Unit* unit;
                GetArgs(a_data, 0, self, unitFactory, unit);
                int unitId = a_circuit->addUnit(unit->clone());
                // Queue return message
                if (a_isLast) {
                    GUIMessage* msg = new GUIMessage;
                    msg->action = [](MainWindow* a_win, ByteChunk* a_data) {
                                CircuitWidget* self;
                                int unitId;
                                GetArgs(a_data, 0, self, unitId);
                                self->_changeState(new cwstate::CreatingUnitState(unitId));
                            };
                    PutArgs(&msg->data, self, unitId);
                    self->m_window->queueExternalMessage(msg);
                }
            };

    auto self = this;
    auto unit = m_uf->getFactoryPrototype(a_classId)->prototype;
    PutArgs(&msg->data, self, m_uf, unit);
    m_vm->queueAction(msg);
}

synui::UnitWidget* synui::CircuitWidget::createUnitWidget(syn::UnitTypeId a_classId, int a_unitId) {
    UnitWidget* widget;
    if (m_registeredUnitWidgets.find(a_classId) == m_registeredUnitWidgets.end())
        widget = new DefaultUnitWidget(this, m_vm, a_unitId);
    else
        widget = m_registeredUnitWidgets[a_classId](this, m_vm, a_unitId);
    widget->setName(m_uf->generateUnitName(a_classId));
    widget->setEditorCallback([&](syn::UnitTypeId classId, int unitId) {
            if (m_unitEditorHost->selectedIndex() >= 0)
                m_unitWidgets[m_unitEditorHost->getActiveUnitId()]->setHighlighted(false);
            m_unitEditorHost->activateEditor(classId, unitId);
            m_unitWidgets[unitId]->setHighlighted(true);
        });
    // Performing the layout is necesarry so we know what size the widget will be.
    screen()->performLayout();
    return widget;
}

void synui::CircuitWidget::deleteUnit_(int a_unitId) {
    // Disallow removing the input and output units.
    if (a_unitId == m_vm->getPrototypeCircuit()->getInputUnitId() || a_unitId == m_vm->getPrototypeCircuit()->getOutputUnitId())
        return;

    deleteUnitWidget(m_unitWidgets[a_unitId]);

    // Delete the unit from the circuit
    syn::RTMessage* msg = new syn::RTMessage();
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
                int unitId;
                GetArgs(a_data, 0, unitId);
                a_circuit->removeUnit(unitId);
            };

    PutArgs(&msg->data, a_unitId);
    m_vm->queueAction(msg);
}

void synui::CircuitWidget::deleteConnection(const Port& a_inputPort, const Port& a_outputPort) {
    // Delete the wire widget
    bool foundWire = false;
    for (int i = 0; i < m_wires.size(); i++) {
        auto wire = m_wires[i];
        if (wire->getInputPort() == a_inputPort && wire->getOutputPort() == a_outputPort) {
            deleteWireWidget(wire);
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

    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
                CircuitWidget* self;
                int fromUnit, fromPort, toUnit, toPort;
                GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
                a_circuit->disconnectInternal(fromUnit, fromPort, toUnit, toPort);

                // Queue return message
                if (a_isLast) {
                    GUIMessage* msg = new GUIMessage;
                    msg->action = [](MainWindow* a_win, ByteChunk* a_data) {
                                CircuitWidget* self;
                                int fromUnit, fromPort, toUnit, toPort;
                                GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
                                UnitWidget* fromWidget = self->m_unitWidgets[fromUnit];
                                UnitWidget* toWidget = self->m_unitWidgets[toUnit];
                                self->updateUnitPos(fromWidget, fromWidget->position(), true);
                                self->updateUnitPos(toWidget, toWidget->position(), true);
                            };
                    PutArgs(&msg->data, self, fromUnit, fromPort, toUnit, toPort);
                    self->m_window->queueExternalMessage(msg);
                }
            };

    m_vm->queueAction(msg);
}

void synui::CircuitWidget::createConnection(const Port& a_inputPort, const Port& a_outputPort) {
    // send the new connection request to the real-time thread
    syn::RTMessage* msg = new syn::RTMessage();
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
                CircuitWidget* self;
                int fromUnit, fromPort, toUnit, toPort;
                GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
                a_circuit->connectInternal(fromUnit, fromPort, toUnit, toPort);

                // Queue return message
                if (a_isLast) {
                    GUIMessage* msg = new GUIMessage;
                    msg->action = [](MainWindow* a_win, ByteChunk* a_data) {
                                CircuitWidget* self;
                                int fromUnit, fromPort, toUnit, toPort;
                                GetArgs(a_data, 0, self, fromUnit, fromPort, toUnit, toPort);
                                self->onConnectionCreated_({toUnit, toPort}, {fromUnit, fromPort});
                            };
                    PutArgs(&msg->data, self, fromUnit, fromPort, toUnit, toPort);
                    self->m_window->queueExternalMessage(msg);
                }
            };

    CircuitWidget* self = this;

    PutArgs(&msg->data, self, a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);
    m_vm->queueAction(msg);
}

void synui::CircuitWidget::onConnectionCreated_(const Port& a_inputPort, const Port& a_outputPort) {
    // Remove any wire widgets that already point to that unit input
    for (int i = 0; i < m_wires.size(); i++) {
        auto wire = m_wires[i];
        if (wire->getInputPort() == a_inputPort) {
            deleteWireWidget(wire);
            i--;
        }
    }

    CircuitWire* wire = new CircuitWire(this);
    wire->setInputPort(a_inputPort);
    wire->setOutputPort(a_outputPort);
    wire->updatePath();
    m_wires.push_back(wire);

    // Re-set the widget positions on the grid
    updateUnitPos(m_unitWidgets[a_inputPort.first], m_unitWidgets[a_inputPort.first]->position(), true);
    updateUnitPos(m_unitWidgets[a_outputPort.first], m_unitWidgets[a_outputPort.first]->position(), true);
}

void synui::CircuitWidget::updateUnitPos(UnitWidget* a_unitWidget, const Vector2i& a_newPos, bool a_force) {
    /* Compute the grid cells of the unit's boundaries  */
    Grid2DPoint topLeftCell = m_grid.fromPixel(a_newPos, m_gridSpacing);
    Grid2DPoint bottomRightCell = m_grid.fromPixel(a_newPos + a_unitWidget->size(), m_gridSpacing) + Vector2i::Ones();

    /* Abort if unit widget already exists at the requested position */
    bool positionMatch = (m_grid.toPixel(topLeftCell, m_gridSpacing).array() == a_unitWidget->position().array()).all();
    if (!a_force && positionMatch)
        return;

    auto blk = m_grid.forceGetBlock(topLeftCell, bottomRightCell);;

    /* Erase unit from grid */
    m_grid.replaceValue({GridCell::Unit, a_unitWidget}, m_grid.getEmptyValue());

    /* Place the unit and reroute the wires */
    std::unordered_set<CircuitWire*> wires;
    // Record any wires we may be overwriting
    for (int r = 0; r < blk.rows(); r++) {
        for (int c = 0; c < blk.cols(); c++) {
            const GridCell& cellContents = blk(r, c);
            if (cellContents.state == GridCell::Wire)
                wires.insert(reinterpret_cast<CircuitWire*>(cellContents.ptr));
        }
    }
    // Place the unit
    m_grid.forceSetBlock(topLeftCell, bottomRightCell, {GridCell::Unit, a_unitWidget}, false);
    a_unitWidget->setPosition(m_grid.toPixel(topLeftCell, m_gridSpacing));
    // Reroute wires
    for (auto wire : wires)
        wire->updatePath();
    for (auto wire : m_wires) {
        if (wires.find(wire) != wires.end())
            continue;
        if (wire->getInputPort().first == a_unitWidget->getUnitId() || wire->getOutputPort().first == a_unitWidget->getUnitId())
            wire->updatePath();
    }
}

bool synui::CircuitWidget::checkUnitPos(UnitWidget* a_unitWidget, const Vector2i& a_newPos) {
    // Compute the grid cells of the unit's boundaries
    Grid2DPoint topLeftCell = m_grid.fromPixel(a_newPos, m_gridSpacing);
    Vector2i bottomRightPixel = a_newPos + a_unitWidget->size();
    Grid2DPoint bottomRightCell = m_grid.fromPixel(bottomRightPixel, m_gridSpacing);
    bottomRightCell += Vector2i::Ones();

    // Check that the unit would not intersect any other units
    auto blk = m_grid.forceGetBlock(topLeftCell, bottomRightCell);
    auto condition = [a_unitWidget](const GridCell& x) { return x.state != GridCell::Unit || x.ptr == a_unitWidget ? GridCell{GridCell::Empty, nullptr} : x; };
    return !blk.unaryExpr(condition).array().any();
}

void synui::CircuitWidget::deleteWireWidget(CircuitWire* wire) {
    if (!wire)
        return;
    m_wires.erase(find(m_wires.begin(), m_wires.end(), wire));
    delete wire;
}

void synui::CircuitWidget::deleteUnitWidget(UnitWidget* widget) {
    int unitId = widget->getUnitId();

    // Delete the wire widgets connected to this unit
    for (int i = 0; i < m_wires.size(); i++) {
        auto wire = m_wires[i];
        if (wire->getInputPort().first == unitId || wire->getOutputPort().first == unitId) {
            deleteWireWidget(wire);
            i--;
        }
    }

    // Delete the unit editor widget
    m_unitEditorHost->removeEditor(unitId);

    // Remove the widget from the current selection if necessary
    if (m_unitSelection.find(m_unitWidgets[unitId]) != m_unitSelection.end())
        m_unitSelection.erase(m_unitWidgets[unitId]);

    // Delete the unit widget
    m_grid.replaceValue({GridCell::Unit, m_unitWidgets[unitId]}, m_grid.getEmptyValue());
    removeChild(m_unitWidgets[unitId]);
    m_unitWidgets.erase(unitId);
}

void synui::CircuitWidget::createJunction(CircuitWire* toWire, CircuitWire* fromWire, const Vector2i& pos, syn::Unit* a_unit) {
    syn::RTMessage* msg = new syn::RTMessage();
    msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
                CircuitWidget* self;
                syn::UnitFactory* unitFactory;
                syn::Unit* unit;
                CircuitWire* toWire;
                CircuitWire* fromWire;
                int px, py;
                GetArgs(a_data, 0, self, unitFactory, unit, toWire, fromWire, px, py);
                int unitId = a_circuit->addUnit(unit->clone());
                syn::UnitTypeId classId = unit->getClassIdentifier();
                // Connect both wires to the sum unit
                a_circuit->connectInternal(fromWire->getOutputPort().first, fromWire->getOutputPort().second, unitId, 0);
                a_circuit->connectInternal(toWire->getOutputPort().first, toWire->getOutputPort().second, unitId, 1);
                // Replace toWire's connection with one from sum unit
                a_circuit->connectInternal(unitId, 0, toWire->getInputPort().first, toWire->getInputPort().second);

                // Queue return message
                if (a_isLast) {
                    GUIMessage* msg = new GUIMessage;
                    msg->action = [](MainWindow* a_win, ByteChunk* a_data) {
                                CircuitWidget* self;
                                syn::UnitTypeId classId;
                                int unitId;
                                CircuitWire* toWire;
                                CircuitWire* fromWire;
                                int px, py;
                                GetArgs(a_data, 0, self, classId, unitId, toWire, fromWire, px, py);

                                // Create + place unit widget
                                UnitWidget* uw = self->createUnitWidget(classId, unitId);
                                Vector2i pos = Vector2i{px, py} - Vector2i::Ones() * self->gridSpacing();
                                if (self->checkUnitPos(uw, pos)) {
                                    self->m_unitWidgets[unitId] = uw;
                                    self->updateUnitPos(uw, pos);
                                    uw->triggerEditorCallback();
                                } else {
                                    self->removeChild(uw);
                                    self->_changeState(new cwstate::CreatingUnitState(unitId));
                                }
                                // Finish drawing wire
                                self->onConnectionCreated_({unitId, 0}, fromWire->getOutputPort());
                                // Add the wire going into the sum unit...
                                self->onConnectionCreated_({unitId, 1}, toWire->getOutputPort());
                                // and a wire going from the sum unit to the original input port
                                self->onConnectionCreated_(toWire->getInputPort(), {unitId,0});

                                delete fromWire;
                            };
                    PutArgs(&msg->data, self, classId, unitId, toWire, fromWire, px, py);
                    self->m_window->queueExternalMessage(msg);
                    delete unit;
                }
            };

    auto self = this;
    PutArgs(&msg->data, self, m_uf, a_unit, toWire, fromWire, pos.x(), pos.y());
    m_vm->queueAction(msg);
}

void synui::CircuitWidget::_changeState(cwstate::State* a_state) {
    if (m_state != nullptr) {
        m_state->exit(*this, *a_state);
    }
    a_state->enter(*this, *m_state);
    m_state.reset(a_state);
}

bool synui::cwstate::IdleState::mouseButtonEvent(CircuitWidget& cw, const Vector2i& p, int button, bool down, int modifiers) {
    bool captured = cw.Widget::mouseButtonEvent(p, button, down, modifiers);

    Vector2i mousePos = p - cw.position();
    Grid2DPoint pt = cw.grid().fromPixel(mousePos, cw.gridSpacing());
    if (!cw.grid().contains(pt)) {
        pt[0] = syn::CLAMP<int>(pt[0], 0, cw.grid().getShape()[0]);
        pt[1] = syn::CLAMP<int>(pt[1], 0, cw.grid().getShape()[1]);
    }

    const auto& cell = cw.grid().get(pt);
    // Reset the current selection if it does not contain the clicked unit widget.
    if (cell.state == CircuitWidget::GridCell::Unit) {
        UnitWidget* w = reinterpret_cast<UnitWidget*>(cell.ptr);
        if (cw.unitSelection().find(w) == cw.unitSelection().end()) {
            cw.unitSelection().clear();
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && down) {
        if (cw.grid().get(pt).state == CircuitWidget::GridCell::Unit) {
            UnitWidget* uw = reinterpret_cast<UnitWidget*>(cw.grid().get(pt).ptr);
            int unitId = uw->getUnitId();
            bool isOutput = true;
            int portId = uw->getOutputPort(mousePos - uw->position());
            if (portId < 0) {
                isOutput = false;
                portId = uw->getInputPort(mousePos - uw->position());
            }
            if(portId>=0) {
                changeState(cw, *new DrawingWireState({unitId,portId}, isOutput));
                return true;
            }
            changeState(cw, *new MovingUnitState());
            return true;
        }
        if(!captured){
            changeState(cw, *new DrawingSelectionState());
            return true;
        }
    }

    if (m_highlightedWire) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT && !down) {
            cw.deleteConnection(m_highlightedWire->getInputPort(), m_highlightedWire->getOutputPort());
            m_highlightedWire = nullptr;
            return true;
        }
    }

    return true;
}

bool synui::cwstate::IdleState::mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) {
    Grid2DPoint cell = cw.grid().fromPixel(p, cw.gridSpacing());
    if (cw.grid().contains(cell) && cw.grid().get(cell).state == CircuitWidget::GridCell::Wire) {
        if (cw.grid().get(cell).ptr != m_highlightedWire) {
            if (m_highlightedWire) {
                m_highlightedWire->highlight = CircuitWire::None;
            }
            m_wireHighlightTime = glfwGetTime();
            m_highlightedWire = reinterpret_cast<CircuitWire*>(cw.grid().get(cell).ptr);
            m_highlightedWire->highlight = CircuitWire::Selected;
        }
    } else if (m_highlightedWire) {
        m_highlightedWire->highlight = CircuitWire::None;
        m_highlightedWire = nullptr;
    }
    cw.Widget::mouseMotionEvent(p, rel, button, modifiers);
    return true;
}

void synui::cwstate::IdleState::draw(CircuitWidget& cw, NVGcontext* ctx) {
    Vector2i mousePos = cw.screen()->mousePos() - cw.absolutePosition();
    if (cw.contains(mousePos)) {
        double elapsed = glfwGetTime() - m_wireHighlightTime;
        if (m_highlightedWire) {
            if (elapsed > 0.15) {
                nvgSave(ctx);
                drawTooltip(ctx, {mousePos.x(),mousePos.y() + 25}, m_highlightedWire->info(), elapsed);
                nvgRestore(ctx);
            }
        }
    }
}

void synui::cwstate::DrawingWireState::draw(CircuitWidget& cw, NVGcontext* ctx) {
    Vector2i mousePos = cw.screen()->mousePos() - cw.absolutePosition();
    m_wire->updateStartAndEndPositions(mousePos);
    m_wire->draw(ctx);
    // Draw an overlay when dragging one wire onto another
    Grid2DPoint mousePt = cw.grid().fromPixel(mousePos, cw.gridSpacing());
    if (cw.grid().get(mousePt).state == CircuitWidget::GridCell::Wire && m_startedFromOutput) {
        Vector2i fixedMousePos = cw.grid().toPixel(mousePt, cw.gridSpacing());
        nvgBeginPath(ctx);
        nvgStrokeColor(ctx, nanogui::Color(200, 0, 0, 200));
        nvgMoveTo(ctx, fixedMousePos.x() - cw.gridSpacing() * 0.5, fixedMousePos.y() - cw.gridSpacing() * 0.5);
        nvgLineTo(ctx, fixedMousePos.x() + cw.gridSpacing() * 0.5, fixedMousePos.y() + cw.gridSpacing() * 0.5);
        nvgMoveTo(ctx, fixedMousePos.x() + cw.gridSpacing() * 0.5, fixedMousePos.y() - cw.gridSpacing() * 0.5);
        nvgLineTo(ctx, fixedMousePos.x() - cw.gridSpacing() * 0.5, fixedMousePos.y() + cw.gridSpacing() * 0.5);
        nvgStroke(ctx);
    }
}

void synui::cwstate::DrawingWireState::enter(CircuitWidget& cw, State& oldState) {
    Eigen::Vector2i mousePos = cw.screen()->mousePos() - cw.absolutePosition();
    if(m_wire==nullptr)
        m_wire = new CircuitWire(&cw);
    else
        *m_wire = CircuitWire(&cw);
    m_wire->updateStartAndEndPositions(mousePos);
    if (m_startedFromOutput)
        m_wire->setOutputPort(m_startPort);
    else
        m_wire->setInputPort(m_startPort);
}

void synui::cwstate::DrawingWireState::exit(CircuitWidget& cw, State& newState) {
    // If connection is invalid, delete the wire and reset state.
    bool isValid = m_endPort.first >= 0 && m_endPort.second >= 0 && !(m_endedOnOutput == m_startedFromOutput || m_wire->getInputPort().first == m_endPort.first || m_wire->getOutputPort().first == m_endPort.first);
    if (!isValid) {
        // If the port was simply clicked on, activate the unit's editor
        if (m_startPort == m_endPort)
            cw.unitWidgets()[m_endPort.first]->triggerEditorCallback();
    } else {
        if (m_endedOnOutput)
            m_wire->setOutputPort(m_endPort);
        else
            m_wire->setInputPort(m_endPort);
        cw.createConnection(m_wire->getInputPort(), m_wire->getOutputPort());
    }
}

synui::cwstate::DrawingWireState::~DrawingWireState() {
    if (m_wire != nullptr)
        delete m_wire;
}

bool synui::cwstate::DrawingWireState::mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) {
    if (!down) {
        Vector2i mousePos = p - cw.position();
        Grid2DPoint mousePt = cw.grid().fromPixel(mousePos, cw.gridSpacing());
        if (cw.grid().get(mousePt).state == CircuitWidget::GridCell::Unit) {
            UnitWidget* unit = reinterpret_cast<UnitWidget*>(cw.grid().get(mousePt).ptr);
            int unitId = unit->getUnitId();
            bool isOutput = true;
            int portId = unit->getOutputPort(mousePos - unit->position());
            if (portId < 0) {
                isOutput = false;
                portId = unit->getInputPort(mousePos - unit->position());
            }
            m_endPort = {unitId, portId};
            m_endedOnOutput = isOutput;
            changeState(cw, *new IdleState());
            return true;
        } else if (cw.grid().get(mousePt).state == CircuitWidget::GridCell::Wire && m_startedFromOutput) {
            CircuitWire* toWire = static_cast<CircuitWire*>(cw.grid().get(mousePt).ptr);
            CircuitWire* fromWire = new CircuitWire(&cw);
            fromWire->setInputPort(m_wire->getInputPort());
            fromWire->setOutputPort(m_wire->getOutputPort());
            ContextMenu<int>* cm = new ContextMenu<int>(&cw, true, [&cw, fromWire, toWire, mousePos](const string& a_item, const int& a_value) {
                    if (a_value == 0) {
                        syn::Unit* u = new syn::SummerUnit("");
                        /// TODO: Remove unit names on the real-time thread.
                        u->setName(syn::UnitFactory::instance().generateUnitName(u->getClassIdentifier()));
                        cw.createJunction(toWire, fromWire, mousePos, u);
                    } else if (a_value == 1) {
                        syn::Unit* u = new syn::GainUnit("");
                        u->setName(syn::UnitFactory::instance().generateUnitName(u->getClassIdentifier()));
                        cw.createJunction(toWire, fromWire, mousePos, u);
                    }
                });
            cm->addMenuItem("Sum", 0);
            cm->addMenuItem("Mul", 1);
            cm->activate();
            cw.performLayout(cw.screen()->nvgContext());
            cm->setPosition(mousePos - Vector2i{cm->width() * 0.5,0});
        }
        m_endedOnOutput = m_startedFromOutput;
        m_endPort = {-1, -1};
        changeState(cw, *new IdleState());
        return true;
    }
    return true;
}

bool synui::cwstate::CreatingUnitState::mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && down) {
        Vector2i a_pos = p - cw.position();
        // Place the unit if the position is valid
        if (cw.checkUnitPos(m_widget, a_pos)) {
            m_widget->setVisible(true);
            m_widget->setEnabled(true);
            cw.updateUnitPos(m_widget, a_pos);
            m_widget->triggerEditorCallback();
            changeState(cw, *new IdleState());
            return true;
        }
        cw.deleteUnitWidget(m_widget);
        m_widget = nullptr;
        changeState(cw, *new IdleState());
    }else if (button == GLFW_MOUSE_BUTTON_RIGHT && down) {        
        // Delete the wire widgets connected to this unit
        for (int i = 0; i < cw.wires().size(); i++) {
            auto wire = cw.wires()[i];
            if (wire->getInputPort().first == m_unitId || wire->getOutputPort().first == m_unitId) {
                cw.deleteWireWidget(wire);
                i--;
            }
        }
        cw.removeChild(m_widget);
        m_widget = nullptr;
        cw.unitWidgets().erase(m_unitId);
        changeState(cw, *new IdleState());
    }
    return true;
}

void synui::cwstate::CreatingUnitState::draw(CircuitWidget& cw, NVGcontext* ctx) {
    Vector2i mousePos = cw.screen()->mousePos() - cw.absolutePosition();
    UnitWidget* w = m_widget;
    m_isValid = cw.checkUnitPos(m_widget, mousePos);
    nvgBeginPath(ctx);
    if (m_isValid)
        nvgStrokeColor(ctx, nanogui::Color(0.7f, 0.7f, 0.7f, 0.75f));
    else
        nvgStrokeColor(ctx, nanogui::Color(0.7f, 0.0f, 0.0f, 0.75f));
    nvgRect(ctx, mousePos[0], mousePos[1], w->width(), w->height());
    nvgStroke(ctx);
}

void synui::cwstate::CreatingUnitState::enter(CircuitWidget& cw, State& oldState) {
    syn::UnitTypeId classId = cw.vm().getUnit(m_unitId).getClassIdentifier();
    m_widget = cw.createUnitWidget(classId, m_unitId);
    m_widget->setVisible(false);
    m_widget->setEnabled(false);
    cw.unitWidgets()[m_unitId] = m_widget;
}

bool synui::cwstate::MovingUnitState::mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) {
    if (!down) {
        Vector2i mousePos = p - cw.position();
        Vector2i offset = mousePos - m_start;

        // We only perform the move if it will work for every unit, so check if any of the units will fail the
        // move.
        bool canDo = true;
        for (auto& oPos : m_origPositions) {
            UnitWidget* w = cw.unitWidgets()[oPos.first];
            Vector2i newPos = oPos.second + offset;
            canDo &= cw.checkUnitPos(w, newPos);
        }
        // Either reset the units or commit them to their new positions.
        for (auto& oPos : m_origPositions) {
            UnitWidget* w = cw.unitWidgets()[oPos.first];
            Vector2i oldPos = oPos.second;
            Vector2i newPos = oPos.second + offset;
            if (canDo)
                cw.updateUnitPos(w, newPos, true);
            else
                cw.updateUnitPos(w, oldPos, true);
        }
        changeState(cw, *new IdleState());
    }
    return true;
}

bool synui::cwstate::MovingUnitState::mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) {
    Vector2i mousePos = p - cw.position();
    Vector2i offset = mousePos - m_start;

    for (auto& w : cw.unitSelection()) {
        Vector2i newPos = m_origPositions[w->getUnitId()] + offset;
        w->setPosition(newPos);
    }
    return true;
}

void synui::cwstate::MovingUnitState::draw(CircuitWidget& cw, NVGcontext* ctx) { }

void synui::cwstate::MovingUnitState::enter(CircuitWidget& cw, State& oldState) {
    m_start = cw.screen()->mousePos() - cw.absolutePosition();
    auto cell = cw.grid().get(cw.grid().fromPixel(m_start, cw.gridSpacing()));
    // Reset the current selection if it does not contain the target unit widget.
    if (cell.state == CircuitWidget::GridCell::Unit) {
        UnitWidget* w = reinterpret_cast<UnitWidget*>(cell.ptr);
        if (cw.unitSelection().find(w) == cw.unitSelection().end()) {
            cw.unitSelection().clear();
            cw.unitSelection().insert(w);
        }
    }
    for (auto& w : cw.unitSelection()) {
        cw.grid().replaceValue({CircuitWidget::GridCell::Unit, w}, {CircuitWidget::GridCell::Empty, nullptr});
        m_origPositions[w->getUnitId()] = w->position();
    }
}

bool synui::cwstate::DrawingSelectionState::mouseButtonEvent(CircuitWidget& cw, const Vector2i& p, int button, bool down, int modifiers) {
    if (!down) {
        if (cw.unitSelection().size() == 1) {
            (*cw.unitSelection().begin())->triggerEditorCallback();
            cw.unitSelection().clear();
        }
        changeState(cw, *new IdleState());
        return true;
    }
    return false;
}

bool synui::cwstate::DrawingSelectionState::mouseMotionEvent(CircuitWidget& cw, const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
    cw.unitSelection().clear();
    m_endPos = cw.screen()->mousePos() - cw.absolutePosition();
    Grid2DPoint pt0 = cw.grid().fromPixel(m_startPos, cw.gridSpacing());
    Grid2DPoint pt1 = cw.grid().fromPixel(m_endPos, cw.gridSpacing());

    // Extend selection box by 1 grid point on the bottom and the right.
    if (pt1.x() > pt0.x())
        pt1.x() += 1;
    else
        pt0.x() += 1;
    if (pt1.y() > pt0.y())
        pt1.y() += 1;
    else
        pt0.y() += 1;

    auto blk = cw.grid().forceGetBlock(pt0, pt1);
    for (int r = 0; r < blk.rows(); r++) {
        for (int c = 0; c < blk.cols(); c++) {
            if (blk(r, c).state == CircuitWidget::GridCell::Unit)
                cw.unitSelection().insert(reinterpret_cast<UnitWidget*>(blk(r, c).ptr));
        }
    }
    cw.Widget::mouseMotionEvent(p, rel, button, modifiers);
    return true;
}

void synui::cwstate::DrawingSelectionState::draw(CircuitWidget& cw, NVGcontext* ctx) {
    nanogui::Color selectionOutlineColor{0.0,0.0,0.0f,1.0f};
    nanogui::Color selectionFillColor{1.0,1.0,0.0f,0.125f};
    const auto& boxPos = m_startPos;
    Vector2i boxSize = m_endPos - boxPos;
    nvgBeginPath(ctx);
    nvgRect(ctx, boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y());
    nvgFillColor(ctx, selectionFillColor);
    nvgFill(ctx);
    nvgStrokeColor(ctx, selectionOutlineColor);
    nvgStrokeWidth(ctx, 0.75f);
    nvgStroke(ctx);
}

void synui::cwstate::DrawingSelectionState::enter(CircuitWidget& cw, State& oldState) {
    cw.unitSelection().clear();
    // Deselect active unit and clear editor.
    if (cw.unitEditorHost().getActiveUnitId() >= 0) {
        cw.unitWidgets()[cw.unitEditorHost().getActiveUnitId()]->setHighlighted(false);
        cw.unitEditorHost().reset();
    }
    m_endPos = m_startPos = cw.screen()->mousePos() - cw.absolutePosition();
}
