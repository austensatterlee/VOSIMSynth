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
 *  \file UITabComponent.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 07/2016
 */

#ifndef __UITABCOMPONENT__
#define __UITABCOMPONENT__
#include "UIComponent.h"

namespace synui {
	class UITabWidget : public UIComponent {
	public:
		UITabWidget(MainWindow* a_window);

		void setActiveTab(int tabIndex);
		int activeTab() const;
		int tabCount() const;

		/**
		* Sets the callable objects which is invoked when a tab is changed.
		* The argument provided to the callback is the index of the new active tab.
		*/
		void setCallback(const std::function<void(int)> &callback) { mCallback = callback; };
		const std::function<void(int)> &callback() const { return mCallback; }

		/// Creates a new tab with the specified name and returns a pointer to the layer.
		UIComponent* createTab(const std::string &label);
		UIComponent* createTab(int index, const std::string &label);

		/// Inserts a tab at the end of the tabs collection and associates it with the provided UIComponent.
		void addTab(const std::string &label, UIComponent *tab);

		/// Inserts a tab into the tabs collection at the specified index and associates it with the provided UIComponent.
		void addTab(int index, const std::string &label, UIComponent *tab);

		/**
		* Removes the tab with the specified label and returns the index of the label.
		* Returns whether the removal was successful.
		*/
		bool removeTab(const std::string &label);

		/// Removes the tab with the specified index.
		void removeTab(int index);

		/// Retrieves the label of the tab at a specific index.
		const std::string &tabLabelAt(int index) const;

		const UITabHeader& header() const;
		const UIStackedComponent& content() const;

		/**
		* Retrieves the index of a specific tab using its tab label.
		* Returns -1 if there is no such tab.
		*/
		int tabLabelIndex(const std::string &label);

		/**
		* Retrieves the index of a specific tab using a UIComponent pointer.
		* Returns -1 if there is no such tab.
		*/
		int tabIndex(UIComponent* tab);

		/**
		* This function can be invoked to ensure that the tab with the provided
		* index the is visible, i.e to track the given tab. Forwards to the tab
		* header UIComponent. This function should be used whenever the client wishes
		* to make the tab header follow a newly added tab, as the content of the
		* new tab is made visible but the tab header does not track it by default.
		*/
		void ensureTabVisible(int index);

		const UIComponent* tab(const std::string &label) const;
		UIComponent* tab(const std::string &label);

		virtual void performLayout(NVGcontext* ctx) override;
		virtual Vector2i calcMinSize(NVGcontext* ctx) const;
		virtual void draw(NVGcontext* ctx) override;

		void notifyChildResized(UIComponent* a_child) override;
	private:
		void _onResize() override;
	private:
		UITabHeader* mHeader;
		UIStackedComponent* mContent;

		std::function<void(int)> mCallback;
	};
}
#endif
