#include "UITextBox.h"
#include "Theme.h"
#include <regex>

namespace syn
{
	UITextBox::UITextBox(VOSIMWindow* a_window, const string& value)
		: UIComponent(a_window),
		  mEnabled(true),
		  mEditable(true),
		  mCommitted(true),
		  mValue(value),
		  mDefaultValue(""),
		  mAlignment(Alignment::Left),
		  mUnits(""),
		  mFormat(""),
		  mUnitsImage(-1),
		  mValidFormat(true),
		  mValueTemp(value),
		  mCursorPos(-1),
		  mSelectionPos(-1),
		  mMousePos(-1, -1),
		  mMouseDownPos(-1, -1),
		  mMouseDragPos(-1, -1),
		  mMouseDownModifier(0),
		  mTextOffset(0),
		  mLastClick(0) 
	{
		mFontSize = m_window->theme()->mTextBoxFontSize;
		updateMinSize_();
	}

	void UITextBox::setEditable(bool editable) {
		mEditable = editable;
		// \todo
		//setCursor(editable ? Cursor::IBeam : Cursor::Arrow);
	}

	void UITextBox::updateMinSize_() {
		NVGcontext* ctx = m_window->getContext();
		Vector2i minsize(0, mFontSize * 1.4f);

		float uw = 0;
		if (mUnitsImage > 0) {
			int w, h;
			nvgImageSize(ctx, mUnitsImage, &w, &h);
			float uh = minsize(1) * 0.4f;
			uw = w * uh / h;
		} else if (!mUnits.empty()) {
			uw = nvgTextBounds(ctx, 0, 0, mUnits.c_str(), nullptr, nullptr);
		}

		float ts = nvgTextBounds(ctx, 0, 0, mValue.c_str(), nullptr, nullptr);
		minsize(0) = minsize(1) + ts + uw;
		setMinSize(minsize);
	}

	void UITextBox::draw(NVGcontext* ctx) {
		UIComponent::draw(ctx);

		NVGpaint bg = nvgBoxGradient(ctx,
		                             0 + 1, 0 + 1 + 1.0f, size().x() - 2, size().y() - 2,
		                             3, 4, Color(255, 32), Color(32, 32));
		NVGpaint fg1 = nvgBoxGradient(ctx,
		                              0 + 1, 0 + 1 + 1.0f, size().x() - 2, size().y() - 2,
		                              3, 4, Color(150, 32), Color(32, 32));
		NVGpaint fg2 = nvgBoxGradient(ctx,
		                              0 + 1, 0 + 1 + 1.0f, size().x() - 2, size().y() - 2,
		                              3, 4, nvgRGBA(255, 0, 0, 100), nvgRGBA(255, 0, 0, 50));

		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, 0 + 1, 0 + 1 + 1.0f, size().x() - 2,
		               size().y() - 2, 3);

		if (mEditable && focused())
			mValidFormat ? nvgFillPaint(ctx, fg1) : nvgFillPaint(ctx, fg2);
		else
			nvgFillPaint(ctx, bg);

		nvgFill(ctx);

		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, 0 + 0.5f, 0 + 0.5f, size().x() - 1,
		               size().y() - 1, 2.5f);
		nvgStrokeColor(ctx, Color(0, 48));
		nvgStroke(ctx);

		nvgFontSize(ctx, mFontSize);
		nvgFontFace(ctx, "sans");
		Vector2i drawPos(0, 0 + size().y() * 0.5f + 1);

		float xSpacing = size().y() * 0.3f;

		float unitWidth = 0;

		if (mUnitsImage > 0) {
			int w, h;
			nvgImageSize(ctx, mUnitsImage, &w, &h);
			float unitHeight = size().y() * 0.4f;
			unitWidth = w * unitHeight / h;
			NVGpaint imgPaint = nvgImagePattern(
				ctx, 0 + size().x() - xSpacing - unitWidth,
				drawPos.y() - unitHeight * 0.5f, unitWidth, unitHeight, 0,
				mUnitsImage, mEnabled ? 0.7f : 0.35f);
			nvgBeginPath(ctx);
			nvgRect(ctx, 0 + size().x() - xSpacing - unitWidth,
			        drawPos.y() - unitHeight * 0.5f, unitWidth, unitHeight);
			nvgFillPaint(ctx, imgPaint);
			nvgFill(ctx);
			unitWidth += 2;
		} else if (!mUnits.empty()) {
			unitWidth = nvgTextBounds(ctx, 0, 0, mUnits.c_str(), nullptr, nullptr);
			nvgFillColor(ctx, Color(255, mEnabled ? 64 : 32));
			nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
			nvgText(ctx, 0 + size().x() - xSpacing, drawPos.y(),
			        mUnits.c_str(), nullptr);
			unitWidth += 2;
		}

		switch (mAlignment) {
		case Alignment::Left:
			nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
			drawPos.x() += xSpacing;
			break;
		case Alignment::Right:
			nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
			drawPos.x() += size().x() - unitWidth - xSpacing;
			break;
		case Alignment::Center:
			nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			drawPos.x() += size().x() * 0.5f;
			break;
		}

		nvgFontSize(ctx, mFontSize);
		nvgFillColor(ctx,
		             mEnabled ? theme()->mTextColor : theme()->mDisabledTextColor);

		// clip visible text area
		float clipX = 0 + xSpacing - 1.0f;
		float clipY = 0 + 1.0f;
		float clipWidth = size().x() - unitWidth - 2 * xSpacing + 2.0f;
		float clipHeight = size().y() - 3.0f;
		nvgScissor(ctx, clipX, clipY, clipWidth, clipHeight);

		Vector2i oldDrawPos(drawPos);
		drawPos.x() += mTextOffset;

		if (mCommitted) {
			nvgText(ctx, drawPos.x(), drawPos.y(), mValue.c_str(), nullptr);
		} else {
			const int maxGlyphs = 1024;
			NVGglyphPosition glyphs[maxGlyphs];
			float textBound[4];
			nvgTextBounds(ctx, drawPos.x(), drawPos.y(), mValueTemp.c_str(),
			              nullptr, textBound);
			float lineh = textBound[3] - textBound[1];

			// find cursor positions
			int nglyphs =
					nvgTextGlyphPositions(ctx, drawPos.x(), drawPos.y(),
					                      mValueTemp.c_str(), nullptr, glyphs, maxGlyphs);
			updateCursor(ctx, textBound[2], glyphs, nglyphs);

			// compute text offset
			int prevCPos = mCursorPos > 0 ? mCursorPos - 1 : 0;
			int nextCPos = mCursorPos < nglyphs ? mCursorPos + 1 : nglyphs;
			float prevCX = cursorIndex2Position(prevCPos, textBound[2], glyphs, nglyphs);
			float nextCX = cursorIndex2Position(nextCPos, textBound[2], glyphs, nglyphs);

			if (nextCX > clipX + clipWidth)
				mTextOffset -= nextCX - (clipX + clipWidth) + 1;
			if (prevCX < clipX)
				mTextOffset += clipX - prevCX + 1;

			drawPos.x() = oldDrawPos.x() + mTextOffset;

			// draw text with offset
			nvgText(ctx, drawPos.x(), drawPos.y(), mValueTemp.c_str(), nullptr);
			nvgTextBounds(ctx, drawPos.x(), drawPos.y(), mValueTemp.c_str(),
			              nullptr, textBound);

			// recompute cursor positions
			nglyphs = nvgTextGlyphPositions(ctx, drawPos.x(), drawPos.y(),
			                                mValueTemp.c_str(), nullptr, glyphs, maxGlyphs);

			if (mCursorPos > -1) {
				if (mSelectionPos > -1) {
					float caretx = cursorIndex2Position(mCursorPos, textBound[2],
					                                    glyphs, nglyphs);
					float selx = cursorIndex2Position(mSelectionPos, textBound[2],
					                                  glyphs, nglyphs);

					if (caretx > selx)
						swap(caretx, selx);

					// draw selection
					nvgBeginPath(ctx);
					nvgFillColor(ctx, nvgRGBA(255, 255, 255, 80));
					nvgRect(ctx, caretx, drawPos.y() - lineh * 0.5f, selx - caretx,
					        lineh);
					nvgFill(ctx);
				}

				float caretx = cursorIndex2Position(mCursorPos, textBound[2], glyphs, nglyphs);

				// draw cursor
				nvgBeginPath(ctx);
				nvgMoveTo(ctx, caretx, drawPos.y() - lineh * 0.5f);
				nvgLineTo(ctx, caretx, drawPos.y() + lineh * 0.5f);
				nvgStrokeColor(ctx, nvgRGBA(255, 192, 0, 255));
				nvgStrokeWidth(ctx, 1.0f);
				nvgStroke(ctx);
			}
		}

		nvgResetScissor(ctx);
	}

	UIComponent* UITextBox::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {

		if (mEditable && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			mMouseDownPos = a_relCursor - getRelPos();

			if (a_isDblClick) {
				/* Double-click: select all text */
				mSelectionPos = 0;
				mCursorPos = (int)mValueTemp.size();
				mMouseDownPos = Vector2i(-1, 1);
			}
		}

		return this;
	}

	bool UITextBox::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		mMouseDownPos = Vector2i(-1, -1);
		mMouseDragPos = Vector2i(-1, -1);
		return true;
	}

	bool UITextBox::onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		if (mEditable && focused()) {
			mMousePos = a_relCursor - getRelPos();
			return true;
		}
		return false;
	}

	bool UITextBox::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		if (mEditable && focused() && a_diffCursor.squaredNorm() >= 1) {
			mMouseDragPos = a_relCursor - getRelPos();
			return true;
		}
		return false;
	}

	void UITextBox::onFocusEvent(bool a_isFocused) {
		UIComponent::onFocusEvent(a_isFocused);

		string backup = mValue;

		if (mEditable) {
			if (a_isFocused) {
				mValueTemp = mValue;
				mCommitted = false;
				mCursorPos = 0;
			} else {
				if (mValidFormat) {
					if (mValueTemp == "")
						mValue = mDefaultValue;
					else
						mValue = mValueTemp;
				}

				if (mCallback && !mCallback(mValue))
					mValue = backup;

				mValidFormat = true;
				mCommitted = true;
				mCursorPos = -1;
				mSelectionPos = -1;
				mTextOffset = 0;
			}

			mValidFormat = (mValueTemp == "") || checkFormat(mValueTemp, mFormat);
		}
	}

	bool UITextBox::onKeyDown(const sf::Event::KeyEvent& a_keyEvent) {
		if (mEditable && focused()) {
			sf::Keyboard::Key key = a_keyEvent.code;
			if (key == sf::Keyboard::Left) {
				if (a_keyEvent.shift) {
					if (mSelectionPos == -1)
						mSelectionPos = mCursorPos;
				} else {
					mSelectionPos = -1;
				}

				if (mCursorPos > 0)
					mCursorPos--;
			} else if (key == sf::Keyboard::Right) {
				if (a_keyEvent.shift) {
					if (mSelectionPos == -1)
						mSelectionPos = mCursorPos;
				} else {
					mSelectionPos = -1;
				}

				if (mCursorPos < (int)mValueTemp.length())
					mCursorPos++;
			} else if (key == sf::Keyboard::Home) {
				if (a_keyEvent.shift) {
					if (mSelectionPos == -1)
						mSelectionPos = mCursorPos;
				} else {
					mSelectionPos = -1;
				}

				mCursorPos = 0;
			} else if (key == sf::Keyboard::End) {
				if (a_keyEvent.shift) {
					if (mSelectionPos == -1)
						mSelectionPos = mCursorPos;
				} else {
					mSelectionPos = -1;
				}

				mCursorPos = (int)mValueTemp.size();
			} else if (key == sf::Keyboard::BackSpace) {
				if (!deleteSelection()) {
					if (mCursorPos > 0) {
						mValueTemp.erase(mValueTemp.begin() + mCursorPos - 1);
						mCursorPos--;
					}
				}
			} else if (key == sf::Keyboard::Delete) {
				if (!deleteSelection()) {
					if (mCursorPos < (int)mValueTemp.length())
						mValueTemp.erase(mValueTemp.begin() + mCursorPos);
				}
			} else if (key == sf::Keyboard::Return) {
				if (!mCommitted)
					m_window->forfeitFocus(this);
			} else if (key == sf::Keyboard::A && a_keyEvent.control) {
				mCursorPos = (int)mValueTemp.length();
				mSelectionPos = 0;
			} else if (key == sf::Keyboard::X && a_keyEvent.control) {
				copySelection();
				deleteSelection();
			} else if (key == sf::Keyboard::C && a_keyEvent.control) {
				copySelection();
			} else if (key == sf::Keyboard::V && a_keyEvent.control) {
				deleteSelection();
				pasteFromClipboard();
			}

			mValidFormat = (mValueTemp == "") || checkFormat(mValueTemp, mFormat);


			return true;
		}

		return false;
	}

	bool UITextBox::onTextEntered(sf::Uint32 a_unicode) {
		if (mEditable && focused()) {
			ostringstream convert;
			convert << (char)a_unicode;

			deleteSelection();
			mValueTemp.insert(mCursorPos, convert.str());
			mCursorPos++;

			mValidFormat = (mValueTemp == "") || checkFormat(mValueTemp, mFormat);

			return true;
		}

		return false;
	}

	bool UITextBox::checkFormat(const string& input, const string& format) const {
		if (format.empty())
			return true;
		regex regex(format);
		return regex_match(input, regex);
	}

	bool UITextBox::copySelection() {
		/// \todo
		//		if (mSelectionPos > -1) {
		//			Screen *sc = dynamic_cast<Screen *>(this->window()->parent());
		//
		//			int begin = mCursorPos;
		//			int end = mSelectionPos;
		//
		//			if (begin > end)
		//				swap(begin, end);
		//
		//			glfwSetClipboardString(sc->glfwWindow(),
		//				mValueTemp.substr(begin, end).c_str());
		//			return true;
		//		}

		return false;
	}

	void UITextBox::pasteFromClipboard() {
		/// \todo
		//		Screen *sc = dynamic_cast<Screen *>(this->window()->parent());
		//		string str(glfwGetClipboardString(sc->glfwWindow()));		
		//		mValueTemp.insert(mCursorPos, str);
	}

	bool UITextBox::deleteSelection() {
		if (mSelectionPos > -1) {
			int begin = mCursorPos;
			int end = mSelectionPos;

			if (begin > end)
				swap(begin, end);

			if (begin == end - 1)
				mValueTemp.erase(mValueTemp.begin() + begin);
			else
				mValueTemp.erase(mValueTemp.begin() + begin,
				                 mValueTemp.begin() + end);

			mCursorPos = begin;
			mSelectionPos = -1;
			return true;
		}

		return false;
	}

	void UITextBox::updateCursor(NVGcontext*, float lastx,
	                             const NVGglyphPosition* glyphs, int size) {
		// handle mouse cursor events
		if (mMouseDownPos.x() != -1) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
				if (mSelectionPos == -1)
					mSelectionPos = mCursorPos;
			} else
				mSelectionPos = -1;

			mCursorPos = position2CursorIndex(mMouseDownPos.x(), lastx, glyphs, size);

			mMouseDownPos = Vector2i(-1, -1);
		} else if (mMouseDragPos.x() != -1) {
			if (mSelectionPos == -1)
				mSelectionPos = mCursorPos;

			mCursorPos = position2CursorIndex(mMouseDragPos.x(), lastx, glyphs, size);
		} else {
			// set cursor to last character
			if (mCursorPos == -2)
				mCursorPos = size;
		}

		if (mCursorPos == mSelectionPos)
			mSelectionPos = -1;
	}

	float UITextBox::cursorIndex2Position(int index, float lastx,
	                                      const NVGglyphPosition* glyphs, int size) {
		float pos;
		if (index == size)
			pos = lastx; // last character
		else
			pos = glyphs[index].x;

		return pos;
	}

	int UITextBox::position2CursorIndex(float posx, float lastx,
	                                    const NVGglyphPosition* glyphs, int size) const {
		int mCursorId = 0;
		float caretx = glyphs[mCursorId].x;
		for (int j = 1; j < size; j++) {
			if (abs(caretx - posx) > abs(glyphs[j].x - posx)) {
				mCursorId = j;
				caretx = glyphs[mCursorId].x;
			}
		}
		if (abs(caretx - posx) > abs(lastx - posx))
			mCursorId = size;

		return mCursorId;
	}
}
