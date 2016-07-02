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
 *  \file UICoord.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 06/2016
 */

#ifndef __UICOORD__
#define __UICOORD__

#include <eigen/Core>

namespace synui {
	class UIComponent;

	/**
	 * Represents a point inside a UIComponent's local coordinate system.
	 */
	class UICoord
	{
		Eigen::Vector2i m_globalPt;
		const UIComponent* m_referenceComponent;
	public:
		UICoord();
		explicit UICoord(const Eigen::Vector2i& a_globalPt);
		/**
		 * Transform the point into a new UIComponent's coordinate system 
		 */
		UICoord(const UICoord& a_coord, const UIComponent* a_referenceComponent);
		Eigen::Vector2i globalCoord() const;
		Eigen::Vector2i localCoord(const UIComponent* a_refComponent) const;
		UICoord toComponent(const UIComponent* a_refComponent) const;
		const UIComponent* refComponent() const;
		friend UICoord operator+(const UICoord& a_coord, const Eigen::Vector2i& a_offset);
		friend UICoord operator-(const UICoord& a_coord, const Eigen::Vector2i& a_offset);
	private:
		UICoord(const Eigen::Vector2i& a_pt, const UIComponent* a_referenceComponent);
	};
}

#endif
