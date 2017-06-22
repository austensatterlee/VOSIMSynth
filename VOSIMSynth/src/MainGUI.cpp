#include "MainGUI.h"
#include "MainWindow.h"
#include "VoiceManager.h"
#include "CircuitWidget.h"
#include "UnitEditor.h"
#include "Logger.h"
#include "OscilloscopeWidget.h"

#include <Command.h>
#include <UnitFactory.h>
#include <nanogui/nanogui.h>
#include <nanogui/theme.h>
#include <IPlug/Log.h>

namespace synui {
    class EnhancedWindow : public nanogui::Window {
    public:
        typedef std::function<void(EnhancedWindow*, NVGcontext*)> DrawFunc;

        EnhancedWindow(Widget* a_parent, const string& a_title)
            : Window(a_parent, a_title) {
            if (!a_title.empty()) {
                // Close button
                auto settings_close_button = new nanogui::Button(buttonPanel(), "", ENTYPO_CROSS);
                settings_close_button->setFixedWidth(18);
                settings_close_button->setCallback([this]() { setVisible(false); });
            }
        }

        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override {
            if (!title().empty()) {
                return Window::mouseButtonEvent(p, button, down, modifiers);
            } else {
                Widget::mouseButtonEvent(p, button, down, modifiers);
                return true;
            }
        }

        void draw(NVGcontext* ctx) override {
            if (m_drawCallback) {
                m_drawCallback(this, ctx);
            } else {
                Window::draw(ctx);
            }
        }

        nanogui::Button* createOpenButton(Widget* a_parent, const string& text = "", int icon = 0, std::function<void()> a_callback = nullptr) {
            auto openButton = new nanogui::Button(a_parent, text, icon);
            openButton->setFixedSize({20,20});
            openButton->setCallback([this, a_callback]() {
                    setVisible(true);
                    screen()->moveWindowToFront(this);
                    if (a_callback)
                        a_callback();
                });
            return openButton;
        }

        void setDrawCallback(DrawFunc f) { m_drawCallback = f; }
        DrawFunc getDrawCallback() const { return m_drawCallback; }

        bool mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) override {
            Widget::mouseMotionEvent(p, rel, button, modifiers);
            return true;
        }
        ;
    private:
        DrawFunc m_drawCallback;
    };
}

synui::MainGUI::operator json() const {
    TRACE
    json j;
    j["circuit"] = m_circuitWidget->operator json();
    json& settings = j["settings"] = json();
    settings = m_settingsFormHelper->operator json();

    return j;
}

synui::MainGUI* synui::MainGUI::load(const json& j) {
    TRACE
    const json& settings = j.value("settings", json());
    if (!settings.empty()) {
        m_settingsFormHelper->load(settings);
    }
    m_circuitWidget->load(j["circuit"]);
    return this;
}

void synui::MainGUI::reset() {
    TRACE
    m_unitEditorHost->reset();
    m_circuitWidget->reset();
}

void synui::MainGUI::resize(int a_w, int a_h) {
    TRACE
    m_screen->resizeCallbackEvent(a_w, a_h);
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_sidePanelR->setPosition({m_sidePanelL->width(), m_buttonPanel->height()});
    m_sidePanelR->setFixedWidth(m_screen->width() - m_sidePanelR->absolutePosition().x());
    m_circuitWidget->setFixedHeight(m_screen->height() - m_sidePanelR->absolutePosition().y());
    m_circuitWidget->setFixedWidth(m_screen->width() - m_sidePanelR->absolutePosition().x());
    m_circuitWidget->resizeGrid(m_circuitWidget->gridSpacing());
    m_buttonPanel->setPosition({m_sidePanelL->width(), 0});
    m_buttonPanel->setFixedWidth(m_sidePanelR->width());
    m_screen->performLayout();
}

void synui::MainGUI::setGLFWWindow(GLFWwindow* a_window) {
    TRACE

    TRACEMSG("Initializing nanogui screen.")
    m_screen->initialize(a_window, true);
    TRACEMSG("Finished initializing nanogui screen.");

    /* Setup event handlers. */
    glfwSetWindowUserPointer(a_window, this);// Set user pointer so we can access ourselves inside the event handlers

    glfwSetCursorPosCallback(a_window,
        [](GLFWwindow* w, double x, double y) { static_cast<MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->cursorPosCallbackEvent(x, y); }
    );

    glfwSetMouseButtonCallback(a_window,
        [](GLFWwindow* w, int button, int action, int modifiers) { static_cast<MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->mouseButtonCallbackEvent(button, action, modifiers); }
    );

    glfwSetKeyCallback(a_window,
        [](GLFWwindow* w, int key, int scancode, int action, int mods) { static_cast<MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->keyCallbackEvent(key, scancode, action, mods); }
    );

    glfwSetCharCallback(a_window,
        [](GLFWwindow* w, unsigned int codepoint) { static_cast<MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->charCallbackEvent(codepoint); }
    );

    glfwSetDropCallback(a_window,
        [](GLFWwindow* w, int count, const char** filenames) { static_cast<MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->dropCallbackEvent(count, filenames); }
    );

    glfwSetScrollCallback(a_window,
        [](GLFWwindow* w, double x, double y) { static_cast<MainGUI*>(glfwGetWindowUserPointer(w))->m_screen->scrollCallbackEvent(x, y); }
    );

    auto resizeCallback = [](GLFWwindow* w, int width, int height) {
                auto mainGui = static_cast<MainGUI*>(glfwGetWindowUserPointer(w));
                mainGui->resize(width, height);
            };
    glfwSetFramebufferSizeCallback(a_window, resizeCallback);
    glfwSetWindowSizeCallback(a_window, resizeCallback);
}

void synui::MainGUI::createUnitSelector_(nanogui::Widget* a_widget) {
    TRACE
    nanogui::BoxLayout* layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1);
    a_widget->setLayout(layout);

    auto unitSelectorTitle = new nanogui::Widget(a_widget);
    unitSelectorTitle->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 2, 10));

    for (auto gname : m_uf->getGroupNames()) {
        nanogui::PopupButton* button = new nanogui::PopupButton(a_widget, gname);
        button->setFontSize(15);
        nanogui::Popup* popup = button->popup();
        popup->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
        button->setCallback([this, popup]() { m_screen->moveWindowToFront(popup); });
        for (auto pname : m_uf->getPrototypeNames(gname)) {
            nanogui::Button* subbtn = new nanogui::Button(popup, pname);
            syn::UnitTypeId classId = m_uf->getClassId(gname, pname);
            subbtn->setFontSize(14);
            // Close popup
            subbtn->setCallback([this, button, classId]() {
                    this->m_circuitWidget->loadPrototype(classId);
                    button->setPushed(false);
                    m_screen->updateFocus(nullptr);
                });
        }
        if(m_uf->getPrototypeNames(gname).size()==1)
            popup->setAnchorHeight(15);
    }
}

void synui::MainGUI::createSettingsEditor_(nanogui::Widget* a_widget, SerializableFormHelper* a_fh) {
    TRACE
    auto layout = new nanogui::AdvancedGridLayout({10, 0, 10, 0}, {});
    layout->setMargin(10);
    layout->setColStretch(2, 1);
    a_widget->setLayout(layout);
    SerializableFormHelper* helper = a_fh;
    helper->setWidget(a_widget);

    helper->addGroup("Plugin Settings");

    helper->addSerializableVariable<int>("grid_spacing", "Grid spacing", [this](const int& s) {
            m_circuitWidget->resizeGrid(s);
            m_screen->performLayout();
        }, [this]() {
            int gs = m_circuitWidget->gridSpacing();
            return gs;
        });

    helper->addSerializableVariable<int>("window_width", "Window width", [this](const int& w) { m_window->resize(w, m_screen->height()); }, [this]() { return m_screen->width(); });
    helper->addSerializableVariable<int>("window_height", "Window height", [this](const int& h) { m_window->resize(m_screen->width(), h); }, [this]() { return m_screen->height(); });
    helper->addVariable<bool>("Curved Wires",
        [this](const bool& s) {
            m_circuitWidget->setWireDrawStyle(static_cast<CircuitWidget::WireDrawStyle>(s));
        }, [this]() {
            return static_cast<bool>(m_circuitWidget->wireDrawStyle());
        });

    helper->addVariable<int>("Internal buffer size",
        [this, helper](const int& size) {
            auto f = [this, size, helper]() {
                        m_vm->setInternalBufferSize(size);
                            helper->refresh();
                    };
            m_vm->queueAction(syn::MakeCommand(f));
        }, [this]() {
            return m_vm->getInternalBufferSize();
        });

    helper->addSerializableVariable<int>("max_voices", "Max voices",
        [this, helper](const int& maxVoices) {
            auto f = [this, maxVoices, helper]() {
                        m_vm->setMaxVoices(maxVoices);
                            helper->refresh();
                    };
            m_vm->queueAction(syn::MakeCommand(f));
        }, [this]() {
            return m_vm->getMaxVoices();
        });

    helper->addSerializableVariable<bool>("legato", "Legato",
        [this, helper](const bool& legato) {
            auto f = [this, legato, helper]() {
                        m_vm->setLegato(legato);
                            helper->refresh();
                    };
            m_vm->queueAction(syn::MakeCommand(f));
        }, [this]() {
            return m_vm->getLegato();
        });

    helper->addVariable<syn::VoiceManager::VoiceStealingPolicy>("Voice Stealing",
        [this, helper](const syn::VoiceManager::VoiceStealingPolicy& policy) {
            auto f = [this, policy, helper]() {
                        m_vm->setVoiceStealingPolicy(policy);
                            helper->refresh();
                    };
            m_vm->queueAction(syn::MakeCommand(f));
        }, [this]() {
            return m_vm->getVoiceStealingPolicy();
        })->setItems({"Oldest", "Newest", "Highest", "Lowest"});

    
#define ADD_FH_VAR(label, type, lvalue) helper->addVariable<type>(label, [this](const type& val){ lvalue = val; }, [this](){ return lvalue; });
    /* Spacing-related parameters */
    helper->addGroup("Spacing");
    ADD_FH_VAR("Standard Font Size", int, m_screen->theme()->mStandardFontSize);
    ADD_FH_VAR("Button Font Size", int, m_screen->theme()->mButtonFontSize);
    ADD_FH_VAR("Text Box Font Size", int, m_screen->theme()->mTextBoxFontSize)
    ADD_FH_VAR("Window Corner Radius", int, m_screen->theme()->mWindowCornerRadius)
    ADD_FH_VAR("Window Drop Shadow Size", int, m_screen->theme()->mWindowDropShadowSize)
    ADD_FH_VAR("Button Corner Radius", int, m_screen->theme()->mButtonCornerRadius)

    /* Generic colors */
    helper->addGroup("Global Colors");
    ADD_FH_VAR("Border Dark", nanogui::Color, m_screen->theme()->mBorderDark)
    ADD_FH_VAR("Border Light", nanogui::Color, m_screen->theme()->mBorderLight)
    ADD_FH_VAR("Border Medium", nanogui::Color, m_screen->theme()->mBorderMedium)
    ADD_FH_VAR("Drop Shadow", nanogui::Color, m_screen->theme()->mDropShadow)
    ADD_FH_VAR("Icon Color", nanogui::Color, m_screen->theme()->mIconColor)
    ADD_FH_VAR("Transparent", nanogui::Color, m_screen->theme()->mTransparent)
    ADD_FH_VAR("Text Color", nanogui::Color, m_screen->theme()->mTextColor)
    ADD_FH_VAR("Text Color",  nanogui::Color, m_screen->theme()->mTextColorShadow)
    ADD_FH_VAR("Disabled Text Color", nanogui::Color, m_screen->theme()->mDisabledTextColor)

    /* Button colors */
    helper->addGroup("Button Colors");
    ADD_FH_VAR("Button Top (Unfocused)", nanogui::Color, m_screen->theme()->mButtonGradientTopUnfocused)
    ADD_FH_VAR("Button Bot (Unfocused)", nanogui::Color, m_screen->theme()->mButtonGradientBotUnfocused)
    ADD_FH_VAR("Button Top (Focused)", nanogui::Color, m_screen->theme()->mButtonGradientTopFocused)
    ADD_FH_VAR("Button Bot (Focused)", nanogui::Color, m_screen->theme()->mButtonGradientBotFocused)
    ADD_FH_VAR("Button Top (Pushed)", nanogui::Color, m_screen->theme()->mButtonGradientTopPushed)
    ADD_FH_VAR("Button Bot (Pushed)", nanogui::Color, m_screen->theme()->mButtonGradientBotPushed)

    /* Window colors */
    helper->addGroup("Window Colors");
    ADD_FH_VAR("Window Fill (Unfocused)", nanogui::Color, m_screen->theme()->mWindowFillUnfocused)
    ADD_FH_VAR("Window Title (Unfocused)", nanogui::Color, m_screen->theme()->mWindowTitleUnfocused)
    ADD_FH_VAR("Window Fill (Focused)", nanogui::Color, m_screen->theme()->mWindowFillFocused)
    ADD_FH_VAR("Window Title (Focused)", nanogui::Color, m_screen->theme()->mWindowTitleFocused)

    ADD_FH_VAR("Window Header Top", nanogui::Color, m_screen->theme()->mWindowHeaderGradientTop)
    ADD_FH_VAR("Window Header Bot", nanogui::Color, m_screen->theme()->mWindowHeaderGradientBot)
    ADD_FH_VAR("Window Header Sep Top", nanogui::Color, m_screen->theme()->mWindowHeaderSepTop)
    ADD_FH_VAR("Window Header Sep Bot", nanogui::Color, m_screen->theme()->mWindowHeaderSepBot)

    ADD_FH_VAR("Window Popup", nanogui::Color, m_screen->theme()->mWindowPopup)
    ADD_FH_VAR("Window Popup Transparent", nanogui::Color, m_screen->theme()->mWindowPopupTransparent)

#undef ADD_FH_VAR
}

void synui::MainGUI::createLogViewer_(nanogui::Widget* a_widget) {
    TRACE
    auto layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 5);
    a_widget->setLayout(layout);

    Logger::instance().addListener([a_widget]() {
            const auto& log = Logger::instance().getLogs().back();
            std::ostringstream value;
            value << "[ " << log.first << "] " << log.second;
            auto txt = new nanogui::Label(a_widget, value.str(), "sans", 12);
            a_widget->screen()->performLayout();
        });
}

void synui::MainGUI::createOscilloscopeViewer_(nanogui::Widget* a_widget) {
    TRACE
    auto layout = new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Fill, 0, 0);
    a_widget->setLayout(layout);

    OscilloscopeWidget* oscwidget = new OscilloscopeWidget(a_widget, m_vm);
}

synui::MainGUI::MainGUI(MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf)
    : m_window(a_window),
      m_screen(nullptr),
      m_vm(a_vm),
      m_uf(a_uf) {
    TRACE
    m_screen = new nanogui::Screen();
    setGLFWWindow(m_window->getWindow());

    m_screen->theme()->mWindowDropShadowSize = 2;

    /* Create left pane. */
    m_sidePanelL = new EnhancedWindow(m_screen, "");
    m_sidePanelL->setIsBackgroundWindow(true);
    m_sidePanelL->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 5, 0));
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_sidePanelL->setFixedWidth(200);
    m_sidePanelL->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx) {
            nanogui::Color fillColor = self->theme()->mWindowFillUnfocused.cwiseProduct(nanogui::Color(0.9f, 1.0f));

            nvgSave(ctx);

            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, 0, 0, self->width(), self->height(), 1.0f);
            nvgFillColor(ctx, fillColor);
            nvgFill(ctx);

            nvgRestore(ctx);

            self->Widget::draw(ctx);
        });

    /* Create tab widget in left pane. */
    m_tabWidget = new nanogui::TabWidget(m_sidePanelL);

    /* Create unit selector tab. */
    m_unitSelector = m_tabWidget->createTab("Build");
    createUnitSelector_(m_unitSelector);

    /* Create unit editor tab. */
    m_unitEditorHost = new UnitEditorHost(nullptr, m_vm);
    m_tabWidget->addTab("Editor", m_unitEditorHost);
    m_unitEditorHost->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 1));

    m_tabWidget->setActiveTab(0);

    /* Create right pane. */
    m_sidePanelR = new EnhancedWindow(m_screen, "");
    m_sidePanelR->setIsBackgroundWindow(true);
    m_sidePanelR->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 0));
    m_sidePanelR->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx) {
            nanogui::Color fillColor = self->theme()->mWindowFillUnfocused.cwiseProduct(nanogui::Color(0.9f, 1.0f));
            nanogui::Color shineColor = self->theme()->mBorderLight;
            nanogui::Color shadowColor = self->theme()->mBorderDark;

            nvgSave(ctx);

            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, 0, 0, self->width(), self->height(), 1.0f);
            nvgFillColor(ctx, fillColor);
            nvgFill(ctx);

            nvgRestore(ctx);

            self->Widget::draw(ctx);

            nvgSave(ctx);


            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, 1.25f, self->height());
            nvgLineTo(ctx, 1.25f, 1.5f);
            nvgLineTo(ctx, self->width(), 1.5f);
            nvgStrokeColor(ctx, shineColor);
            nvgStrokeWidth(ctx, 1.0f);
            nvgLineJoin(ctx, NVG_ROUND);
            nvgStroke(ctx);

            nvgBeginPath(ctx);
            nvgMoveTo(ctx, 0.75f, self->height());
            nvgLineTo(ctx, 0.75f, 0.5f);
            nvgLineTo(ctx, self->width(), 0.5f);
            nvgStrokeColor(ctx, shadowColor);
            nvgStrokeWidth(ctx, 1.0f);
            nvgLineJoin(ctx, NVG_ROUND);
            nvgStroke(ctx);

            nvgRestore(ctx);
        });

    /* Create circuit widget in right pane. */
    m_circuitWidget = new CircuitWidget(m_sidePanelR, m_window, m_unitEditorHost, a_vm, a_uf);

    /* Create button panel */
    m_buttonPanel = new EnhancedWindow(m_screen, "");
    m_buttonPanel->setIsBackgroundWindow(true);
    m_buttonPanel->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx) {
            nanogui::Color fillColor = self->theme()->mWindowFillUnfocused.cwiseProduct(nanogui::Color(0.9f, 1.0f));

            nvgSave(ctx);

            nvgTranslate(ctx, self->position().x(), self->position().y());
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, 0, 0, self->width(), self->height(), 1.0f);
            nvgFillColor(ctx, fillColor);
            nvgFill(ctx);

            nvgRestore(ctx);

            self->Widget::draw(ctx);
        });
    auto buttonPanelLayout = new nanogui::AdvancedGridLayout({}, {0}, 5);
    m_buttonPanel->setLayout(buttonPanelLayout);
    buttonPanelLayout->appendCol(0, 1.0);

    /* Create log viewer window. */
    m_logViewer = new EnhancedWindow(m_screen, "Logs");
    m_logViewer->setVisible(false);
    auto logScrollPanel = new nanogui::VScrollPanel(m_logViewer);
    m_logViewer->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    logScrollPanel->setFixedHeight(400);
    createLogViewer_(new nanogui::Widget(logScrollPanel));

    /* Add a button for openning the log viewer window. */
    buttonPanelLayout->appendCol(0, 0);

    auto log_callback = [this]() {
                m_screen->centerWindow(m_logViewer);
            };
    auto logviewer_button = m_logViewer->createOpenButton(m_buttonPanel, "", ENTYPO_TRAFFIC_CONE, log_callback);
    buttonPanelLayout->setAnchor(logviewer_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});
    
    /* Create oscilloscope viewer window. */
    m_oscViewer = new EnhancedWindow(m_screen, "Visualizers");
    m_oscViewer->setVisible(false);
    auto oscScrollPanel = new nanogui::VScrollPanel(m_oscViewer);
    m_oscViewer->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    createOscilloscopeViewer_(new nanogui::Widget(oscScrollPanel));

    /* Add a button for openning the log viewer window. */
    buttonPanelLayout->appendCol(0, 0);

    auto osc_viewer_callback = [this]() {
                m_screen->centerWindow(m_oscViewer);
            };
    auto osc_viewer_button = m_oscViewer->createOpenButton(m_buttonPanel, "", ENTYPO_AREA_GRAPH, osc_viewer_callback);
    buttonPanelLayout->setAnchor(osc_viewer_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});

    /* Create the settings editor window. */
    m_settingsEditor = new EnhancedWindow(m_screen, "Settings");
    m_settingsEditor->setVisible(false);
    auto settingsScrollPanel = new nanogui::VScrollPanel(m_settingsEditor);
    settingsScrollPanel->setFixedHeight(400);
    m_settingsEditor->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    m_settingsFormHelper = std::make_shared<SerializableFormHelper>(m_screen);
    createSettingsEditor_(new nanogui::Widget(settingsScrollPanel), m_settingsFormHelper.get());

    /* Add a button for openning the settings editor window. */
    buttonPanelLayout->appendCol(0, 0);
    auto settings_callback = [this]() {
                m_settingsFormHelper->refresh();
                m_screen->centerWindow(m_settingsEditor);
            };
    auto settings_button = m_settingsEditor->createOpenButton(m_buttonPanel, "", ENTYPO_COG, settings_callback);
    buttonPanelLayout->setAnchor(settings_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});

    m_screen->performLayout();
}

void synui::MainGUI::show() {
    TRACE
    m_screen->setVisible(true);
    m_screen->performLayout();
}

void synui::MainGUI::hide() {
    TRACE
    m_screen->setVisible(false);
}

void synui::MainGUI::draw() {
    m_screen->drawAll();
}

synui::MainGUI::~MainGUI() {
    TRACE
    Logger::instance().reset();
}
