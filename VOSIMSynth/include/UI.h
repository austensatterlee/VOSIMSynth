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
#include <IPlug/IPlugStructs.h>
#include <DSPMath.h>
#include <functional>
#include <eigen/Core>
#include <nanovg.h>
#include <vector>
#include <memory>

using namespace std; 

namespace syn
{
	using Eigen::Vector2f;
	using Eigen::Vector3f;
	using Eigen::Vector4f;
	using Eigen::Vector2i;
	using Eigen::Vector3i;
	using Eigen::Vector4i;
	using Eigen::Matrix3f;
	using Eigen::Matrix4f;
	using Eigen::VectorXf;
	using Eigen::MatrixXf;

	typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXu;

	/// Stores an RGBA color value
	class Color : public Eigen::Matrix<float, 4, 1, Eigen::DontAlign> {
		typedef Eigen::Matrix<float,4,1,Eigen::DontAlign> Base;
	public:
		Color() : Color(0, 0, 0, 0) {}

		Color(const Eigen::Vector4f &color) : Base(color) { }

		Color(const Eigen::Vector3f &color, float alpha)
			: Color(color(0), color(1), color(2), alpha) { }

		Color(const Eigen::Vector3i &color, int alpha)
			: Color(color.cast<float>() / 255.f, alpha / 255.f) { }

		Color(const Eigen::Vector3f &color) : Color(color, 1.0f) {}

		Color(const Eigen::Vector3i &color)
			: Color(static_cast<Vector3f>(color.cast<float>() / 255.f)) { }

		Color(const Eigen::Vector4i &color)
			: Color(static_cast<Vector4f>(color.cast<float>() / 255.f)) { }

		Color(float intensity, float alpha)
			: Color(Vector3f::Constant(intensity), alpha) { }

		Color(int intensity, int alpha)
			: Color(Vector3i::Constant(intensity), alpha) { }

		Color(float r, float g, float b, float a) : Color(Vector4f(r, g, b, a)) { }

		Color(int r, int g, int b, int a) : Color(Vector4i(r, g, b, a)) { }

		/// Construct a color vector from MatrixBase (needed to play nice with Eigen)
		template <typename Derived> Color(const Eigen::MatrixBase<Derived>& p)
			: Base(p) { }

		/// Assign a color vector from MatrixBase (needed to play nice with Eigen)
		template <typename Derived> Color &operator=(const Eigen::MatrixBase<Derived>& p) {
			this->Base::operator=(p);
			return *this;
		}

		/// Return a reference to the red channel
		float &r() { return x(); }
		/// Return a reference to the red channel (const version)
		const float &r() const { return x(); }
		/// Return a reference to the green channel
		float &g() { return y(); }
		/// Return a reference to the green channel (const version)
		const float &g() const { return y(); }
		/// Return a reference to the blue channel
		float &b() { return z(); }
		/// Return a reference to the blue channel (const version)
		const float &b() const { return z(); }

		Color contrastingColor() const {
			float luminance = cwiseProduct(Color(0.299f, 0.587f, 0.144f, 0.f)).sum();
			return Color(luminance < 0.5f ? 1.f : 0.f, 1.f);
		}

		inline operator const NVGcolor &() const;
	};

	inline Color::operator const struct NVGcolor&() const {
		return reinterpret_cast<const NVGcolor &>(*this->data());
	}

	/* Cursor shapes */
	enum class Cursor {
		Arrow = 0,
		IBeam,
		Crosshair,
		Hand,
		HResize,
		VResize,
		CursorCount
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
}
#endif

