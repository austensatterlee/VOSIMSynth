#include "vosimsynth/widgets/OscilloscopeWidget.h"
#include "vosimsynth/VOSIMTheme.h"
#include <vosimlib/DSPMath.h>
#include <nanovg.h>
#include <nanogui/screen.h>

/// Maximum scope buffer length
#define MAX_BUF 96000
/// Maximum scope subsampling period
#define MAX_SUBP 4

using syn::UnitParameter;

namespace synui {

    OscilloscopeUnit::OscilloscopeUnit(const string& a_name)
        : Unit(a_name),
          m_readIndex(0),
          m_writeIndex(0),
          m_bufSize(MAX_BUF * MAX_SUBP),
          m_numBuffers(1),
          m_subPeriod(MAX_SUBP),
          m_subCount(0),
          m_lastPhase(0.0),
          m_samplesSinceLastSync(0),
          m_syncCount(0) 
    {
        addParameter_(pBufSize, {"buffer size", 3 * MAX_SUBP, MAX_BUF * MAX_SUBP, m_bufSize, UnitParameter::Samples});
        addParameter_(pNumPeriods, UnitParameter{"periods", 1, 16, 1}.setVisible(false));
        m_buffers.resize(m_numBuffers);
        for (int i = 0; i < m_numBuffers; i++) {
            addInput_("in"+std::to_string(i+1));
            m_buffers[i].resize(MAX_BUF);
            m_buffers[i].fill(0.0f);
        }
        m_iPhase = addInput_("sync");
    }

    int OscilloscopeUnit::getNumBuffers() const {
        return m_buffers.size();
    }

    syn::CircularView<double> OscilloscopeUnit::getBuffer(int a_bufIndex) const {
        int viewSize = isConnected(a_bufIndex) ? m_bufSize : 0;
        return syn::CircularView<double>{
            m_buffers[a_bufIndex].data(), (int)m_buffers[a_bufIndex].size(), viewSize, m_readIndex
        };
    }

    void OscilloscopeUnit::reset() {
        m_writeIndex = 0;
        m_samplesSinceLastSync = 0;
        m_lastPhase = 0.0;
        m_syncCount = 0;
        m_subCount = 0;
        for (int i = 0; i < m_numBuffers; i++) {
            m_buffers[i].fill(0.0f);
        }
    }

    void OscilloscopeUnit::onParamChange_(int a_paramId) {
        switch (a_paramId) {
        case pBufSize:
        {
            m_subPeriod = param(pBufSize).getInt() / MAX_BUF + 1;
            m_bufSize = param(pBufSize).getInt() / m_subPeriod;
            m_writeIndex = syn::WRAP(m_writeIndex, m_bufSize);
            break;
        }
        default:
            break;
        }
    }

    void OscilloscopeUnit::onInputConnection_(int a_inputPort) {
        if (a_inputPort == m_iPhase) {
            param(pNumPeriods).setVisible(true);
        } else {
            // Clear buffer when a new input wire is attached.
            m_buffers[a_inputPort].fill(0.0);
        }
    }

    void OscilloscopeUnit::onInputDisconnection_(int a_inputPort) {
        if (a_inputPort == m_iPhase) {
            param(pNumPeriods).setVisible(false);
        }
    }

    void OscilloscopeUnit::process_() {
        BEGIN_PROC_FUNC
            if (!m_subCount) {
                // Read data into buffers
                for (int i = 0; i < m_buffers.size(); i++) {
                    m_buffers[i][m_writeIndex] = READ_INPUT(i);
                }
                // Increment counters
                if (isConnected(m_iPhase))
                    _processPeriodic();
                else
                    _processNonPeriodic();
            }
            m_subCount = syn::WRAP(m_subCount + 1, m_subPeriod);
        END_PROC_FUNC
    }

    void OscilloscopeUnit::_processPeriodic() {
        const double phase = READ_INPUT(m_iPhase);
        // sync on phase reset
        if (phase - m_lastPhase < -0.5) {
            _sync();
        }
        m_samplesSinceLastSync++;
        m_lastPhase = phase;
        m_readIndex = 0;
        m_writeIndex = syn::WRAP<int>(m_writeIndex + 1, m_bufSize);
    }

    void OscilloscopeUnit::_processNonPeriodic() {
        m_readIndex = syn::WRAP<int>(m_writeIndex + 1 - m_bufSize, MAX_BUF);
        m_writeIndex = syn::WRAP<int>(m_writeIndex + 1, MAX_BUF);
    }

    void OscilloscopeUnit::_sync() {
        m_syncCount++;
        int numPeriods = param(pNumPeriods).getInt();
        if (m_syncCount >= numPeriods) {
            double bufSize = m_samplesSinceLastSync;
            double period = m_samplesSinceLastSync * (1.0 / numPeriods);
            // Try to force number of periods so that buffer size is in range.
            while (bufSize > param(pBufSize).getMax() && numPeriods > 1) {
                bufSize -= period;
                numPeriods--;
            }
            setParam(pBufSize, bufSize * m_subPeriod);
            setParam(pNumPeriods, numPeriods);
            m_samplesSinceLastSync = 0;
            m_syncCount = 0;
            m_writeIndex = 0;
        }
    }

    void OscilloscopeWidget::draw(NVGcontext* ctx) {
        const auto& circuit = m_vm->getPrototypeCircuit();
        auto unitClassId = circuit.getUnit(m_unitId).getClassIdentifier();
        if (unitClassId != OscilloscopeUnit::classIdentifier())
            return;

        const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId,
            m_vm->getNewestVoiceID()));
        this->setCaption(unit->name());
        setValues(unit->getBuffer(0));
        std::pair<int, int> argMinMax = m_values.argMinMax();
        double yMin = m_values[argMinMax.first];
        double yMax = m_values[argMinMax.second];
        updateYBounds_(yMin, yMax);

        {
            std::ostringstream oss;
            oss << " max: " << std::setprecision(2) << yMax;
            setHeader(oss.str());
        }
        {
            std::ostringstream oss;
            oss << "min: " << std::setprecision(2) << yMin;
            setFooter(oss.str());
        }

        Widget::draw(ctx);

        m_bgColor = theme()->get<Color>("/OscilloscopeWidget/bg-color", {20, 128});
        m_fgColor = theme()->get<Color>("/OscilloscopeWidget/fg-color", {255, 192, 0, 128});
        m_textColor = theme()->get<Color>("/OscilloscopeWidget/text-color", {240, 192});
        nvgSave(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());
        nvgBeginPath(ctx);
        nvgRect(ctx, 0.0f, 0.0f, mSize.x(), mSize.y());
        nvgFillColor(ctx, m_bgColor);
        nvgFill(ctx);

        if (!m_caption.empty()) {
            nvgFontFace(ctx, "sans-bold");
            nvgFontSize(ctx, 18.0f);
            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgFillColor(ctx, m_textColor);
            nvgText(ctx, width() / 2, 1, m_caption.c_str(), nullptr);
        }

        if (m_values.size() < 2) {
            nvgFontFace(ctx, "mono");
            nvgFontSize(ctx, 16.0f);
            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFillColor(ctx, m_textColor);
            nvgTextBox(ctx, 0, height() / 2, width(), "[disconnected]", nullptr);
        } else {
            drawGrid(ctx);

            nvgBeginPath(ctx);
            for (int i = 0; i < m_values.size(); i++) {
                double value = m_values[i];
                double vx = m_sideMargin + i * (width() - 2*m_sideMargin) / static_cast<double>(m_values.size() - 1);
                double vy = toScreen_(value);
                if (i == 0)
                    nvgMoveTo(ctx, vx, vy);
                else
                    nvgLineTo(ctx, vx, vy);
            }
            nvgStrokeColor(ctx, m_fgColor);
            nvgStroke(ctx);

            nvgFontFace(ctx, "sans");
            if (!m_header.empty()) {
                nvgFontSize(ctx, 12.0f);
                nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgFillColor(ctx, m_textColor);
                nvgText(ctx, mSize.x() - 3, 1, m_header.c_str(), nullptr);
            }

            if (!m_footer.empty()) {
                nvgFontSize(ctx, 12.0f);
                nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
                nvgFillColor(ctx, m_textColor);
                nvgText(ctx, mSize.x() - 3, mSize.y() - 1, m_footer.c_str(), nullptr);
            }
        }
        nvgRestore(ctx);
    }

    void OscilloscopeWidget::drawGrid(NVGcontext* ctx) {
        const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId));

        nvgSave(ctx);
        const nanogui::Color tickColor = theme()->get<Color>("/OscilloscopeWidget/tick-color",
            {0.86f, 1.0f, 0.0f, 1.00f});
        const nanogui::Color tickLabelColor = theme()->get<Color>("/OscilloscopeWidget/tick-label-color",
            {0.752f, 0.85f, 0.15f, 1.00f});
        nvgFontFace(ctx, "mono");
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgFontSize(ctx, 12.0f);
        nvgStrokeColor(ctx, tickColor);
        nvgFillColor(ctx, tickLabelColor);
        nvgStrokeWidth(ctx, 1.0f);

        const float yTickStartX = m_sideMargin - 5;
        const double yTickSize = (width() - m_sideMargin) - yTickStartX;
        // Zero Y tick
        const double zeroPxY = toScreen_(0.0);
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, yTickStartX, zeroPxY);
        nvgLineTo(ctx, yTickStartX + yTickSize, zeroPxY);
        nvgStroke(ctx);
        // Other Y ticks
        const int numYTicks = 2;
        double yTickPosStep, yTickNegStep;
        double yTickPosStart, yTickNegStart;
        if (m_yMax >= 0 && m_yMin <= 0) {
            yTickPosStep = abs(m_yMax) / numYTicks;
            yTickNegStep = -abs(m_yMin) / numYTicks;
            yTickPosStart = 0;
            yTickNegStart = yTickNegStep;
        } else {
            yTickPosStep = (m_yMax - m_yMin) / (2 * numYTicks);
            yTickNegStep = 0;
            yTickPosStart = m_yMin;
            yTickNegStart = m_yMin - 1;
        }
        // Positive ticks
        if (abs(toScreen_(yTickPosStart) - toScreen_(m_yMax)) >= 16.0) {
            double yTick = yTickPosStart;
            while (yTick <= m_yMax) {
                const float px = toScreen_(yTick);
                nvgBeginPath(ctx);
                nvgMoveTo(ctx, yTickStartX, px);
                nvgLineTo(ctx, yTickStartX + yTickSize, px);
                nvgStroke(ctx);
                std::ostringstream oss;
                oss << std::setprecision(2);
                oss << yTick;
                nvgBeginPath(ctx);
                nvgText(ctx, yTickStartX, px, oss.str().c_str(), nullptr);
                nvgFill(ctx);
                yTick += yTickPosStep;
            }
        }
        // Negative ticks
        if (abs(toScreen_(yTickNegStart) - toScreen_(m_yMin)) >= 16.0) {
            double yTick = yTickNegStart;
            while (yTick >= m_yMin) {
                const float px = toScreen_(yTick);
                nvgBeginPath(ctx);
                nvgMoveTo(ctx, yTickStartX, px);
                nvgLineTo(ctx, yTickStartX + yTickSize, px);
                nvgStroke(ctx);
                std::ostringstream oss;
                oss << std::setprecision(2);
                oss << yTick;
                nvgBeginPath(ctx);
                nvgText(ctx, yTickStartX, px, oss.str().c_str(), nullptr);
                nvgFill(ctx);
                yTick += yTickNegStep;
            }
        }

        // X ticks
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        const int numXTicks = 4;
        const double xMax = m_values.size();
        const double xTickStep = 1.0 / numXTicks;
        const double xTickSize = height() - m_bottomMargin - m_topMargin;
        const float xTickStartY = m_topMargin;
        double xTick = 0.0;
        while (xTick <= 1.0) {
            const float px = m_sideMargin + xTick * (width() - 2*m_sideMargin);
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, px, xTickStartY);
            nvgLineTo(ctx, px, xTickStartY + xTickSize);
            nvgStroke(ctx);

            const int nsample = xTick * xMax * unit->getDecimationFactor();
            std::ostringstream oss;
            oss << std::setprecision(2);
            oss << nsample;
            nvgBeginPath(ctx);
            nvgText(ctx, px, xTickStartY + xTickSize, oss.str().c_str(), nullptr);
            nvgFill(ctx);

            xTick += xTickStep;
        }
        nvgRestore(ctx);
    }

    void OscilloscopeWidget::updateYBounds_(float a_yMin, float a_yMax) {
        double lo = a_yMin, hi = a_yMax;
        const bool isextremanan = isnan(lo) || isnan(hi);
        if (isextremanan) {
            lo = -1;
            hi = 1;
        }
        const double margin = 0.10 * syn::MAX(hi - lo, 1E-1);
        const double minSetPoint = lo - margin;
        const double maxSetPoint = hi + margin;
        const double coeff = m_autoAdjustSpeed / (2 * DSP_PI * screen()->fps());
        m_yMin = m_yMin + coeff * (minSetPoint - m_yMin);
        m_yMax = m_yMax + coeff * (maxSetPoint - m_yMax);
    }

    float OscilloscopeWidget::toScreen_(float a_yPt) {
        return syn::LERP<float>(mSize.y() - m_bottomMargin - m_topMargin, m_topMargin,
            syn::INVLERP<float>(m_yMin, m_yMax, a_yPt));
    }

    float OscilloscopeWidget::fromScreen_(float a_yScreen) {
        return syn::LERP<float>(m_yMin, m_yMax,
            syn::INVLERP<float>(mSize.y() - m_bottomMargin - m_topMargin, m_topMargin, a_yScreen));
    }
}
