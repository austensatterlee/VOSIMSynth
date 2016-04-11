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

#ifndef __UI__
#define __UI__
#include "NDPoint.h"
#include <IPlug/IPlugStructs.h>
#include <DSPMath.h>
#include <functional>

using namespace std;

namespace syn
{
	/**
	 * Stores an RGB color as a 3-dimenional point.
	 */
	class ColorPoint : public NDPoint<3>
	{
	public:
		ColorPoint(const NDPoint<3>& a_pt) :
			NDPoint<3>(a_pt),
			m_iColor{ 0xFF, int(a_pt[0] * 0xFF), int(a_pt[1] * 0xFF), int(a_pt[2] * 0xFF) }
		{
		}

		ColorPoint(unsigned int a_word) :
			ColorPoint(NDPoint<3>(double((a_word >> 16) & 0xFF) * 1.0 / 0xFF, double((a_word >> 8) & 0xFF) * 1.0 / 0xFF, double(a_word & 0xFF) * 1.0 / 0xFF)) {}
		
		ColorPoint(const IColor& a_color) :
			ColorPoint(NDPoint<3>(double(a_color.R), double(a_color.G), double(a_color.B)) / 255.0) {}

		const IColor& getIColor() const {
			return m_iColor;
		}

		unsigned getInt() const {
			return unsigned(0xFF << 24) | unsigned(m_pvec[0] * 0xFF) << 16 | unsigned(m_pvec[1] * 0xFF) << 8 | unsigned(m_pvec[2] * 0xFF);
		}

		LICE_pixel getLicePixel() const {
			return LICE_RGBA(int(m_pvec[0] * 0xFF), int(m_pvec[1] * 0xFF), int(m_pvec[2] * 0xFF), 0xFF);
		}

		int R() const {
			return m_pvec[0] * 0xFF;
		}
		int G() const {
			return m_pvec[1] * 0xFF;
		}
		int B() const {
			return m_pvec[2] * 0xFF;
		}
		float r() const {
			return m_pvec[0];
		}
		float g() const {
			return m_pvec[1];
		}
		float b() const {
			return m_pvec[2];
		}
	private:
		IColor m_iColor;
	};

	class CachedImage
	{
	public:		
		CachedImage() {}

		/*
		 Check that the watched values have not changed since the last time this method was called.
		 If they have, use the provided callable to refresh the cache by passing the cached bitmap as an argument.
		 The callable should have the signature void(LICE_IBitmap*).
		 */
		bool draw(function<void (LICE_IBitmap*)> f) {	
			bool redraw_cond = isDirty();
			if (redraw_cond) {
				f(&m_cachedImage);
				m_size[0] = m_cachedImage.getWidth();
				m_size[1] = m_cachedImage.getHeight();
				m_isFirstUpdate = false;
			}
			return redraw_cond;
		}
		/**
		 * Set a_size[1] negative to flip the bitmap vertically
		 */
		void blit(LICE_IBitmap* a_dst, const NDPoint<2,int>& a_pos, const NDPoint<2,int>& a_size, double a_alphaWeight=1.0, int a_mode=-1) {
			if(a_mode<0) {
				a_mode = LICE_BLIT_MODE_COPY | LICE_BLIT_FILTER_BILINEAR | LICE_BLIT_USE_ALPHA;
			}			
			LICE_WrapperBitmap wbmp{ m_cachedImage.getBits(), m_cachedImage.getWidth(), m_cachedImage.getHeight(), m_cachedImage.getRowSpan(), a_size[1] < 0 };
			LICE_ScaledBlit(a_dst, &wbmp, a_pos[0], a_pos[1], a_size[0], abs(a_size[1]), \
				0.0f, 0.0f, float(m_size[0]), float(m_size[1]), float(a_alphaWeight), a_mode);
		}
		void blit(LICE_IBitmap* a_dst, const NDPoint<2,int>& a_pos, double a_alphaWeight=1.0, int a_mode=-1) {
			blit(a_dst, a_pos, m_size, a_alphaWeight, a_mode);
		}
		template<typename... Args>
		void resetConds(const Args*... args) {
			m_conds.clear();
			addConds(args...);
		}
		template<typename First, typename... Rest>
		void addConds(const First* first,const Rest*... rest) {
			m_conds.push_back(make_unique<UpdateConditionImplem<First> >(first));
			addConds(rest...);
		}
		template<typename Only>
		void addConds(const Only* only) {
			m_conds.push_back(make_unique<UpdateConditionImplem<Only> >(only));
		}
		int width() const { return m_size[0]; }
		int height() const { return m_size[1]; }
		NDPoint<2, int> size() const { return m_size; }
		bool isDirty() const { 
			bool redraw_cond = m_isFirstUpdate;
			for (int i = 0; i < m_conds.size();i++) {
				redraw_cond |= m_conds[i]->update();
			} 
			return redraw_cond;
		}
		void setDirty() {
			m_isFirstUpdate = true;
		}
	private:
		class UpdateCondition
		{
		public:
			virtual ~UpdateCondition() {}
			virtual bool update() = 0;
		};

		template<typename T>
		class UpdateConditionImplem : public UpdateCondition
		{		
			T lastval;
			const T* currval;
		public:	
			explicit UpdateConditionImplem(const T* a_addr) 
				: currval(a_addr) 
			{}

			bool update() override {
				bool result = *currval != lastval;
				lastval = *currval;
				return result;
			}
		};
		LICE_SysBitmap m_cachedImage;
		NDPoint<2,int> m_size;
		vector<unique_ptr<UpdateCondition> > m_conds;
		bool m_isFirstUpdate = true;
	};	

	inline IRECT makeIRectFromPoints(int x1, int y1, int x2, int y2) {
		int L = MIN(x1, x2);
		int R = MAX(x1, x2);
		int T = MIN(y1, y2);
		int B = MAX(y1, y2);
		return{ L,T,R,B };
	}

	enum PALETTE
	{
		PRIMARY = 0,
		SECONDARY,
		TERTIARY,
		COMPLEMENT
	};

	const ColorPoint globalPalette[4][5] = {
		{
			0x3F51B5,
			0x5C6BC0,
			0x7986CB,
			0x9FA8DA,
			0xC5CAE9,
		},
		{
			0x2196F3,
			0x42A5F5,
			0x64B5F6,
			0x90CAF9,
			0xBBDEFB
		},
		{
			0x4CAF50,
			0x66BB6A,
			0x81C784,
			0xA5D6A7,
			0xC8E6C9
		},
		{
			0x80D8FF,
			0x40C4FF,
			0x00B0FF,
			0x0091EA,
			0x01579B
		}
	};
}
#endif

