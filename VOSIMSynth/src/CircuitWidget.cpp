#include "CircuitWidget.h"
#include "UI.h"
#include "MainWindow.h"
#include "UnitWidget.h"
#include "widgets/DefaultUnitWidget.h"
#include "MainGUI.h"
#include "UnitEditor.h"
#include "ContextMenu.h"
#include "CircuitWire.h"
#include <VoiceManager.h>
#include <Command.h>
#include <units/MathUnits.h>
#include <unordered_set>
#include <set>

synui::CircuitWidget::CircuitWidget(Widget* a_parent, MainWindow* a_mainWindow, UnitEditorHost* a_unitEditorHost, syn::VoiceManager* a_vm)
    : Widget(a_parent),
      m_window(a_mainWindow),
      m_unitEditorHost(a_unitEditorHost),
      m_vm(a_vm),
      m_grid{{0,0}, GridCell{}},
      m_gridSpacing(15),
      m_wireDrawStyle(Curved),
      m_state(new cwstate::IdleState()),
      m_uninitialized(true) 
{
    registerUnitWidget<syn::InputUnit>([](CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new InputUnitWidget(parent, a_vm, unitId); });
    registerUnitWidget<syn::OutputUnit>([](CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new OutputUnitWidget(parent, a_vm, unitId); });
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
        if (!m_grid.get(pt)) {
            nvgStrokeColor(ctx, nanogui::Color{70,255});
        } else if (m_grid.get(pt).contains(GridCell::State::Unit)) {
            nvgStrokeColor(ctx, nanogui::Color{10, 255});
        } else if (m_grid.get(pt).contains(GridCell::State::Wire)) {
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
    nvgTranslate(ctx, mPos.x(), mPos.y());
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

    m_state->draw(*this, ctx);
    nvgRestore(ctx);
}

void synui::CircuitWidget::loadPrototype(syn::UnitTypeId a_classId) { createUnit(a_classId); }

Eigen::Vector2i synui::CircuitWidget::fixToGrid(const Vector2i& a_pixelLocation) const { return m_grid.toPixel(m_grid.fromPixel(a_pixelLocation, m_gridSpacing), m_gridSpacing); }

void synui::CircuitWidget::performLayout(NVGcontext* ctx) {
    std::unordered_map<int, Vector2i> oldSizes;
    for (auto w : m_unitWidgets) {
        if (!w.second->visible())
            continue;
        oldSizes[w.first] = w.second->size();
        // update layout
        w.second->onGridChange_();
        // reset size so it doesn't affect next layout computation
        w.second->setSize({0,0});
    }
    Widget::performLayout(ctx);
    for (auto w : m_unitWidgets) {
        if (!w.second->visible())
            continue;
        if ((oldSizes[w.first].array() != w.second->size().array()).any())
            updateUnitPos(w.second, w.second->position(), true);
    }
}

void synui::CircuitWidget::resizeGrid(int a_newGridSpacing) {
    // Require at least 10 pixels per grid point.
    const int minSpacing = 10;
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
    for (auto w : m_wires) { wires.push_back(w->operator json()); }
    json& units = j["units"] = json();
    for (auto u : m_unitWidgets) { units[std::to_string(u.first)] = u.second->operator json(); }
    return j;
}

synui::CircuitWidget* synui::CircuitWidget::load(const json& j) {
    /* Reset to clean state */
    reset();

    /* Load grid spacing */
    resizeGrid(j["grid_spacing"].get<int>());

    /* Load new unit widgets */
    const json& units = j["units"];
    const syn::Circuit& circ = m_vm->getPrototypeCircuit();
    for (json::const_iterator it = units.cbegin(); it != units.cend(); ++it) {
        int unitId = stoi(it.key());
        syn::UnitTypeId classId = circ.getUnit(unitId).getClassIdentifier();
        const json& unit = it.value();
        Vector2i pos{unit["x"].get<int>(), unit["y"].get<int>()};
        UnitWidget* widget = createUnitWidget(classId, unitId)->load(unit);
        updateUnitPos(widget, pos);
        m_unitWidgets[unitId] = widget;
    }

    /* Load new wires */
    const json& wires = j["wires"];
    for (const json& w : wires) {
        std::shared_ptr<CircuitWire> newWire = std::make_shared<CircuitWire>(this);
        newWire->load(w);
        m_wires.push_back(newWire);
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
    const syn::Circuit& circ = m_vm->getPrototypeCircuit();
    int inputUnitId = circ.getInputUnitId();
    syn::UnitTypeId inputClassId = circ.getUnit(inputUnitId).getClassIdentifier();
    int outputUnitId = circ.getOutputUnitId();
    syn::UnitTypeId outputClassId = circ.getUnit(outputUnitId).getClassIdentifier();

    UnitWidget* inWidget = createUnitWidget(inputClassId, inputUnitId);
    UnitWidget* outWidget = createUnitWidget(outputClassId, outputUnitId);
    screen()->performLayout();
    if (inWidget != nullptr)
        updateUnitPos(inWidget, {0.5 * inWidget->width(), height() * 0.5 - inWidget->height() * 0.5});
    if (outWidget != nullptr)
        updateUnitPos(outWidget, {width() - 1.5 * outWidget->width(), height() * 0.5 - outWidget->height() * 0.5});
}

void synui::CircuitWidget::createUnit(syn::UnitTypeId a_classId) {
    auto unit = syn::UnitFactory::instance().createUnit(a_classId);
    auto f = [this, unit]() {
                for (int i = 0; i < m_vm->getMaxVoices(); i++) {
                    m_vm->getVoiceCircuit(i).addUnit(unit->clone());
                }
                int unitId = m_vm->getPrototypeCircuit().addUnit(unit);
                // Queue return message
                auto f = [this, unitId]() {
                            _changeState(new cwstate::CreatingUnitState(unitId));
                        };
                m_window->queueExternalMessage(syn::MakeCommand(f));
            };
    m_vm->queueAction(syn::MakeCommand(f));
}

synui::UnitWidget* synui::CircuitWidget::createUnitWidget(syn::UnitTypeId a_classId, int a_unitId) {
    if (m_unitWidgets.find(a_unitId) != m_unitWidgets.end())
        return nullptr;
    UnitWidget* widget;
    if (m_registeredUnitWidgets.find(a_classId) == m_registeredUnitWidgets.end())
        widget = new DefaultUnitWidget(this, m_vm, a_unitId);
    else
        widget = m_registeredUnitWidgets[a_classId](this, m_vm, a_unitId);
    widget->setName(syn::UnitFactory::instance().getFactoryPrototype(a_classId)->name);
    widget->setEditorCallback([&](syn::UnitTypeId classId, int unitId) {
            if (m_unitEditorHost->selectedIndex() >= 0)
                m_unitWidgets[m_unitEditorHost->getActiveUnitId()]->setHighlighted(false);
            m_unitEditorHost->activateEditor(classId, unitId);
            m_unitWidgets[unitId]->setHighlighted(true);
        });
    m_unitWidgets[a_unitId] = widget;
    // Update the widget's size
    Vector2i pref = widget->preferredSize(screen()->nvgContext()), fix = widget->fixedSize();
    widget->setSize(Vector2i(
        fix[0] ? fix[0] : pref[0],
        fix[1] ? fix[1] : pref[1]
    ));
    widget->performLayout(screen()->nvgContext());

    onAddUnit(widget);

    return widget;
}

void synui::CircuitWidget::deleteUnit(int a_unitId) {
    // Disallow removing the input and output units.
    if (a_unitId == m_vm->getPrototypeCircuit().getInputUnitId() || a_unitId == m_vm->getPrototypeCircuit().getOutputUnitId())
        return;
    
    deleteUnitWidget(m_unitWidgets[a_unitId]);

    // Delete the unit from the circuit
    auto f = [this, a_unitId]() {
                for (int i = 0; i < m_vm->getMaxVoices(); i++) {
                    m_vm->getVoiceCircuit(i).removeUnit(a_unitId);
                }
                m_vm->getPrototypeCircuit().removeUnit(a_unitId);          
            };
    m_vm->queueAction(syn::MakeCommand(f));
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
    auto f = [this, a_outputPort, a_inputPort]() {
                for (int i = 0; i < m_vm->getMaxVoices(); i++) {
                    syn::Circuit& circuit = m_vm->getVoiceCircuit(i);
                    circuit.disconnectInternal(a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);
                }
                syn::Circuit& circuit = m_vm->getPrototypeCircuit();
                circuit.disconnectInternal(a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);
                // Queue return message
                auto f = [this, a_outputPort, a_inputPort]() {
                            UnitWidget* fromWidget = m_unitWidgets[a_outputPort.first];
                            UnitWidget* toWidget = m_unitWidgets[a_inputPort.first];
                            updateUnitPos(fromWidget, fromWidget->position(), true);
                            updateUnitPos(toWidget, toWidget->position(), true);
                        };
                m_window->queueExternalMessage(syn::MakeCommand(f));
            };
    m_vm->queueAction(syn::MakeCommand(f));
}

void synui::CircuitWidget::createConnection(const Port& a_inputPort, const Port& a_outputPort) {
    // send the new connection request to the real-time thread
    auto f = [this, a_inputPort, a_outputPort]() {
                for (int i = 0; i < m_vm->getMaxVoices(); i++) {
                    syn::Circuit& circuit = m_vm->getVoiceCircuit(i);
                    circuit.connectInternal(a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);
                }
                syn::Circuit& circuit = m_vm->getPrototypeCircuit();
                circuit.connectInternal(a_outputPort.first, a_outputPort.second, a_inputPort.first, a_inputPort.second);

                // Queue return message
                auto f = [this, a_outputPort, a_inputPort]() {
                            this->createWireWidget(a_inputPort, a_outputPort);
                        };
                m_window->queueExternalMessage(syn::MakeCommand(f));
            };

    m_vm->queueAction(syn::MakeCommand(f));
}

void synui::CircuitWidget::createWireWidget(const Port& a_inputPort, const Port& a_outputPort) {
    // Remove any wire widgets that already point to that unit input
    for (int i = 0; i < m_wires.size(); i++) {
        auto wire = m_wires[i];
        if (wire->getInputPort() == a_inputPort) {
            deleteWireWidget(wire);
            i--;
        }
    }

    std::shared_ptr<CircuitWire> wire = std::make_shared<CircuitWire>(this);
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
    m_grid.map([&](GridCell& cell) { cell.remove({GridCell::State::Unit, a_unitWidget});});

    /* Place the unit and reroute the wires */
    std::unordered_set<std::shared_ptr<CircuitWire>> wires;
    // Record any wires we may be overwriting
    for (int r = 0; r < blk.rows(); r++) {
        for (int c = 0; c < blk.cols(); c++) {
            const GridCell& cellContents = blk(r, c);
            for (const auto& state : cellContents.states) {
                if (state.type == GridCell::State::Wire)
                    wires.insert(reinterpret_cast<CircuitWire*>(state.ptr)->shared_from_this());
            }
        }
    }
    // Place the unit
    m_grid.forceMapBlock(topLeftCell, bottomRightCell, [&](GridCell& cell) { cell.states.push_back({GridCell::State::Unit, a_unitWidget}); }, false);
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
    auto condition = [a_unitWidget](const GridCell& x) { return x.contains(GridCell::State::Unit) && !x.contains(a_unitWidget) ? x : GridCell{}; };
    return !blk.unaryExpr(condition).array().any();
}

void synui::CircuitWidget::deleteWireWidget(std::shared_ptr<CircuitWire> wire) {
    if (!wire)
        return;
    m_wires.erase(find(m_wires.begin(), m_wires.end(), wire->shared_from_this()));
    
#ifndef NDEBUG
    // Verify connections
    for (auto w : m_wires) {
        const syn::Circuit& circ = vm().getPrototypeCircuit();
        syn::ConnectionRecord conn{w->getOutputPort().first, w->getOutputPort().second, w->getInputPort().first, w->getInputPort().second};
        const vector<syn::ConnectionRecord>& connections = circ.getConnections();
        assert(std::find(connections.begin(), connections.end(), conn) != connections.end());
    }
#endif

}

void synui::CircuitWidget::deleteUnitWidget(UnitWidget* widget) {
    int unitId = widget->getUnitId();
    
    this->onRemoveUnit(m_unitWidgets[unitId]);

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
    m_grid.map([&](GridCell& cell) { cell.remove({GridCell::State::Unit, m_unitWidgets[unitId]});});
    removeChild(m_unitWidgets[unitId]);
    m_unitWidgets.erase(unitId);
}

void synui::CircuitWidget::createJunction(std::shared_ptr<CircuitWire> a_toWire, std::shared_ptr<CircuitWire> a_fromWire, const Vector2i& a_pos, syn::UnitTypeId a_classId) {
    auto unit = syn::UnitFactory::instance().createUnit(a_classId);
    auto f = [this, unit, a_toWire, a_fromWire, a_pos]() {
                for (int i = 0; i < m_vm->getMaxVoices(); i++) {
                    m_vm->getVoiceCircuit(i).addUnit(unit->clone());
                }
                int unitId = m_vm->getPrototypeCircuit().addUnit(unit);
                // Queue return message
                auto f = [this, unit, unitId, a_toWire, a_fromWire, a_pos]() {
                            UnitWidget* uw = createUnitWidget(unit->getClassIdentifier(), unitId);
                            Vector2i pos = a_pos - Vector2i::Ones() * gridSpacing();
                            auto onSuccess = [this, unitId, a_fromWire, a_toWire]() {
                                        // Connect wires to the new unit.
                                        createConnection({unitId, 0}, a_fromWire->getOutputPort());
                                        createConnection({unitId, 1}, a_toWire->getOutputPort());
                                        // Connect the new unit to the original wire's destination
                                        createConnection(a_toWire->getInputPort(), {unitId,0});
                                    };
                            if (checkUnitPos(uw, pos)) {
                                updateUnitPos(uw, pos);
                                uw->triggerEditorCallback();
                                onSuccess();
                            } else {
                                deleteUnitWidget(uw);
                                _changeState(new cwstate::CreatingUnitState(unitId, onSuccess));
                            }                    

                        #ifndef NDEBUG
                            // Verify connections
                            for (auto w : m_wires) {
                                const syn::Circuit& circ = vm().getPrototypeCircuit();
                                syn::ConnectionRecord conn{w->getOutputPort().first, w->getOutputPort().second, w->getInputPort().first, w->getInputPort().second};
                                const vector<syn::ConnectionRecord>& connections = circ.getConnections();
                                assert(std::find(connections.begin(), connections.end(), conn) != connections.end());
                            }
                        #endif
                        };
                m_window->queueExternalMessage(syn::MakeCommand(f));
            };
    m_window->queueExternalMessage(syn::MakeCommand(f));
}

void synui::CircuitWidget::_changeState(cwstate::State* a_state) {
    if (m_state != nullptr) {
        m_state->exit(*this, *a_state);
    }
    a_state->enter(*this, *m_state);
    m_state.reset(a_state);
}

bool synui::cwstate::IdleState::mouseButtonEvent(CircuitWidget& cw, const Vector2i& p, int button, bool down, int modifiers) {
    m_clickedWidget = nullptr;
    bool captured = cw.Widget::mouseButtonEvent(p, button, down, modifiers);

    Vector2i mousePos = p - cw.position();
    Grid2DPoint pt = cw.grid().fromPixel(mousePos, cw.gridSpacing());
    if (!cw.grid().contains(pt)) {
        pt[0] = syn::CLAMP<int>(pt[0], 0, cw.grid().getShape()[0]-1);
        pt[1] = syn::CLAMP<int>(pt[1], 0, cw.grid().getShape()[1]-1);
    }

    const auto& cell = cw.grid().get(pt);
    // Reset the current selection if it does not contain the clicked unit widget.
    for (const auto& s : cell.states) {
        if (s.type == CircuitWidget::GridCell::State::Unit) {
            UnitWidget* w = reinterpret_cast<UnitWidget*>(s.ptr);
            if (cw.unitSelection().find(w) == cw.unitSelection().end()) {
                cw.unitSelection().clear();
                break;
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && down) {
        for (const auto& s : cell.states) {
            if (s.type == CircuitWidget::GridCell::State::Unit) {
                UnitWidget* uw = reinterpret_cast<UnitWidget*>(s.ptr);
                int unitId = uw->getUnitId();
                bool isOutput = true;
                int portId = uw->getOutputPort(mousePos - uw->position());
                if (portId < 0) {
                    isOutput = false;
                    portId = uw->getInputPort(mousePos - uw->position());
                }
                if (portId >= 0) {
                    changeState(cw, *new DrawingWireState({unitId,portId}, isOutput));
                    return true;
                }
                m_clickedWidget = uw;
                return true;
            }
        }
        if (!captured) {
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

bool synui::cwstate::IdleState::mouseMotionEvent(CircuitWidget& cw, const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
    // Remove old highlighted wire
    if (m_highlightedWire) {
        m_highlightedWire->highlight = CircuitWire::None;
        m_highlightedWire = nullptr;
    }

    Grid2DPoint gridPt = cw.grid().fromPixel(p, cw.gridSpacing());
    if (!cw.grid().contains(gridPt))
        return true;

    // Find new highlighted wire, if any
    const auto& cell = cw.grid().get(gridPt);
    for (const auto& s : cell.states) {
        if (s.type == CircuitWidget::GridCell::State::Wire) {
            if (s.ptr != m_highlightedWire.get()) {
                if (m_highlightedWire) {
                    m_highlightedWire->highlight = CircuitWire::None;
                }
                m_wireHighlightTime = glfwGetTime();
                m_highlightedWire = reinterpret_cast<CircuitWire*>(s.ptr)->shared_from_this();
                m_highlightedWire->highlight = CircuitWire::Selected;
                break;
            }
        }
    }

    if (m_clickedWidget) {
        changeState(cw, *new MovingUnitState());
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
    if (cw.grid().get(mousePt).contains(CircuitWidget::GridCell::State::Wire) && m_startedFromOutput) {
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
    Vector2i mousePos = cw.screen()->mousePos() - cw.absolutePosition();
    m_wire = std::make_shared<CircuitWire>(&cw);
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

bool synui::cwstate::DrawingWireState::mouseButtonEvent(CircuitWidget& cw, const Vector2i& p, int button, bool down, int modifiers) {
    if (!down) {
        Vector2i mousePos = p - cw.position();
        Grid2DPoint mousePt = cw.grid().fromPixel(mousePos, cw.gridSpacing());
        auto unitState = cw.grid().get(mousePt).find(CircuitWidget::GridCell::State::Unit);
        if (unitState!=cw.grid().get(mousePt).states.end()) {
            UnitWidget* unit = reinterpret_cast<UnitWidget*>(unitState->ptr);
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
        } 
        auto wireState = cw.grid().get(mousePt).find(CircuitWidget::GridCell::State::Wire);
        if (wireState!=cw.grid().get(mousePt).states.end() && m_startedFromOutput) {
            std::shared_ptr<CircuitWire> toWire = static_cast<CircuitWire*>(wireState->ptr)->shared_from_this();
            std::shared_ptr<CircuitWire> fromWire = std::make_shared<CircuitWire>(&cw);
            fromWire->setInputPort(m_wire->getInputPort());
            fromWire->setOutputPort(m_wire->getOutputPort());
            ContextMenu<int>* cm = new ContextMenu<int>(&cw, true, [&cw, fromWire, toWire, mousePos](const string& a_item, const int& a_value) {
                    if (a_value == 0) {
                        cw.createJunction(toWire, fromWire, mousePos, syn::SummerUnit::classIdentifier());
                    } else if (a_value == 1) {
                        cw.createJunction(toWire, fromWire, mousePos, syn::GainUnit::classIdentifier());
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

bool synui::cwstate::CreatingUnitState::mouseButtonEvent(CircuitWidget& cw, const Vector2i& p, int button, bool down, int modifiers) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && down) {
        Vector2i a_pos = p - cw.position();
        a_pos -= m_widget->size() / 2;
        // Place the unit if the position is valid
        if (cw.checkUnitPos(m_widget, a_pos)) {
            m_widget->setVisible(true);
            m_widget->setEnabled(true);
            cw.updateUnitPos(m_widget, a_pos);
            m_widget->triggerEditorCallback();
            m_isValid = true;
            changeState(cw, *new IdleState());
        } else {
            cw.deleteUnit(m_unitId);
            m_widget = nullptr;
            m_isValid = false;
            changeState(cw, *new IdleState());
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && down) {
        cw.deleteUnit(m_unitId);
        m_widget = nullptr;
        m_isValid = false;
        changeState(cw, *new IdleState());
    }
    return true;
}

void synui::cwstate::CreatingUnitState::draw(CircuitWidget& cw, NVGcontext* ctx) {
    Vector2i mousePos = cw.screen()->mousePos() - cw.absolutePosition();
    mousePos -= m_widget->size() / 2;
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
}

void synui::cwstate::CreatingUnitState::exit(CircuitWidget& cw, State& newState) {
    if (m_isValid) {
        m_onSuccess();
    } else {
        m_onFailure();
    }
}

bool synui::cwstate::MovingUnitState::mouseButtonEvent(CircuitWidget& cw, const Vector2i& p, int button, bool down, int modifiers) {
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

bool synui::cwstate::MovingUnitState::mouseMotionEvent(CircuitWidget& cw, const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
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
    auto unitState = cell.find(CircuitWidget::GridCell::State::Unit);
    if (unitState != cell.states.end()) {
        UnitWidget* w = reinterpret_cast<UnitWidget*>(unitState->ptr);
        if (cw.unitSelection().find(w) == cw.unitSelection().end()) {
            cw.unitSelection().clear();
            cw.unitSelection().insert(w);
        }
    }
    // Remove selected units from grid while they're being moved.
    for (auto& w : cw.unitSelection()) {
        cw.grid().map([&](auto& c){ c.remove({CircuitWidget::GridCell::State::Unit, w}); });
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
            auto unitState = blk(r, c).find(CircuitWidget::GridCell::State::Unit);
            if (unitState!=blk(r, c).states.end())
                cw.unitSelection().insert(reinterpret_cast<UnitWidget*>(unitState->ptr));
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
