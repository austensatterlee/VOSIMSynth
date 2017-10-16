#include "vosimsynth/MainGUI.h"
#include "vosimsynth/MainWindow.h"
#include "vosimsynth/widgets/UnitWidget.h"
#include "vosimsynth/widgets/CircuitWidget.h"
#include "vosimsynth/widgets/UnitEditor.h"
#include "vosimsynth/widgets/OscilloscopeWidget.h"
#include "vosimsynth/Logging.h"
#include "vosimsynth/VOSIMTheme.h"
#include <vosimlib/VoiceManager.h>
#include <vosimlib/Command.h>
#include <vosimlib/UnitFactory.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/tabwidget.h>
#include <fstream>

using nanogui::Color;

namespace synui {
    class EnhancedWindow : public nanogui::Window {
    public:
        typedef std::function<void(EnhancedWindow*, NVGcontext*)> DrawFunc;

        EnhancedWindow(Widget* a_parent, const string& a_title)
            : Window(a_parent, a_title) {
            if (!a_title.empty()) {
                // Close button
                auto settings_close_button = new nanogui::Button(buttonPanel(), "", ENTYPO_ICON_CROSS);
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
            openButton->setFixedSize({ 20,20 });
            openButton->setCallback([this, a_callback]() {
                setVisible(!visible());
                screen()->moveWindowToFront(this);
                if (a_callback)
                    a_callback();
            });
            return openButton;
        }

        void setDrawCallback(DrawFunc f) { m_drawCallback = f; }
        DrawFunc getDrawCallback() const { return m_drawCallback; }
    private:
        DrawFunc m_drawCallback;
    };
}

synui::MainGUI::operator json() const {
    TIME_TRACE
    json j;
    j["circuit"] = m_circuitWidget->operator json();
    j["settings"] = m_settingsFormHelper->operator json();
    j["theme"] = m_screen->theme()->operator json();
    return j;
}

synui::MainGUI* synui::MainGUI::load(const json& j) {
    TIME_TRACE
    // Load theme
    if(j.find("theme")!=j.end())
        m_screen->theme()->update(j.at("theme"));
    // Load settings
    if (j.find("settings") != j.end())
        m_settingsFormHelper->load(j.at("settings"));
    // Load circuit
    m_circuitWidget->load(j.at("circuit"));
    return this;
}

void synui::MainGUI::reset() {
    TIME_TRACE
    m_unitEditorHost->reset();
    m_circuitWidget->reset();
}

void synui::MainGUI::resize(int a_w, int a_h) {
    TIME_TRACE
    m_screen->resizeCallbackEvent(a_w, a_h);
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_screen->performLayout();
    m_sidePanelR->setPosition({m_sidePanelL->width(), m_buttonPanel->height()});
    m_sidePanelR->setFixedWidth(m_screen->width() - m_sidePanelR->absolutePosition().x());
    m_sidePanelR->setFixedHeight(m_screen->height() - m_sidePanelR->absolutePosition().y());
    m_screen->performLayout();
    m_circuitWidget->setFixedHeight(m_sidePanelR->height() - m_circuitWidget->position().y());
    m_circuitWidget->resizeGrid(m_circuitWidget->gridSpacing());
    m_screen->performLayout();
    m_buttonPanel->setPosition({m_sidePanelL->width(), 0});
    m_buttonPanel->setFixedWidth(m_sidePanelR->width());
    m_screen->performLayout();
}

void synui::MainGUI::setGLFWWindow(GLFWwindow* a_window) {
    TIME_TRACE
        
    TRACEMSG("Initializing nanogui screen.");
    m_screen->initialize(a_window, false);
    m_screen->setTheme(new VOSIMTheme(m_screen->nvgContext()));
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
    TIME_TRACE
    nanogui::BoxLayout* layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 3, 3);
    a_widget->setLayout(layout);

    auto unitSelectorTitle = new nanogui::Widget(a_widget);
    unitSelectorTitle->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 2, 10));

    const syn::UnitFactory& uf = syn::UnitFactory::instance();
    for (const auto& gname : uf.getGroupNames()) {
        nanogui::PopupButton* button = new nanogui::PopupButton(a_widget, gname);
        button->setFontSize(button->theme()->prop("/button/text-size"));
        nanogui::Popup* popup = button->popup();
        popup->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 2, 2));
        button->setCallback([this, popup]()
        {
            m_screen->moveWindowToFront(popup);
        });
        for (const auto& pname : uf.getPrototypeNames(gname)) {
            nanogui::Button* subbtn = new nanogui::Button(popup, pname);
            syn::UnitTypeId classId = uf.getClassId(gname, pname);
            subbtn->setFontSize(subbtn->theme()->get<int>("/button/text-size")-2);
            // Close popup and place selected unit
            subbtn->setCallback([this, button, classId]() {
                this->m_circuitWidget->createUnit(classId);
                button->setPushed(false);
                m_screen->updateFocus(nullptr);
            });
        }
    }
}

void synui::MainGUI::createSettingsEditor_(nanogui::Widget* a_widget) {
    TIME_TRACE
    auto layout = new nanogui::AdvancedGridLayout({ 10, 0, 10, 0 }, {});
    layout->setMargin(10);
    layout->setColStretch(2, 1);
    a_widget->setLayout(layout);
    SerializableFormHelper* helper = m_settingsFormHelper.get();
    helper->setWidget(a_widget);

    helper->addGroup("Plugin Settings");

    helper->addSerializableVariable<int>("max_voices", "Max voices", [this, helper](const int& maxVoices) {
        auto f = [this, maxVoices, helper]() {
            m_vm->setMaxVoices(maxVoices);
            helper->refresh();
        };
        m_vm->queueAction(syn::MakeCommand(f));
    }, [this]() {
        return m_vm->getMaxVoices();
    });

    helper->addSerializableVariable<int>("grid_spacing", "Grid spacing", [this](const int& s) {
        m_circuitWidget->resizeGrid(s);
        m_circuitWidget->performLayout(m_screen->nvgContext());
    }, [this]() {
        int gs = m_circuitWidget->gridSpacing();
        return gs;
    });

    helper->addSerializableVariable<int>("window_width", "Window width", [this](const int& w) { m_window->resize(w, m_screen->height()); }, [this]() { return m_screen->width(); });
    helper->addSerializableVariable<int>("window_height", "Window height", [this](const int& h) { m_window->resize(m_screen->width(), h); }, [this]() { return m_screen->height(); });

    helper->addVariable<bool>("Curved Wires", [this](const bool& s) {
        m_circuitWidget->setWireDrawStyle(static_cast<CircuitWidget::WireDrawStyle>(s));
    }, [this]() {
        return static_cast<bool>(m_circuitWidget->wireDrawStyle());
    });

    helper->addVariable<int>("Internal buffer size", [this, helper](const int& size) {
        auto f = [this, size, helper]() {
            m_vm->setInternalBufferSize(size);
            helper->refresh();
        };
        m_vm->queueAction(syn::MakeCommand(f));
    }, [this]() {
        return m_vm->getInternalBufferSize();
    });

    helper->addSerializableVariable<bool>("legato", "Legato", [this, helper](const bool& legato) {
        auto f = [this, legato, helper]() {
            m_vm->setLegato(legato);
            helper->refresh();
        };
        m_vm->queueAction(syn::MakeCommand(f));
    }, [this]() {
        return m_vm->getLegato();
    });

    using VoiceStealPolicy = syn::VoiceManager::VoiceStealPolicy;
    helper->addVariable<VoiceStealPolicy>("Voice Stealing", [this, helper](const VoiceStealPolicy& policy) {
        auto f = [this, policy, helper]() {
            m_vm->setVoiceStealPolicy(policy);
            helper->refresh();
        };
        m_vm->queueAction(syn::MakeCommand(f));
    }, [this]() {
        return m_vm->getVoiceStealPolicy();
    })->setItems({ "Oldest", "Newest", "Highest", "Lowest" });
}

void synui::MainGUI::createThemeEditor_(nanogui::Widget* a_widget) {
    TIME_TRACE
    auto layout = new nanogui::AdvancedGridLayout({ 10, 0, 10, 0 }, {});
    layout->setMargin(10);
    layout->setColStretch(2, 1);
    a_widget->setLayout(layout);
    SerializableFormHelper* helper = m_themeFormHelper.get();

    helper->setWidget(a_widget);

#define ADD_FH_VAR(label, type, lvalue) helper->addVariable<type>(label, [this](const type& val){ lvalue = val; }, [this](){ return lvalue; })
    helper->addGroup("CircuitWidget");
    ADD_FH_VAR("CircuitWidget/bg-color", Color, m_screen->theme()->prop("/CircuitWidget/bg-color"));

    helper->addGroup("Sum");
    ADD_FH_VAR("Sum/bg-color", Color, m_screen->theme()->prop("/SummerUnitWidget/bg-color"));
    ADD_FH_VAR("Sum/fg-color", Color, m_screen->theme()->prop("/SummerUnitWidget/fg-color"));
    helper->addGroup("Gain");
    ADD_FH_VAR("Gain/bg-color", Color, m_screen->theme()->prop("/GainUnitWidget/bg-color"));
    ADD_FH_VAR("Gain/fg-color", Color, m_screen->theme()->prop("/GainUnitWidget/fg-color"));

    helper->addGroup("DefaultUnitWidget");
    ADD_FH_VAR("DefaultUnitWidget/bg-color", Color, m_screen->theme()->prop("/DefaultUnitWidget/bg-color"));
    ADD_FH_VAR("DefaultUnitWidget/title/bg-color", Color, m_screen->theme()->prop("/DefaultUnitWidget/title/bg-color"));
    ADD_FH_VAR("DefaultUnitWidget/output/bg-color", Color, m_screen->theme()->prop("/DefaultUnitWidget/output/bg-color"));
    ADD_FH_VAR("DefaultUnitWidget/input/bg-color", Color, m_screen->theme()->prop("/DefaultUnitWidget/input/bg-color"));
    ADD_FH_VAR("DefaultUnitWidget/focused/shadow-size", float, m_screen->theme()->prop("/DefaultUnitWidget/focused/shadow-size"));
    ADD_FH_VAR("DefaultUnitWidget/focused/shadow-feather", float, m_screen->theme()->prop("/DefaultUnitWidget/focused/shadow-feather"));
    ADD_FH_VAR("DefaultUnitWidget/focused/shadow-color", Color, m_screen->theme()->prop("/DefaultUnitWidget/focused/shadow-color"));
    ADD_FH_VAR("DefaultUnitWidget/hovered/shadow-size", float, m_screen->theme()->prop("/DefaultUnitWidget/hovered/shadow-size"));
    ADD_FH_VAR("DefaultUnitWidget/hovered/shadow-feather", float, m_screen->theme()->prop("/DefaultUnitWidget/hovered/shadow-feather"));
    ADD_FH_VAR("DefaultUnitWidget/hovered/shadow-color", Color, m_screen->theme()->prop("/DefaultUnitWidget/hovered/shadow-color"));

    helper->addGroup("OscilloscopeWidget"); 
    ADD_FH_VAR("OscilloscopeWidget/bg-color", Color, m_screen->theme()->prop("/OscilloscopeWidget/bg-color"));
    ADD_FH_VAR("OscilloscopeWidget/fg-color", Color, m_screen->theme()->prop("/OscilloscopeWidget/fg-color"));
    ADD_FH_VAR("OscilloscopeWidget/text-color", Color, m_screen->theme()->prop("/OscilloscopeWidget/text-color"));
    ADD_FH_VAR("OscilloscopeWidget/tick-color", Color, m_screen->theme()->prop("/OscilloscopeWidget/tick-color"));
    ADD_FH_VAR("OscilloscopeWidget/tick-label-color", Color, m_screen->theme()->prop("/OscilloscopeWidget/tick-label-color"));

    helper->addGroup("ContextMenu");
    ADD_FH_VAR("ContextMenu/text-size", int, m_screen->theme()->prop("/ContextMenu/text-size"));
    ADD_FH_VAR("ContextMenu/bg-color", Color, m_screen->theme()->prop("/ContextMenu/bg-color"));
    ADD_FH_VAR("ContextMenu/margin-color", Color, m_screen->theme()->prop("/ContextMenu/margin-color"));
    ADD_FH_VAR("ContextMenu/hover-color", Color, m_screen->theme()->prop("/ContextMenu/hover-color"));

    /* Spacing-related parameters */
    helper->addGroup("Spacing");
    ADD_FH_VAR("Standard Font Size", int, m_screen->theme()->prop("/text-size"));
    ADD_FH_VAR("Button Font Size", int, m_screen->theme()->prop("/button/text-size"));
    ADD_FH_VAR("Text Box Font Size", int, m_screen->theme()->prop("/textbox/text-size"));
    ADD_FH_VAR("Window Corner Radius", int, m_screen->theme()->prop("/window/corner-radius"));
    ADD_FH_VAR("Window Drop Shadow Size", int, m_screen->theme()->prop("/window/shadow-size"));
    ADD_FH_VAR("Button Corner Radius", int, m_screen->theme()->prop("/button/corner-radius"));

    /* Generic colors */
    helper->addGroup("Global Colors");
    ADD_FH_VAR("Border Dark", Color, m_screen->theme()->prop("/border/dark"));
    ADD_FH_VAR("Border Light", Color, m_screen->theme()->prop("/border/light"));
    ADD_FH_VAR("Border Medium", Color, m_screen->theme()->prop("/border/medium"));
    ADD_FH_VAR("Drop Shadow", Color, m_screen->theme()->prop("/shadow"));
    ADD_FH_VAR("Icon Color", Color, m_screen->theme()->prop("/icon-color"));
    ADD_FH_VAR("Text Color", Color, m_screen->theme()->prop("/text-color"));
    ADD_FH_VAR("Disabled Text Color", Color, m_screen->theme()->prop("/disabled-text-color"));

    /* Button colors */
    helper->addGroup("Button Colors");
    ADD_FH_VAR("Button Top (Unfocused)", Color, m_screen->theme()->prop("/button/unfocused/grad-top"));
    ADD_FH_VAR("Button Bot (Unfocused)", Color, m_screen->theme()->prop("/button/unfocused/grad-bot"));
    ADD_FH_VAR("Button Top (Focused)", Color, m_screen->theme()->prop("/button/focused/grad-top"));
    ADD_FH_VAR("Button Bot (Focused)", Color, m_screen->theme()->prop("/button/focused/grad-bot"));
    ADD_FH_VAR("Button Top (Pushed)", Color, m_screen->theme()->prop("/button/pushed/grad-top"));
    ADD_FH_VAR("Button Bot (Pushed)", Color, m_screen->theme()->prop("/button/pushed/grad-bot"));

    /* Window colors */
    helper->addGroup("Window Colors");
    ADD_FH_VAR("Window Fill (Unfocused)", Color, m_screen->theme()->prop("/window/unfocused/fill"));
    ADD_FH_VAR("Window Title (Unfocused)", Color, m_screen->theme()->prop("/window/unfocused/title"));
    ADD_FH_VAR("Window Fill (Focused)", Color, m_screen->theme()->prop("/window/focused/fill"));
    ADD_FH_VAR("Window Title (Focused)", Color, m_screen->theme()->prop("/window/focused/title"));

    ADD_FH_VAR("Window Header Top", Color, m_screen->theme()->prop("/window/header/grad-top"));
    ADD_FH_VAR("Window Header Bot", Color, m_screen->theme()->prop("/window/header/grad-bot"));
    ADD_FH_VAR("Window Header Sep Top", Color, m_screen->theme()->prop("/window/header/sep-top"));
    ADD_FH_VAR("Window Header Sep Bot", Color, m_screen->theme()->prop("/window/header/sep-bot"));

    ADD_FH_VAR("Window Popup", Color, m_screen->theme()->prop("/popup/fill"));

    helper->addButton("Save Theme", [this]() {
        std::string filepath = nanogui::file_dialog({ { "json", "VOSIMSynth theme" } }, true);
        if (filepath.empty())
            return;
        json theme = m_screen->theme()->operator json();
        std::ofstream outfile;
        outfile.open(filepath, std::ios::trunc);
        if (outfile.is_open()) {
            outfile << std::setw(4) << theme;
        } else {
            std::ostringstream oss;
            oss << "Unable to open \"" << filepath << "\" for writing.";
            alert("Error", oss.str(), nanogui::MessageDialog::Type::Warning);
        }
    });

    helper->addButton("Load Theme", [this, helper]() {
        std::string filepath = nanogui::file_dialog({ { "json", "VOSIMSynth theme" } }, false);
        if (filepath.empty())
            return;
        std::ifstream infile;
        infile.open(filepath);
        if (infile.is_open()) {
            try {
                json newtheme;
                infile >> newtheme;
                m_screen->theme()->update(newtheme);
                m_screen->performLayout();
                helper->refresh();
            } catch (const std::exception& e) {
                std::ostringstream alertmsg;
                alertmsg << "Unable to load preset!" << std::endl;
                alertmsg << e.what();
                alert("Error", alertmsg.str(), nanogui::MessageDialog::Type::Warning);
            }
        } else {
            std::ostringstream oss;
            oss << "Unable to open \"" << filepath << "\" for reading.";
            alert("Error", oss.str(), nanogui::MessageDialog::Type::Warning);
        }
    });
#undef ADD_FH_VAR
}

void synui::MainGUI::createOscilloscopeViewer_(nanogui::Widget* a_widget) {
    TIME_TRACE
    auto oscPanel = a_widget->add<nanogui::Widget>();
    oscPanel->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 2, 3));
    // Add message that displays when no scopes exist
    auto lblPanel = oscPanel->add<nanogui::Widget>();
    lblPanel->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle));
    lblPanel->setFixedHeight(200);
    lblPanel->setVisibleIf([oscPanel]() { return oscPanel->children().size() == 1; });
    auto lbl = lblPanel->add<nanogui::Label>("No oscilloscopes found.\nBuild one in the circuit to see it here.", "sans-bold", 36);
    lbl->setFixedWidth(400);
    lbl->setHorizAlign(nanogui::Label::HAlign::Center);


    const std::function<void(UnitWidget*)> addScope = [this, oscPanel](UnitWidget* w)
    {
        int unitId = w->getUnitId();
        const syn::Circuit& circuit = m_vm->getPrototypeCircuit();
        const syn::Unit& unit = circuit.getUnit(unitId);
        if (unit.getClassIdentifier() == OscilloscopeUnit::classIdentifier()) {
            oscPanel->add<OscilloscopeWidget>(m_vm, unitId);
        }        
        m_screen->performLayout();
    };

    const std::function<void(UnitWidget*)> removeScope = [this, oscPanel](UnitWidget* w) {
        int unitId = w->getUnitId();
        const syn::Circuit& circuit = m_vm->getPrototypeCircuit();
        const syn::Unit& unit = circuit.getUnit(unitId);
        if (unit.getClassIdentifier() == OscilloscopeUnit::classIdentifier()) {
            for (const nanogui::Widget* child : oscPanel->children()) {
                int wId = static_cast<const OscilloscopeWidget*>(child)->getUnitId();
                if (wId == unitId) {
                    oscPanel->removeChild(child);
                    break;
                }
            }
        }
        m_screen->performLayout();
    };
    m_circuitWidget->onAddUnit.connect(addScope);
    m_circuitWidget->onRemoveUnit.connect(removeScope);
}

synui::MainGUI::MainGUI(MainWindow* a_window, syn::VoiceManager* a_vm)
    : m_window(a_window),
      m_screen(new nanogui::Screen()),
      m_vm(a_vm) 
{
    TIME_TRACE
    setGLFWWindow(m_window->getWindow());

    /* Create left pane. */
    m_sidePanelL = new EnhancedWindow(m_screen, "");
    m_sidePanelL->setIsBackgroundWindow(true);
    m_sidePanelL->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 5, 0));
    m_sidePanelL->setFixedHeight(m_screen->height());
    m_sidePanelL->setFixedWidth(200);
    m_sidePanelL->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx) {
        Color fillColor = self->theme()->get<Color>("/window/unfocused/fill").cwiseProduct(Color(0.9f, 1.0f));

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
        Color fillColor = self->theme()->get<Color>("/window/unfocused/fill").cwiseProduct(Color(0.9f, 1.0f));
        Color shineColor = self->theme()->prop("/border/light");
        Color shadowColor = self->theme()->prop("/border/dark");

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
    m_circuitWidget = new CircuitWidget(m_sidePanelR, m_window, m_unitEditorHost, a_vm);

    /* Create button panel */
    m_buttonPanel = new EnhancedWindow(m_screen, "");
    m_buttonPanel->setIsBackgroundWindow(true);
    m_buttonPanel->setDrawCallback([](EnhancedWindow* self, NVGcontext* ctx) {
        Color fillColor = self->theme()->get<Color>("/window/unfocused/fill").cwiseProduct(Color(0.9f, 1.0f));

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

    /* Create oscilloscope viewer window. */
    auto oscViewer = new EnhancedWindow(m_screen, "Visualizers");
    oscViewer->setVisible(false);
    oscViewer->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    auto oscScrollPanel = oscViewer->add<nanogui::VScrollPanel>();
    oscScrollPanel->setMaxHeight(450);
    createOscilloscopeViewer_(oscScrollPanel);
    /* Add a button for opening the oscilloscope viewer window. */
    buttonPanelLayout->appendCol(0, 0);
    auto osc_viewer_callback = [this, oscViewer]() {
        m_screen->centerWindow(oscViewer);
    };
    auto osc_viewer_button = oscViewer->createOpenButton(m_buttonPanel, "", ENTYPO_ICON_LINE_GRAPH, osc_viewer_callback);
    buttonPanelLayout->setAnchor(osc_viewer_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});

    /* Create the settings editor window. */
    auto settingsEditor = new EnhancedWindow(m_screen, "Settings");
    settingsEditor->setVisible(false);
    auto settingsScrollPanel = new nanogui::VScrollPanel(settingsEditor);
    settingsScrollPanel->setMaxHeight(400);
    settingsEditor->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    m_settingsFormHelper = std::make_shared<SerializableFormHelper>(m_screen);
    createSettingsEditor_(new nanogui::Widget(settingsScrollPanel));
    /* Add a button for opening the settings editor window. */
    buttonPanelLayout->appendCol(0, 0);
    auto settings_callback = [this, settingsEditor]() {
        m_settingsFormHelper->refresh();
        m_screen->centerWindow(settingsEditor);
    };
    auto settings_button = settingsEditor->createOpenButton(m_buttonPanel, "", ENTYPO_ICON_COG, settings_callback);
    buttonPanelLayout->setAnchor(settings_button, nanogui::AdvancedGridLayout::Anchor{buttonPanelLayout->colCount() - 1,0});

    /* Create the theme editor window. */
    auto themeEditor = new EnhancedWindow(m_screen, "Theme");
    themeEditor->setVisible(false);
    auto themeScrollPanel = new nanogui::VScrollPanel(themeEditor);
    themeScrollPanel->setMaxHeight(400);
    themeEditor->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));
    m_themeFormHelper = std::make_shared<SerializableFormHelper>(m_screen);
    createThemeEditor_(new nanogui::Widget(themeScrollPanel));
    /* Add a button for openning the settings editor window. */
    buttonPanelLayout->appendCol(0, 0);
    auto theme_callback = [this, themeEditor]() {
        m_themeFormHelper->refresh();
        m_screen->centerWindow(themeEditor);
    };
    auto theme_button = themeEditor->createOpenButton(m_buttonPanel, "", ENTYPO_ICON_PALETTE, theme_callback);
    buttonPanelLayout->setAnchor(theme_button, nanogui::AdvancedGridLayout::Anchor{ buttonPanelLayout->colCount() - 1,0 });

    m_screen->performLayout();
}

synui::MainGUI::~MainGUI() {
    TIME_TRACE
}

void synui::MainGUI::show() {
    TIME_TRACE
    m_screen->setVisible(true);
    m_screen->performLayout();
}

void synui::MainGUI::hide() {
    TIME_TRACE
    m_screen->setVisible(false);
}

void synui::MainGUI::draw() {
    m_screen->drawAll();
}

void synui::MainGUI::alert(const std::string& a_title, const std::string& a_msg, nanogui::MessageDialog::Type a_type) {
    new nanogui::MessageDialog(m_screen, a_type, a_title, a_msg);
}
