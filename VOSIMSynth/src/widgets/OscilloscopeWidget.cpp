#include "OscilloscopeWidget.h"
#include "DSPMath.h"
#include <nanovg.h>
#include <nanogui/screen.h>

namespace synui {
    OscilloscopeUnit::OscilloscopeUnit(const string& a_name)
        :
        Unit(a_name),
        m_widget(nullptr),
        m_bufferIndex(0),
        m_numBuffers(1),
        m_lastPhase(0.0),
        m_lastSync(0),
        m_syncCount(0) {
        addParameter_(pBufferSize, syn::UnitParameter("buffer size", 2, 96000, 256));
        addParameter_(pNumPeriods, syn::UnitParameter("periods", 1, 16, 1));
        m_bufferSize = param(pBufferSize).getInt();
        m_buffers.resize(m_numBuffers);
        for (int i = 0; i < m_numBuffers; i++) {
            addInput_("in" + std::to_string(i));
            m_buffers[i].resize(param(pBufferSize).getMax());
            m_buffers[i].fill(0.0f);
        }
        m_iPhase = addInput_("ph");        
    }

    int OscilloscopeUnit::getNumBuffers() const {
        return m_buffers.size();
    }

    Eigen::Map<const Eigen::VectorXf> OscilloscopeUnit::getBuffer(int a_bufIndex) const {        
        int bufSize = inputSource(a_bufIndex) ? m_bufferSize : 1;
        Eigen::Map<const Eigen::VectorXf> bufView(m_buffers[a_bufIndex].data(), bufSize);
        return bufView;
    }

    void OscilloscopeUnit::onParamChange_(int a_paramId) {
        if (a_paramId == pBufferSize) {
            int newBufferSize = param(pBufferSize).getInt();
            m_bufferIndex = syn::WRAP(m_bufferIndex, newBufferSize);
            m_bufferSize = newBufferSize;
        }
    }

    void OscilloscopeUnit::process_() {
        BEGIN_PROC_FUNC
        double phase = READ_INPUT(m_iPhase);
        if (phase - m_lastPhase < -0.5) {
            _sync();
        }
        m_lastSync++;
        m_lastPhase = phase;
        for (int i = 0; i < m_buffers.size(); i++) {
            m_buffers[i][m_bufferIndex] = READ_INPUT(i);
        }
        m_bufferIndex = syn::WRAP(m_bufferIndex + 1, m_bufferSize);
        END_PROC_FUNC
    }


    void OscilloscopeUnit::_sync() {
        m_syncCount++;
        if (m_syncCount >= param(pNumPeriods).getInt()) {
            setParam(pBufferSize, m_lastSync);
            m_bufferIndex = 0;
            m_lastSync = 0;
            m_syncCount = 0;
        }
    }

    void OscilloscopeWidget::draw(NVGcontext* ctx) {
        const auto& circuit = m_vm->getPrototypeCircuit();
        m_unitId = -1;
        for (int i = 0; i < circuit.getNumUnits(); i++) {
            int unitId = circuit.getUnitId(i);
            auto unitClassId = circuit.getUnit(unitId).getClassIdentifier();
            if (unitClassId == OscilloscopeUnit::classIdentifier())
                m_unitId = unitId;
        }

        if (m_unitId >= 0) {
            const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
            Eigen::Map<const Eigen::VectorXf> bufView = unit->getBuffer(0);
            setValues(bufView);
            updateYBounds(mValues.minCoeff(), mValues.maxCoeff());

            std::ostringstream oss;
            oss << std::setprecision(4) << "ylim: (" << m_yMin << ", " << m_yMax << ")";
            setFooter(oss.str());
        }

        Widget::draw(ctx);
        drawGrid(ctx);
        
        nvgSave(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());
        nvgBeginPath(ctx);
        nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
        nvgFillColor(ctx, mBackgroundColor);
        nvgFill(ctx);

        if (mValues.size() < 2){
            nvgRestore(ctx);
            return;
        }

        nvgBeginPath(ctx);
        for (size_t i = 0; i < (size_t)mValues.size(); i++) {
            float value = mValues[i];
            float vx = mPos.x() + i * mSize.x() / (float)(mValues.size() - 1);
            float vy = mPos.y() + (1 - syn::INVLERP(m_yMin, m_yMax, value)) * mSize.y();
            if(i==0)
                nvgMoveTo(ctx, vx, vy);
            else
                nvgLineTo(ctx, vx, vy);
        }

        nvgStrokeColor(ctx, nanogui::Color(100, 255));
        nvgStroke(ctx);
        nvgStrokeColor(ctx, mForegroundColor);
        nvgStroke(ctx);

        nvgFontFace(ctx, "sans");

        if (!mCaption.empty()) {
            nvgFontSize(ctx, 14.0f);
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, mTextColor);
            nvgText(ctx, mPos.x() + 3, mPos.y() + 1, mCaption.c_str(), nullptr);
        }

        if (!mHeader.empty()) {
            nvgFontSize(ctx, 18.0f);
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            nvgFillColor(ctx, mTextColor);
            nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + 1, mHeader.c_str(), nullptr);
        }

        if (!mFooter.empty()) {
            nvgFontSize(ctx, 15.0f);
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
            nvgFillColor(ctx, mTextColor);
            nvgText(ctx, mPos.x() + mSize.x() - 3, mPos.y() + mSize.y() - 1, mFooter.c_str(), nullptr);
        }

        nvgBeginPath(ctx);
        nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
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
        double zeroPxY = toScreen(0.0);
		nvgMoveTo(ctx, 0.0f, zeroPxY);
		nvgLineTo(ctx, size().x(), zeroPxY);

		// Other Y ticks
		const int numYTicks = 10;
		const float yTickStep = (m_yMax-m_yMin)/numYTicks;
        const float tickSize = 10.0f;
		double yTick = +yTickStep;
		std::ostringstream oss;
        oss << std::setprecision(4);
        // Positive ticks
		while (yTick <= m_yMax) {
			float px = toScreen(yTick);
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
			float px = toScreen(yTick);
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

    void OscilloscopeWidget::updateYBounds(float a_yMin, float a_yMax) {
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

    float OscilloscopeWidget::toScreen(float a_yPt) {
        return syn::LERP<float>(mSize.y(), 0, syn::INVLERP<float>(m_yMin, m_yMax, a_yPt));
    }

    float OscilloscopeWidget::fromScreen(float a_yScreen) {        
        return syn::LERP<float>(m_yMin, m_yMax, syn::INVLERP<float>(mSize.y(), 0, a_yScreen));
    }
}
