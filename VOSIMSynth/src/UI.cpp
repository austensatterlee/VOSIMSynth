#include "vosimsynth/UI.h"

namespace synui
{
    void drawRectShadow(NVGcontext* ctx, float x, float y, float w, float h, float r, float s, float f, const nanogui::Color& a_shadowColor, const nanogui::Color& a_transparentColor)
    {
        NVGpaint shadowPaint = nvgBoxGradient(
            ctx, x, y, w, h, r * 2, f * 2 * s,
            a_shadowColor, a_transparentColor);

        nvgSave(ctx);
        nvgResetScissor(ctx);
        nvgBeginPath(ctx);
        nvgRect(ctx, x - s, y - s, w + 2 * s, h + 2 * s);
        nvgRoundedRect(ctx, x, y, w, h, r);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadowPaint);
        nvgFill(ctx);
        nvgRestore(ctx);
    }

    void drawRadialShadow(NVGcontext* ctx, float x, float y, float r, float s, float f, const nanogui::Color& a_shadowColor, const nanogui::Color& a_transparentColor)
    {
        NVGpaint shadowPaint = nvgRadialGradient(
            ctx, x, y, r+s, r+s+2*f,
            a_shadowColor, a_transparentColor);

        nvgSave(ctx);
        nvgResetScissor(ctx);
        nvgBeginPath(ctx);
        nvgCircle(ctx, x, y, r+s+2*f);
        nvgCircle(ctx, x, y, r);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadowPaint);
        nvgFill(ctx);
        nvgRestore(ctx);
    }

    void drawTooltip(NVGcontext* a_ctx, const Vector2i& a_pos, const std::string& a_str, double elapsed)
    {
        int tooltipWidth = 150;

        float bounds[4];
        nvgFontFace(a_ctx, "sans");
        nvgFontSize(a_ctx, 15.0f);
        nvgTextAlign(a_ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgTextLineHeight(a_ctx, 1.1f);
        Vector2i pos = a_pos;

        nvgTextBoxBounds(a_ctx, pos.x(), pos.y(), tooltipWidth*1.0f, a_str.c_str(), nullptr, bounds);
        int h = (bounds[2] - bounds[0]) / 2;
        if (h > tooltipWidth / 2)
        {
            nvgTextAlign(a_ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgTextBoxBounds(a_ctx, pos.x(), pos.y(), tooltipWidth*1.0f, a_str.c_str(), nullptr, bounds);

            h = (bounds[2] - bounds[0]) / 2;
        }
        nvgGlobalAlpha(a_ctx, std::min<float>(1.0f, 2 * (elapsed - 0.5f)) * 0.8f);

        nvgBeginPath(a_ctx);
        nvgFillColor(a_ctx, nanogui::Color(0, 255));
        nvgRoundedRect(a_ctx, bounds[0] - 4 - h, bounds[1] - 4, (int)(bounds[2] - bounds[0]) + 8, (int)(bounds[3] - bounds[1]) + 8, 3);

        int px = (int)((bounds[2] + bounds[0]) / 2) - h;
        nvgMoveTo(a_ctx, px, bounds[1] - 10);
        nvgLineTo(a_ctx, px + 7, bounds[1] + 1);
        nvgLineTo(a_ctx, px - 7, bounds[1] + 1);
        nvgFill(a_ctx);

        nvgFillColor(a_ctx, nanogui::Color(255, 255));
        nvgFontBlur(a_ctx, 0.0f);
        nvgTextBox(a_ctx, pos.x() - h, pos.y(), tooltipWidth, a_str.c_str(), nullptr);
    }
}
