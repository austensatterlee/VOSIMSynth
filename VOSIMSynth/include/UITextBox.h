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
 *  \file UIUITextBox.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UIUITextBox__
#define __UIUITextBox__
#include "UIComponent.h"

namespace syn
{
	class UITextBox : public UIComponent
	{
	public:
		enum class Alignment
		{
			Left,
			Center,
			Right
		};

		UITextBox(MainWindow* a_window, const string& value = "Untitled");

		bool editable() const {
			return mEditable;
		}

		void setEditable(bool editable);

		virtual const string& value() const {
			return mValue;
		}

		virtual void setValue(const string& value) {
			mValue = value; 
			updateMinSize_();
		}

		const string& defaultValue() const {
			return mDefaultValue;
		}

		void setDefaultValue(const string& defaultValue) {
			mDefaultValue = defaultValue;
		}

		Alignment alignment() const {
			return mAlignment;
		}

		void setAlignment(Alignment align) {
			mAlignment = align;
		}

		const string& units() const {
			return mUnits;
		}

		void setUnits(const string& units) {
			mUnits = units;
			updateMinSize_();
		}

		int unitsImage() const {
			return mUnitsImage;
		}

		void setUnitsImage(int image) {
			mUnitsImage = image;
			updateMinSize_();
		}

		/// Return the underlying regular expression specifying valid formats
		const string& format() const {
			return mFormat;
		}

		/// Specify a regular expression specifying valid formats
		void setFormat(const string& format) {
			mFormat = format;
		}

		void setFontSize(float a_newFontSize) {
			mFontSize = a_newFontSize;
			updateMinSize_();
		}

		/// Set the change callback
		function<bool(const string& str)> callback() const {
			return mCallback;
		}

		virtual void setCallback(const function<bool(const string& str)>& callback) {
			mCallback = callback;
		}

		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseUp(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseMove(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;
		void onFocusEvent(bool a_isFocused) override;

		bool onKeyDown(const sf::Event::KeyEvent& a_key) override;
		bool onTextEntered(sf::Uint32 a_unicode) override;

	protected:
		void draw(NVGcontext* ctx) override;
		void updateMinSize_();

	protected:
		bool checkFormat(const string& input, const string& format) const;
		bool copySelection();
		void pasteFromClipboard();
		bool deleteSelection();

		void updateCursor(NVGcontext* ctx, float lastx,
		                  const NVGglyphPosition* glyphs, int size);
		static float cursorIndex2Position(int index, float lastx,
		                                  const NVGglyphPosition* glyphs, int size);
		int position2CursorIndex(float posx, float lastx,
		                         const NVGglyphPosition* glyphs, int size) const;

	protected:
		bool mEnabled;
		bool mEditable;
		bool mCommitted;
		string mValue;
		string mDefaultValue;
		Alignment mAlignment;
		string mUnits;
		string mFormat;
		int mUnitsImage;
		function<bool(const string& str)> mCallback;
		bool mValidFormat;
		string mValueTemp;
		int mCursorPos;
		int mSelectionPos;
		Vector2i mMousePos;
		Vector2i mMouseDownPos;
		Vector2i mMouseDragPos;
		int mMouseDownModifier;
		float mTextOffset;
		float mFontSize;
		double mLastClick;
	};

	template <typename Scalar>
	class UIIntBox : public UITextBox
	{
	public:
		UIIntBox(MainWindow* a_window, Scalar value = (Scalar)0) : UITextBox(a_window) {
			setDefaultValue("0");
			setFormat(is_signed<Scalar>::value ? "[-]?[0-9]*" : "[0-9]*");
			UIIntBox<Scalar>::setValue(value);
		}

		Scalar value() const override {
			Scalar value;
			istringstream iss(UITextBox::value());
			if (!(iss >> value))
				throw invalid_argument("Could not parse integer value!");
			return value;
		}

		void setValue(Scalar value) override {
			UITextBox::setValue(std::to_string(value));
		}

		void setCallback(const function<void(Scalar)>& cb) override {
			UITextBox::setCallback(
				[cb, this](const string& str) {
					istringstream iss(str);
					Scalar value;
					if (!(iss >> value))
						throw invalid_argument("Could not parse integer value!");
					setValue(value);
					cb(value);
					return true;
				}
			);
		}
	};

	template <typename Scalar>
	class UIFloatBox : public UITextBox
	{
	public:
		UIFloatBox(MainWindow* a_window, Scalar value = (Scalar) 0.f) : UITextBox(a_window) {
			mNumberFormat = sizeof(Scalar) == sizeof(float) ? "%.4g" : "%.7g";
			setDefaultValue("0");
			setFormat("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");
			UIFloatBox<Scalar>::setValue(value);
		}

		string numberFormat() const {
			return mNumberFormat;
		}

		void numberFormat(const string& format) {
			mNumberFormat = format;
		}

		Scalar value() const override {
			return (Scalar)stod(UITextBox::value());
		}

		void setValue(Scalar value) override {
			char buffer[50];
			NANOGUI_SNPRINTF(buffer, 50, mNumberFormat.c_str(), value);
			UITextBox::setValue(buffer);
		}

		void setCallback(const function<void(Scalar)>& cb) override {
			UITextBox::setCallback([cb, this](const string& str) {
				Scalar scalar = (Scalar)stod(str);
				setValue(scalar);
				cb(scalar);
				return true;
			});
		}

	private:
		string mNumberFormat;
	};
}
#endif
