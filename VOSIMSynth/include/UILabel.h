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
 *  \file UILabel.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UILABEL__
#define __UILABEL__
#include "UIComponent.h"

namespace syn
{
	class UILabel : public UIComponent
	{
	public:
		UILabel(VOSIMWindow* a_window);

		void setText(const string& a_newText);

		void setFontSize(float a_newFontSize);

		void setFontBlur(float a_fontBlur);

		void setFontColor(const Color& a_newColor);

		void setFontFace(int a_fontFaceId);

		void setAlignment(int a_newAlign);

		const string& text() const;

		float fontSize() const;

		float fontBlur() const;

		const Color& fontColor() const;

		int fontFace() const;		

		int alignment() const;

		UIComponent* onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;

	protected:
		void draw(NVGcontext* a_nvg) override;

	private:
		Vector2i _measureText() const;

	private:
		string m_text;
		int m_fontFace;
		Color m_color;
		float m_fontSize;
		float m_fontBlur;
		int m_align;
	};
}
#endif
