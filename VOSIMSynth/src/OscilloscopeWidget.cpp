#include "vosimsynth/widgets/OscilloscopeWidget.h"
#include "vosimsynth/VOSIMTheme.h"
#include <vosimlib/DSPMath.h>
#include <nanovg.h>
#include <nanogui/screen.h>

namespace synui {
    OscilloscopeUnit::OscilloscopeUnit(const string& a_name)
        :
        Unit(a_name),
        m_readIndex(0),
        m_writeIndex(0),
        m_numBuffers(1),
        m_lastPhase(0.0),
        m_samplesSinceLastSync(0),
        m_syncCount(0) 
    {
        addParameter_(pBufferSize, syn::UnitParameter("buffer size", 2, MAX_SCOPE_BUFFER_SIZE, 256, syn::UnitParameter::Samples));
        addParameter_(pIsPeriodic, syn::UnitParameter("periodic", true));
        addParameter_(pNumPeriods, syn::UnitParameter("periods", 1, 16, 1));
        m_bufferSize = param(pBufferSize).getInt();
        m_buffers.resize(m_numBuffers);
        for (int i = 0; i < m_numBuffers; i++) {
            addInput_("in" + std::to_string(i));
            m_buffers[i].resize(param(pBufferSize).getMax());
            m_buffers[i].fill(0.0f);
        }
        m_iPhase = addInput_("sync");
    }

    int OscilloscopeUnit::getNumBuffers() const {
        return m_buffers.size();
    }

    syn::CircularView<double> OscilloscopeUnit::getBuffer(int a_bufIndex) const {        
        return { m_buffers[a_bufIndex].data(), (int)m_buffers[a_bufIndex].size(), m_bufferSize, m_readIndex };
    }

    void OscilloscopeUnit::reset() {
        m_writeIndex = 0;
        m_samplesSinceLastSync = 0;
        m_lastPhase = 0.0;
        m_syncCount = 0;
        for (int i = 0; i < m_numBuffers; i++) {
            m_buffers[i].fill(0.0f);
        }
    }

    void OscilloscopeUnit::onParamChange_(int a_paramId) {
        switch (a_paramId) {
        case pBufferSize:
        {
            int newBufferSize = param(pBufferSize).getInt();
            m_writeIndex = syn::WRAP(m_writeIndex, newBufferSize);
            m_bufferSize = newBufferSize;
            break;
        }
        case pIsPeriodic:
        {
            param(pNumPeriods).setVisible(param(pIsPeriodic).getBool());
            break;
        }
        default: break;
        }
    }

    void OscilloscopeUnit::onNoteOn_() {
        if (!param(pIsPeriodic).getBool() && !isConnected(m_iPhase))
            _syncNonPeriodic();
    }

    void OscilloscopeUnit::process_() {
        BEGIN_PROC_FUNC
            // Read data into buffers and increment buffer pointer
            for (int i = 0; i < m_buffers.size(); i++) {
                m_buffers[i][m_writeIndex] = READ_INPUT(i);
            }
            if (param(pIsPeriodic).getBool())
                _processPeriodic();
            else
                _processNonPeriodic();
        END_PROC_FUNC
    }

    void OscilloscopeUnit::_processPeriodic() {
        double phase = READ_INPUT(m_iPhase);
        // sync on phase reset
        if (phase - m_lastPhase < -0.5) {
            _syncPeriodic();
        }
        m_samplesSinceLastSync++;
        m_lastPhase = phase;
        m_readIndex = 0;
        m_writeIndex = syn::WRAP<int>(m_writeIndex + 1, m_bufferSize);
    }

    void OscilloscopeUnit::_processNonPeriodic() {        
        double phase = READ_INPUT(m_iPhase);
        // sync on falling edge
        if (phase - m_lastPhase < -0.5) {
            _syncNonPeriodic();
        }
        m_lastPhase = phase;
        m_readIndex = syn::WRAP<int>(m_writeIndex + 1 - m_bufferSize, MAX_SCOPE_BUFFER_SIZE);
        m_writeIndex = syn::WRAP<int>(m_writeIndex + 1, MAX_SCOPE_BUFFER_SIZE);
    }

    void OscilloscopeUnit::_syncPeriodic() {
        m_syncCount++;
        int numPeriods = param(pNumPeriods).getInt();
        if (m_syncCount >= numPeriods) {
            double bufSize = m_samplesSinceLastSync;
            double period = m_samplesSinceLastSync * (1.0 / numPeriods);
            // Try to force number of periods so that buffer size is in range.
            while (bufSize > param(pBufferSize).getMax() && numPeriods>1) {
                bufSize -= period;
                numPeriods--;
            }
            setParam(pBufferSize, bufSize);
            setParam(pNumPeriods, numPeriods);
            m_samplesSinceLastSync = 0;
            m_syncCount = 0;
            m_writeIndex = 0;
        }
    }

    void OscilloscopeUnit::_syncNonPeriodic() {
        m_writeIndex = 0;
    }

    void OscilloscopeWidget::draw(NVGcontext* ctx) {
        const auto& circuit = m_vm->getPrototypeCircuit();
        auto unitClassId = circuit.getUnit(m_unitId).getClassIdentifier();
        if (unitClassId == OscilloscopeUnit::classIdentifier()) {
            const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
            this->setCaption(unit->name());
            setValues(unit->getBuffer(0));
            std::pair<int, int> argMinMax = m_values.argMinMax();
            double yMin = m_values[argMinMax.first];
            double yMax = m_values[argMinMax.second];
            updateYBounds_(yMin, yMax);

            std::ostringstream oss;
            oss << std::setprecision(4) << "ylim: (" << yMin << ", " << yMax << ")";
            setFooter(oss.str());
        }

        Widget::draw(ctx);
        drawGrid(ctx);
        
        nvgSave(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());
        nvgBeginPath(ctx);
        nvgRect(ctx, 0.0f, 0.0f, mSize.x(), mSize.y());
        nvgFillColor(ctx, m_bgColor);
        nvgFill(ctx);

        if (m_values.size() < 2){
            nvgRestore(ctx);
            return;
        }

        nvgBeginPath(ctx);
        for (int i = 0; i < m_values.size(); i++) {
            double value = m_values[i];
            double vx = m_leftMargin + i * (mSize.x()-m_leftMargin) / static_cast<double>(m_values.size() - 1);
            double vy = toScreen_(value);
            if(i==0)
                nvgMoveTo(ctx, vx, vy);
            else
                nvgLineTo(ctx, vx, vy);
        }
        nvgStrokeColor(ctx, m_fgColor);
        nvgStroke(ctx);

        nvgFontFace(ctx, "sans");

        m_bgColor = theme()->get<Color>("/OscilloscopeWidget/bg-color", {20, 128});
        m_fgColor = theme()->get<Color>("/OscilloscopeWidget/fg-color", {255, 192, 0, 128});
        m_textColor = theme()->get<Color>("/OscilloscopeWidget/text-color", {240, 192});

        if (!m_caption.empty()) {
            nvgFontSize(ctx, 14.0f);
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, m_textColor);
            nvgText(ctx, 3, 1, m_caption.c_str(), nullptr);
        }

        if (!m_header.empty()) {
            nvgFontSize(ctx, 18.0f);
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, m_textColor);
            nvgText(ctx, mSize.x() - 3, 1, m_header.c_str(), nullptr);
        }

        if (!m_footer.empty()) {
            nvgFontSize(ctx, 15.0f);
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
            nvgFillColor(ctx, m_textColor);
            nvgText(ctx, mSize.x() - 3, mSize.y() - 1, m_footer.c_str(), nullptr);
        }

        nvgBeginPath(ctx);
        nvgRect(ctx, 0, 0, 0, 0);
        nvgStrokeColor(ctx, nanogui::Color(100, 255));
        nvgStroke(ctx);
        nvgRestore(ctx);
    }

    void OscilloscopeWidget::drawGrid(NVGcontext* ctx) {
        nvgSave(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());
        nanogui::Color tickColor{nvgHSL(0.19f, 1.0f, 0.5f)}; tickColor.w() = 0.9f;
        nanogui::Color tickLabelColor{nvgHSL(0.19f, 0.7f, 0.5f)}; tickColor.w() = 0.25f;
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, 12.0f);
        nvgStrokeColor(ctx, tickColor);
        nvgFillColor(ctx, tickLabelColor);
        nvgBeginPath(ctx);

        // Zero Y tick
        double zeroPxY = toScreen_(0.0);
        nvgMoveTo(ctx, 0.0f, zeroPxY);
        nvgLineTo(ctx, size().x(), zeroPxY);

        // Other Y ticks
        const int numYTicks = 10;
        const float yTickStep = (m_yMax-m_yMin)/numYTicks;
        const float tickSize = m_leftMargin+2;
        double yTick = +yTickStep;
        std::ostringstream oss;
        oss << std::setprecision(4);
        // Positive ticks
        while (yTick <= m_yMax) {
            float px = toScreen_(yTick);
            nvgMoveTo(ctx, 0.0f, px);
            nvgLineTo(ctx, tickSize, px);
            oss << yTick;
            nvgText(ctx, 0.0f, px, oss.str().c_str(), nullptr);
            oss.seekp(0);
            yTick += yTickStep;
        }
        // Negative ticks
        yTick = -yTickStep;
        while (yTick >= m_yMin) {
            float px = toScreen_(yTick);
            nvgMoveTo(ctx, 0.0f, px);
            nvgLineTo(ctx, tickSize, px);
            oss << yTick;
            nvgText(ctx, 0.0f, px, oss.str().c_str(), nullptr);
            oss.seekp(0);
            yTick -= yTickStep;
        }
        nvgStroke(ctx);
        nvgRestore(ctx);
    }

    void OscilloscopeWidget::updateYBounds_(float a_yMin, float a_yMax) {
        double lo = a_yMin, hi = a_yMax;
        bool isextremanan = isnan(lo) || isnan(hi);
        if (isextremanan) {
            lo = -1;
            hi = 1;
        }
        double margin = 0.10 * syn::MAX(hi - lo, 1E-1);
        double minSetPoint = lo - margin;
        double maxSetPoint = hi + margin;
        double coeff = m_autoAdjustSpeed / (2*DSP_PI*screen()->fps());
        m_yMin = m_yMin + coeff * (minSetPoint - m_yMin);
        m_yMax = m_yMax + coeff * (maxSetPoint - m_yMax);
    }

    float OscilloscopeWidget::toScreen_(float a_yPt) {
        return syn::LERP<float>(mSize.y()-m_bottomMargin, 0, syn::INVLERP<float>(m_yMin, m_yMax, a_yPt));
    }

    float OscilloscopeWidget::fromScreen_(float a_yScreen) {        
        return syn::LERP<float>(m_yMin, m_yMax, syn::INVLERP<float>(mSize.y()-m_bottomMargin, 0, a_yScreen));
    }
}
