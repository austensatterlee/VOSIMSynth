/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *  \file UIColorWheel.h
 *  \brief Mostly influenced by nanogui
 *  \details 
 *  \author Austen Satterlee
 *  \date 06/2016
 */

#ifndef __UICOLORWHEEL__
#define __UICOLORWHEEL__

#include "UIComponent.h"
#include "UI.h"

namespace syn {

	class UIColorWheel : public UIComponent {
	public:
		UIColorWheel(MainWindow *a_window, const Color& color = { 1.f, 0.f, 0.f, 1.f });

		/// Set the change callback
		std::function<void(const Color &)> callback() const { return mCallback; }
		void setCallback(const std::function<void(const Color &)> &callback) { mCallback = callback; }

		/// Get the current color
		Color color() const;
		/// Set the current color
		void setColor(const Color& color);

		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseUp(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;

	protected:
		void draw(NVGcontext *ctx) override;
	private:
		enum Region {
			None = 0,
			InnerTriangle = 1,
			OuterCircle = 2,
			Both = 3
		};

		static Color hue2rgb(float h);
		Region adjustPosition(const UICoord &a_relCoord, Region consideredRegions = Both);
	
	protected:
		float mHue;
		float mWhite;
		float mBlack;
		Region mDragRegion;
		std::function<void(const Color &)> mCallback;
	};
}

#endif
