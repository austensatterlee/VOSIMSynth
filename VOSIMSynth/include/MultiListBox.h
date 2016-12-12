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
 *  \file MultiListBox.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include <TGUI/TGUI.hpp>
#include <TGUI/Widgets/Panel.hpp>
#include <TGUI/Widgets/Label.hpp>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <TGUI/Widgets/ListBox.hpp>

namespace synui
{
	/**
	 * \brief Nested list box widget.
	 * 
	 * Multiple lists may be added, and connections may be formed between list items and other lists. When the
	 * user clicks on a list item, the target list is then activated.
	 * 
	 * Signals:
	 *     - ItemSelected 
	 *         * Optional parameter synui::MultiListBox::ItemLocation: A std::pair of strings containing the name of the selected list and item, respectively.
	 */
	class MultiListBox : public tgui::Panel
	{
	public:
		typedef std::string Item;
		typedef std::vector<Item> List;
		typedef std::string ListLocation;
		typedef std::pair<const ListLocation, const std::string> ItemLocation;

		typedef std::shared_ptr<MultiListBox> Ptr; ///< Shared widget pointer
		typedef std::shared_ptr<const MultiListBox> ConstPtr; ///< Shared constant widget pointer

	private:
		std::unordered_map<ListLocation, List> m_lists;
		std::unordered_map<ItemLocation, ListLocation, boost::hash<ItemLocation>> m_edges;

	public:
		MultiListBox();

		/**
		 * \brief Sets a list as the "root". The root is the first visible list. The root list can always be identified with the name "root".
		 * \param a_list The list to be assigned as the root list.
		 */
		void setRootList(List a_list);

		/**
		 * \brief Register a new list. Note that in may not be visible until at least one connection to it has been defined.
		 * \param a_listLoc The name that will be used to identify the list.
		 * \param a_list The list of items.
		 * \return True if the list was successfully added, false otherwise (e.g. duplicate location, empty location).
		 */
		bool addList(const ListLocation& a_listLoc, const List& a_list);
		
		bool addList(const ListLocation& a_listLoc, tgui::ListBox::Ptr a_list);

		/**
		 * \brief Form a connection between a ListItem and a List. Clicking on a ListItem will activate its target List.
		 * \param a_sourceItemLoc The source ListItem's identifier.
		 * \param a_targetListLoc The target List's identifier.
		 * \return True if the edge was successfully added, false otherwise (e.g. edge formed a cycle).
		 */
		bool addEdge(const ItemLocation& a_sourceItemLoc, const ListLocation& a_targetListLoc);

		Widget::Ptr clone() const override;

	private:
		/**
		 * \brief Detect if the current configuration contains any cycles (e.g. List A -> List B -> List A).
		 * \return True if a cycle exists, false otherwise.
		 */
		bool checkForCycles() const;
	};
}
