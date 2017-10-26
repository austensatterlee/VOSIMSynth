#include "vosimsynth/widgets/ScopeGL.h"

synui::ScopeGL::ScopeGL(Widget* parent)
    : GLCanvas(parent) {
    m_shader.init("simple_shader",
        "#version 330 core\n"
        "in vec2 position;\n"
        "void main(){\n"
        "gl_Position.xy = position;\n"
        "gl_Position.z = 0.0;\n"
        "gl_Position.w = 1.0;\n"
        "}\n",
        "#version 330 core\n"
        "out vec4 color;\n"
        "void main() {\n"
        "color = vec4(1, 0, 0, 1);\n"
        "}\n");
}

synui::ScopeGL::~ScopeGL() {
    m_shader.free();
}

void synui::ScopeGL::setValues(const Eigen::MatrixXf& a_values) {
    m_values = a_values;
}

void synui::ScopeGL::drawGL() {
    m_shader.bind();
    m_shader.uploadAttrib("position", m_values);
    glLineWidth(2.0);
    m_shader.drawArray(GL_LINE_STRIP, 0, m_values.cols());
}
