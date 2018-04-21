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
          m_numBuffers(2),
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

    syn::CircularView<double> OscilloscopeUnit::getScopeBuffer(int a_bufIndex) const {
        if (isInputConnected(a_bufIndex)) {
            return syn::CircularView<double>{
                m_buffers[a_bufIndex].data(), int(m_buffers[a_bufIndex].size()), m_bufSize, m_readIndex
            };
        } else {
            return syn::CircularView<double>();
        }
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
                if (isInputConnected(m_iPhase))
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
        if (m_lastPhase - phase > 0.5) {
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
            const double period = m_samplesSinceLastSync * (1.0 / numPeriods);
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

        m_bgColor = theme()->get<Color>("/OscilloscopeWidget/bg-color", {20, 255});
        m_fgColor = theme()->get<Color>("/OscilloscopeWidget/fg-color", {255, 192, 0, 64});
        m_textColor = theme()->get<Color>("/OscilloscopeWidget/text-color", {240, 192});
        m_scopegl->setBackgroundColor(m_bgColor);

        if (!m_caption.empty()) {
            nvgFontFace(ctx, "sans-bold");
            nvgFontSize(ctx, 18.0f);
            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgFillColor(ctx, m_textColor);
            nvgText(ctx, mPos.x() + width() / 2, mPos.y() + 1, m_caption.c_str(), nullptr);
        }

        // Update min/max values
        double yMin = 0, yMax = 0;
        int numBuffers = unit->getNumScopeBuffers();
        for (int i = 0; i < numBuffers; i++) {
            syn::CircularView<double> buf = unit->getScopeBuffer(i);
            if (buf.size()>0) {
                const std::pair<int, int> argMinMax = buf.argMinMax();
                if (i == 0) {
                    yMin = buf[argMinMax.first];
                    yMax = buf[argMinMax.second];
                }
                else {
                    yMin = std::min(buf[argMinMax.first], yMin);
                    yMax = std::max(buf[argMinMax.second], yMax);
                }
            }
        }
        updateYBounds_(yMin, yMax);

        // Send buffer information to ScopeGL
        m_scopegl->setNumBuffers(numBuffers);
        for (int i = 0; i < numBuffers; i++) {
            syn::CircularView<double> buf = unit->getScopeBuffer(i);
            auto currFgColor = m_fgColor.toHSLA();
            currFgColor(0) += i * 1.0 / numBuffers;
            currFgColor = nanogui::Color::fromHSLA(currFgColor);
            m_scopegl->setColor(i, currFgColor);
            // Unroll CircularView into a flat buffer
            Eigen::Matrix2Xf flatValues(2, std::min(buf.size(), width()*2));
            for (int k = 0; k < flatValues.cols(); k++) {
                float sample = k * (buf.size()-1.) * 1.0 / (flatValues.cols()-1.);
                flatValues(0, k) = k * 2.0 / (flatValues.cols() - 1.) - 1.0;
                flatValues(1, k) = syn::INVLERP(m_yMin, m_yMax, buf[int(sample)])*2.0 - 1.0;
            }
            m_scopegl->setValues(i, flatValues);
        }

        Widget::draw(ctx);

        drawGrid(ctx);

        Widget::draw(ctx);
        nvgFontFace(ctx, "sans");
        if (!m_header.empty()) {
            nvgFontSize(ctx, 12.0f);
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, m_textColor);
            nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + 1, m_header.c_str(), nullptr);
        }

        if (!m_footer.empty()) {
            nvgFontSize(ctx, 12.0f);
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
            nvgFillColor(ctx, m_textColor);
            nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + mSize.y() - 1, m_footer.c_str(), nullptr);
        }
    }

    void OscilloscopeWidget::drawGrid(NVGcontext* ctx) {
        const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId,
            m_vm->getNewestVoiceID()));

        nvgSave(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());
        const nanogui::Color tickColor = theme()->get<Color>("/OscilloscopeWidget/tick-color",
            { 0.86f, 1.0f, 0.0f, 1.00f });
        const nanogui::Color tickLabelColor = theme()->get<Color>("/OscilloscopeWidget/tick-label-color",
            { 0.752f, 0.85f, 0.15f, 1.00f });
        nvgFontFace(ctx, "mono");
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgFontSize(ctx, 12.0f);
        nvgStrokeColor(ctx, tickColor);
        nvgFillColor(ctx, tickLabelColor);
        nvgStrokeWidth(ctx, 2.0f);

        const float yTickStartX = m_sideMargin - 5;
        const double yTickSize = (width() - m_sideMargin) - yTickStartX;
        // Zero Y tick
        const double zeroPxY = toScreen_(0.0);
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, yTickStartX, zeroPxY);
        nvgLineTo(ctx, yTickStartX + yTickSize, zeroPxY);
        nvgStroke(ctx);
        // Y ticks
        const int numYTicks = 2;
        double yTickPosStep, yTickNegStep;
        double yTickPosStart, yTickNegStart;
        if (m_yMax >= 0 && m_yMin <= 0) {
            yTickPosStep = abs(m_yMax) / numYTicks;
            yTickNegStep = -abs(m_yMin) / numYTicks;
            yTickPosStart = 0;
            yTickNegStart = yTickNegStep;
        }
        else {
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

                // Try to find a precision that allows the text to fit inside the screen
                for (int i = 4; i >= 0; i--) {
                    std::ostringstream oss;
                    oss << std::setprecision(i);
                    oss << yTick;
                    float txtWidth = nvgTextBounds(ctx, yTickStartX, px, oss.str().c_str(), nullptr, nullptr);
                    if (i == 0 || txtWidth < yTickStartX - 2) {
                        nvgText(ctx, yTickStartX - 2, px, oss.str().c_str(), nullptr);
                        break;
                    }
                }

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

                for (int i = 4; i >= 0; i--) {
                    std::ostringstream oss;
                    oss << std::setprecision(i);
                    oss << yTick;
                    float txtWidth = nvgTextBounds(ctx, yTickStartX, px, oss.str().c_str(), nullptr, nullptr);
                    if (i == 0 || txtWidth < yTickStartX - 2) {
                        nvgText(ctx, yTickStartX - 2, px, oss.str().c_str(), nullptr);
                        break;
                    }
                }

                yTick += yTickNegStep;
            }
        }

        // X ticks
        nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        const int numXTicks = 4;
        const double xMax = unit->getScopeBufferSize() * unit->getDecimationFactor();
        const double xTickStep = 1.0 / numXTicks;
        const double xTickSize = height() - m_bottomMargin - m_topMargin + 5;
        const float xTickStartY = m_topMargin;
        double xTick = 0.0;
        while (xTick <= 1.0) {
            const float px = m_sideMargin + xTick * (width() - 2 * m_sideMargin);
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, px, xTickStartY);
            nvgLineTo(ctx, px, xTickStartY + xTickSize);
            nvgStroke(ctx);

            const float time = xTick * xMax * 1.0 / unit->fs();
            for (int i = 4; i >= 0; i--) {
                std::ostringstream oss;
                oss << std::setprecision(i);
                oss << time;
                float txtWidth = nvgTextBounds(ctx, yTickStartX, px, oss.str().c_str(), nullptr, nullptr);
                if (i == 0 || txtWidth < xTickStep*(width() - 2 * m_sideMargin)) {
                    nvgText(ctx, px, xTickStartY + xTickSize + 2, oss.str().c_str(), nullptr);
                    break;
                }
            }

            xTick += xTickStep;
        }
        nvgRestore(ctx);
    }

    void OscilloscopeWidget::performLayout(NVGcontext* ctx) {
        Widget::performLayout(ctx);
        m_scopegl->setPosition({ m_sideMargin, m_topMargin });
        m_scopegl->setSize({ width() - m_sideMargin * 2, height() - m_topMargin - m_bottomMargin });
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
        const double coeff = m_autoAdjustSpeed / (2 * SYN_PI * screen()->fps());
        m_yMin = m_yMin + coeff * (minSetPoint - m_yMin);
        m_yMax = m_yMax + coeff * (maxSetPoint - m_yMax);

        // Update header and footer with min/max info
        {
            std::ostringstream oss;
            oss << "max: " << std::setprecision(2) << hi;
            setHeader(oss.str());
        }
        {
            std::ostringstream oss;
            oss << "min: " << std::setprecision(2) << lo;
            setFooter(oss.str());
        }
    }

    float OscilloscopeWidget::toScreen_(float a_yPt) {
        return syn::LERP<float>(mSize.y() - m_bottomMargin, m_topMargin,
            syn::INVLERP<float>(m_yMin, m_yMax, a_yPt));
    }

    float OscilloscopeWidget::fromScreen_(float a_yScreen) {
        return syn::LERP<float>(m_yMin, m_yMax,
            syn::INVLERP<float>(mSize.y() - m_bottomMargin, m_topMargin, a_yScreen));
    }
}
