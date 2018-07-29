#include "vosimsynth/widgets/ScopeGL.h"

synui::ScopeGL::ScopeGL(Widget* parent)
    : GLCanvas(parent) {
    std::string vs = R"(
    #version 330 core
    in vec2 position;
    void main(){
        gl_Position.xy = position;
        gl_Position.z = 0.0;
        gl_Position.w = 1.0;
    }
    )";
    std::string fs = R"(
    #version 330 core
    out vec4 color;
    uniform vec4 lineColor;
    void main() {
        color = lineColor;
    }
    )";
    m_shader.init("simple_shader", vs, fs);
}

synui::ScopeGL::~ScopeGL() {
    m_shader.free();
}

void synui::ScopeGL::draw(NVGcontext* ctx) {
    // Draw scope background
    nvgBeginPath(ctx);
    nvgFillColor(ctx, backgroundColor());
    nvgRect(ctx, mPos.x(), mPos.y(), width(), height());
    nvgFill(ctx);
    GLCanvas::draw(ctx);
}

void synui::ScopeGL::drawGL() {
    glLineWidth(2.0f);
    m_shader.bind();
    for (int i = 0; i < m_values.size(); i++) {
        m_shader.uploadAttrib("position", m_values[i]);
        m_shader.setUniform("lineColor", m_color[i]);
        m_shader.drawArray(GL_LINE_STRIP, 0, m_values[i].cols());
    }
}
