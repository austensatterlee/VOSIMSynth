#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <thread>
#include <NDPoint.h>

#include "nanovg.h"
#include <UIComponent.h>

using sf::Event;

namespace syn
{
	class VOSIMWindow
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		VOSIMWindow(int a_width, int a_height) :
			m_size(a_width,a_height),
			m_cursor({0,0}),
			m_isClicked(false), 
			m_running(false), 
			m_draggingComponent(nullptr), 
			m_focused(nullptr), 
			m_root(nullptr), 
			m_sfmlWindow(nullptr),
			m_frameCount(0) 
		{	
		}

		virtual ~VOSIMWindow() {
			if (m_sfmlWindow) {
				m_sfmlWindow->close();
				delete m_sfmlWindow;
			}
			if(m_root) {
				delete m_root;
			}
		}

		sf::RenderWindow* GetWindow() const {
			return m_sfmlWindow;
		}

		NDPoint<2, double> size() const {
			return m_size;
		}

		void setFocus(UIComponent* a_comp);
		Vector2i cursorPos() const { return m_cursor; }
		Vector2i diffCursorPos() const {return m_dCursor;}
		const Timestamped<Event::MouseButtonEvent>& lastClickEvent() const { return m_lastClick; }
		const Timestamped<Event::MouseWheelScrollEvent>& lastScrollEvent() const { return m_lastScroll; }

		bool OpenWindow(sf::WindowHandle a_system_window);
		void CloseWindow();
	private:
		/**
		* Creates a child window from the given system window handle.
		* \returns the system window handle for the child window.
		*/
		sf::WindowHandle sys_CreateChildWindow(sf::WindowHandle a_system_window);
		void drawThreadFunc();
		void updateCursorPos(const Vector2i& newPos) {
			m_dCursor = newPos - m_cursor;
			m_cursor = newPos;
		}
	private:
		Vector2i m_size;
		Vector2i m_cursor;
		Vector2i m_dCursor;
		Timestamped<Event::MouseButtonEvent> m_lastClick;
		Timestamped<Event::MouseWheelScrollEvent> m_lastScroll;
	
		bool m_isClicked;
		bool m_running;
		UIComponent* m_draggingComponent;
		UIComponent* m_focused;
		UIComponent* m_root;

		sf::RenderWindow* m_sfmlWindow;
		std::thread m_drawThread;
		unsigned m_frameCount;
	};

	class UIWindow : public UIComponent
	{
	public:

		UIWindow(VOSIMWindow* a_window, UIComponent* a_parent, const string& a_title="Untitled")
			: UIComponent{a_window, a_parent},
			  m_title{a_title} {}

		~UIWindow() override
		{};

		bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override {
			setRelPos(a_relCursor);
			return true;
		}
	protected:
		void draw(NVGcontext* a_nvg) override {
			int ds = m_theme->mWindowDropShadowSize, cr = m_theme->mWindowCornerRadius;
			int hh = m_theme->mWindowHeaderHeight;

			/* Draw window */
			nvgSave(a_nvg);
			nvgBeginPath(a_nvg);
			nvgRoundedRect(a_nvg, m_pos[0], m_pos[1], m_size[0], m_size[1], cr);

			nvgFillColor(a_nvg, m_focused ? m_theme->mWindowFillFocused : m_theme->mWindowFillUnfocused);

			nvgFill(a_nvg);

			/* Draw a drop shadow */
			NVGpaint shadowPaint = nvgBoxGradient(a_nvg, m_pos[0], m_pos[1], m_size[0], m_size[1], cr * 2, ds * 2,
				nvgRGBA(0,0,0,255), nvgRGBA(255,255,255,0));

			nvgBeginPath(a_nvg);
			nvgRect(a_nvg, m_pos[0] - ds, m_pos[1] - ds, m_size[0] + 2 * ds, m_size[1] + 2 * ds);
			nvgRoundedRect(a_nvg, m_pos[0], m_pos[1], m_size[0], m_size[1], cr);
			nvgPathWinding(a_nvg, NVG_HOLE);
			nvgFillPaint(a_nvg, shadowPaint);
			nvgFill(a_nvg);

			nvgFontFace(a_nvg, "sans");

			if (!m_title.empty()) {
				/* Draw header */
				NVGpaint headerPaint = nvgLinearGradient(
					a_nvg, m_pos.x(), m_pos.y(), m_pos.x(),
					m_pos.y() + hh,
					m_theme->mWindowHeaderGradientTop,
					m_theme->mWindowHeaderGradientBot);

				nvgBeginPath(a_nvg);
				nvgRoundedRect(a_nvg, m_pos.x(), m_pos.y(), m_size.x(), hh, cr);

				nvgFillPaint(a_nvg, headerPaint);
				nvgFill(a_nvg);

				nvgBeginPath(a_nvg);
				nvgRoundedRect(a_nvg, m_pos.x(), m_pos.y(), m_size.x(), hh, cr);
				nvgStrokeColor(a_nvg, m_theme->mWindowHeaderSepTop);
				nvgScissor(a_nvg, m_pos.x(), m_pos.y(), m_size.x(), 0.5f);
				nvgStroke(a_nvg);
				nvgResetScissor(a_nvg);

				nvgBeginPath(a_nvg);
				nvgMoveTo(a_nvg, m_pos.x() + 0.5f, m_pos.y() + hh - 1.5f);
				nvgLineTo(a_nvg, m_pos.x() + m_size.x() - 0.5f, m_pos.y() + hh - 1.5);
				nvgStrokeColor(a_nvg, m_theme->mWindowHeaderSepBot);
				nvgStroke(a_nvg);

				nvgFontSize(a_nvg, 18.0f);
				nvgFontFace(a_nvg, "sans-bold");
				nvgTextAlign(a_nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

				nvgFontBlur(a_nvg, 2);
				nvgFillColor(a_nvg, m_theme->mDropShadow);
				nvgText(a_nvg, m_pos.x() + m_size.x() / 2,
					m_pos.y() + hh / 2, m_title.c_str(), nullptr);

				nvgFontBlur(a_nvg, 0);
				nvgFillColor(a_nvg, m_focused ? m_theme->mWindowTitleFocused
					: m_theme->mWindowTitleUnfocused);
				nvgText(a_nvg, m_pos.x() + m_size.x() / 2, m_pos.y() + hh / 2 - 1,
					m_title.c_str(), nullptr);
			}

			nvgRestore(a_nvg);
		}
	protected:
		string m_title;
	};
}
#endif

