/*
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
along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2016, Austen Satterlee
*/

/**
*  \file UIButton.h
*  \brief
*  \details Mainly taken from NanoGUI source: https://github.com/wjakob/nanogui
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIBUTTON__
#define __UIBUTTON__
#include "UIComponent.h"

namespace synui
{
	class UIButton : public UIComponent
	{
	public:
		/// Flags to specify the button behavior (can be combined with binary OR)
		enum Flags
		{
			NormalButton = 1,
			RadioButton = 2,
			ToggleButton = 4,
			PopupButton = 8
		};

		enum class IconPosition
		{
			Left,
			LeftCentered,
			RightCentered,
			Right
		};

		UIButton(MainWindow* a_window, const string& caption = "Untitled", int icon = 0);

		const string& caption() const {
			return mCaption;
		}

		void setCaption(const string& caption) {
			mCaption = caption;
			_updateMinSize();
		}

		const Color& backgroundColor() const {
			return mBackgroundColor;
		}

		void setBackgroundColor(const Color& backgroundColor) {
			mBackgroundColor = backgroundColor;
		}

		const Color& textColor() const {
			return mTextColor;
		}

		void setTextColor(const Color& textColor) {
			mTextColor = textColor;
		}

		int icon() const {
			return mIcon;
		}

		void setIcon(int icon) {
			mIcon = icon;
			_updateMinSize();
		}

		int flags() const {
			return mFlags;
		}

		void setFlags(int buttonFlags) {
			mFlags = buttonFlags;
		}

		IconPosition iconPosition() const {
			return mIconPosition;
		}

		void setIconPosition(IconPosition iconPosition) {
			mIconPosition = iconPosition;
		}

		bool pushed() const {
			return mPushed;
		}

		void setPushed(bool pushed) {
			mPushed = pushed;
		}

		/// Set the push callback (for any type of button)
		function<void()> callback() const {
			return mCallback;
		}

		void setCallback(const function<void()>& callback) {
			mCallback = callback;
		}

		/// Set the change callback (for toggle buttons)
		function<void(bool)> changeCallback() const {
			return mChangeCallback;
		}

		void setChangeCallback(const function<void(bool)>& callback) {
			mChangeCallback = callback;
		}

		/// Set the button group (for radio buttons)
		void setButtonGroup(const vector<UIButton *>& buttonGroup) {
			mButtonGroup = buttonGroup;
		}

		const vector<UIButton *>& buttonGroup() const {
			return mButtonGroup;
		}

		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseUp(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;

		void setFontSize(int a_fontSize) {
			m_fontSize = a_fontSize;
			_updateMinSize();
		}

	protected:
		void draw(NVGcontext* ctx) override;
	private:
		void _updateMinSize();
	protected:
		string mCaption;
		int mIcon;
		int m_fontSize;
		IconPosition mIconPosition;
		bool mPushed;
		int mFlags;
		Color mBackgroundColor;
		Color mTextColor;
		bool mEnabled;
		function<void()> mCallback;
		function<void(bool)> mChangeCallback;
		vector<UIButton *> mButtonGroup;
	};
}
#endif
