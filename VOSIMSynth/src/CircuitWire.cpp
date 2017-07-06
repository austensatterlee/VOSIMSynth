#include "CircuitWire.h"
#include "VoiceManager.h"
#include "DSPMath.h"
#include "UnitWidget.h"

std::string synui::CircuitWire::info() const {
    if (!m_isFinal)
        return "";

    std::ostringstream os;
    /// @DEBUG Show wire cost
    os << "Cost: " << m_cost << std::endl;

    // List current output value for this wire held by each active voice
    std::vector<int> activeVoiceIndices = m_parentCircuit->m_vm->getActiveVoiceIndices();
    for (int vind : activeVoiceIndices) {
        const syn::Unit& unit = m_parentCircuit->m_vm->getUnit(m_outputPort.first, vind);
        double value = unit.readOutput(m_outputPort.second, 0);
        os << "Voice " << vind << ": " << std::setprecision(4) << value << std::endl;
    }
    return os.str();
}

void synui::CircuitWire::draw(NVGcontext* ctx) {
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
            wireColor = nanogui::Color(0.45f, 0.45f, 1.00f, 0.6f);
            break;
        case Outgoing:
            wireColor = nanogui::Color(0.25f, 1.0f, 0.25f, 0.6f);
            break;
        case Selected:
            nvgStrokeWidth(ctx, 2.0f);
            wireColor = nanogui::Color(0.75f, 0.75f, 0.75f, 0.75f);
            break;
        case None:
        default:
            wireColor = nanogui::Color(0.85f, 0.20f, 0.10f, 0.50f);
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

        Eigen::Vector2i currPixelPt = i >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(currGridPt, gs);
        nvgLineTo(ctx, currPixelPt.x(), currPixelPt.y());

        if (m_parentCircuit->wireDrawStyle() == CircuitWidget::Curved) {
            const float curvature = 0.5;
            Eigen::Vector2i dCurrGridPt = nextGridPt - currGridPt;
            Eigen::Vector2i dNextGridPt = nextGridPt2 - nextGridPt;
            Eigen::Vector2i dNextGridPt2 = nextGridPt3 - nextGridPt2;
            // Draw a curve when the path changes direction.
            if (dNextGridPt.dot(dCurrGridPt) == 0) {
                Eigen::Vector2i nextPixelPt = i + 1 >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(nextGridPt, gs);
                Eigen::Vector2i nextPixelPt2 = i + 2 >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(nextGridPt2, gs);
                // Handle "S-curves" and corner turns.
                if ((dNextGridPt2.array()!=0).any() && dNextGridPt2.dot(dNextGridPt) == 0) {
                    Eigen::Vector2i nextPixelPt3 = i + 3 >= m_path.size() - 1 ? m_end : m_parentCircuit->m_grid.toPixel(nextGridPt3, gs);
                    Eigen::Vector2f a1 = (nextPixelPt3-currPixelPt).cast<float>();
                    Eigen::Vector2f b1 = (nextPixelPt-currPixelPt).cast<float>().normalized();
                    Eigen::Vector2i ctrlPt1 = (a1.dot(b1)*curvature*b1).cast<int>()+currPixelPt;
                    Eigen::Vector2f a2 = (currPixelPt-nextPixelPt3).cast<float>();
                    Eigen::Vector2f b2 = (nextPixelPt2-nextPixelPt3).cast<float>().normalized();
                    Eigen::Vector2i ctrlPt2 = (a2.dot(b2)*curvature*b2).cast<int>()+nextPixelPt3;
                    nvgBezierTo(ctx, ctrlPt1.x(), ctrlPt1.y(), ctrlPt2.x(), ctrlPt2.y(), nextPixelPt3.x(), nextPixelPt3.y());
                    i += 2;
                } else {
                    Eigen::Vector2i ctrlPt1 = ((nextPixelPt-currPixelPt).cast<float>()*curvature + currPixelPt.cast<float>()).cast<int>();
                    Eigen::Vector2i ctrlPt2 = ((nextPixelPt-nextPixelPt2).cast<float>()*curvature + nextPixelPt2.cast<float>()).cast<int>();
                    nvgBezierTo(ctx, ctrlPt1.x(), ctrlPt1.y(), ctrlPt2.x(), ctrlPt2.y(), nextPixelPt2.x(), nextPixelPt2.y());
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

        Eigen::Vector2i pt = m_end;
        Eigen::Vector2i prevPt = m_parentCircuit->m_grid.toPixel(m_path[m_path.size() - 2], m_parentCircuit->gridSpacing());

        Eigen::Vector2f dir = (pt - prevPt).cast<float>();
        dir.normalize();
        float dangle = asin(dir[1]);

        Eigen::Vector2f headOffset = Eigen::Vector2f{0, 1} * noseSize;
        Eigen::Vector2f noseOffset = Eigen::Vector2f{noseSize / sin(2 * DSP_PI / 180.0 * noseAngle), 0};

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

void synui::CircuitWire::updateStartAndEndPositions(const Eigen::Vector2i& a_endPt) {
    // Determine start/end points
    Eigen::Vector2i startPos;
    Eigen::Vector2i endPos = m_end;
    if (m_inputPort.first >= 0 && m_outputPort.first >= 0) {
        m_isFinal = true;
        auto inputWidget = m_parentCircuit->m_unitWidgets[m_inputPort.first];
        auto outputWidget = m_parentCircuit->m_unitWidgets[m_outputPort.first];
        Eigen::Vector2i inputPos = inputWidget->getInputPortAbsPosition(m_inputPort.second) - m_parentCircuit->absolutePosition();
        Eigen::Vector2i outputPos = outputWidget->getOutputPortAbsPosition(m_outputPort.second) - m_parentCircuit->absolutePosition();
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

template <typename CellType>
int synui::CircuitWire::weight_func<CellType>::operator()(const Grid2D<CellType>& grid, const Grid2DPoint& prev, const Grid2DPoint& curr, const Grid2DPoint& next) const {
    int score = 0;
    // Penalize jagged paths
    if ((prev.array() > -1).all()) {
        if (((curr - prev).array() != (next - curr).array()).any())
            score += 10;
    }
    // Penalize following another wire's path
    if (grid.get(curr).contains(CircuitWidget::GridCell::State::Wire) && grid.get(next).contains(CircuitWidget::GridCell::State::Wire))
            score += 25;

    // Prefer empty over a wire, and prefer a wire over a unit.
    const auto& cell = grid.get(next);
    if(cell.contains(CircuitWidget::GridCell::State::Wire))
        score += 25;
    else if(cell.contains(CircuitWidget::GridCell::State::Unit))
        score += 100;
    else if(!cell)
        score += 1;    
    return score;
}

void synui::CircuitWire::updatePath() {
    // Update start and end positions
    updateStartAndEndPositions(m_end);
    auto startCell = m_parentCircuit->m_grid.fromPixel(m_start, m_parentCircuit->gridSpacing());
    auto endCell = m_parentCircuit->m_grid.fromPixel(m_end, m_parentCircuit->gridSpacing());

    // Remove from grid            
    m_parentCircuit->m_grid.map(
        [&](CircuitWidget::GridCell& cell) { cell.remove({CircuitWidget::GridCell::State::Wire, this}); }
    );
    m_crossings.clear();

    // Find new shortest path between start and end cells
    auto path = m_parentCircuit->m_grid.findPath<manhattan_distance, weight_func>(startCell, endCell, &m_cost);
    m_path.resize(path.size());
    copy(path.begin(), path.end(), m_path.begin());

    // Update grid to reflect new location
    for (auto cell : m_path) {
        if (m_parentCircuit->m_grid.get(cell).contains(CircuitWidget::GridCell::State::Wire))
            m_crossings.insert({cell.x(), cell.y()});
        if (!m_parentCircuit->m_grid.isOccupied(cell))
            m_parentCircuit->m_grid.get(cell).states.push_back({CircuitWidget::GridCell::State::Wire, this});
    }
}

float synui::CircuitWire::pathCost() const { return m_cost; }

void synui::CircuitWire::setInputPort(const CircuitWidget::Port& a_port) {
    m_inputPort = a_port;
    updateStartAndEndPositions();
}

void synui::CircuitWire::setOutputPort(const CircuitWidget::Port& a_port) {
    m_outputPort = a_port;
    updateStartAndEndPositions();
}

synui::CircuitWidget::Port synui::CircuitWire::getInputPort() const { return m_inputPort; }
synui::CircuitWidget::Port synui::CircuitWire::getOutputPort() const { return m_outputPort; }

synui::CircuitWire::operator nlohmann::basic_json<>() const {
    json j;
    j["input"] = {m_inputPort.first, m_inputPort.second};
    j["output"] = {m_outputPort.first, m_outputPort.second};
    return j;
}

void synui::CircuitWire::load(const json& j) {
    setInputPort({j["input"][0], j["input"][1]});
    setOutputPort({j["output"][0], j["output"][1]});
    updatePath();
}
