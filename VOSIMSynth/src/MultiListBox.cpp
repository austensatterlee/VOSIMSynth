#include <MultiListBox.h>

synui::MultiListBox::MultiListBox(): Panel()
{
	m_callback.widgetType = "MultiListBox";
	m_draggableWidget = true;

	addSignal<sf::String, sf::String>("ItemSelected");
}

void synui::MultiListBox::setRootList(List a_list) { m_lists["root"] = a_list; }

bool synui::MultiListBox::addList(const ListLocation& a_listLoc, const List& a_list)
{
	// check for duplicates
	if (m_lists.find(a_listLoc) != m_lists.end())
		return false;

	// check for empty location
	if (a_listLoc.size() == 0)
		return false;

	// Create the widget that will display the list
	tgui::ListBox::Ptr lb = std::make_shared<tgui::ListBox>();
	for (const Item& item : a_list) { lb->addItem(item); }

	return addList(a_listLoc, lb);
}

bool synui::MultiListBox::addList(const ListLocation& a_listLoc, tgui::ListBox::Ptr a_listBox)
{
	// check for duplicates
	if (m_lists.find(a_listLoc) != m_lists.end())
		return false;

	// check for empty location
	if (a_listLoc.size() == 0)
		return false;

	// Add list
	m_lists.insert_or_assign(a_listLoc, std::vector<Item>{a_listBox->getItemCount()});
	for(int i=0;i<a_listBox->getItemCount();i++){
		m_lists[a_listLoc][i] = a_listBox->getItems()[i];
	}

	// Add widget
	add(a_listBox, a_listLoc);

	a_listBox->connect("ItemSelected", [&](const Item& a_selectedItem) {
			ItemLocation itemLoc{a_listLoc, a_selectedItem};
			// If the selected item has a target list, activate it
			if (m_edges.find(itemLoc) != m_edges.end()) {
				const ListLocation& targetListLoc = m_edges[itemLoc];
				// activate(targetListLoc); //TODO
			}
			sendSignal("ItemSelected", itemLoc);
		});

	return true;
}

bool synui::MultiListBox::addEdge(const ItemLocation& a_sourceItemLoc, const ListLocation& a_targetListLoc)
{
	// Check if the given source item already has an outgoing edge
	if (m_edges.find(a_sourceItemLoc) != m_edges.end())
		return false;

	// Add the edge
	m_edges.insert_or_assign(a_sourceItemLoc, a_targetListLoc);

	// If adding the edge formed a cycle, remove the edge again
	if (checkForCycles()) {
		m_edges.erase(a_sourceItemLoc);
		return false;
	}

	return true;
}

tgui::Widget::Ptr synui::MultiListBox::clone() const
{
	return std::make_shared<MultiListBox>(*this); //TODO
}

bool synui::MultiListBox::checkForCycles() const
{
	std::set<ListLocation> stack; // current path.
	std::set<ListLocation> visited; // lists we have already seen.

	std::function<bool(const ListLocation&)> _checkForCycles = [this, &stack, &visited, &_checkForCycles](const ListLocation& a_listLoc)-> bool {
				if (visited.find(a_listLoc) == visited.end()) {
					// Mark as visisted
					visited.insert(a_listLoc);
					// Add to current path
					stack.insert(a_listLoc);

					// Check if children are cyclic
					const List& list = m_lists.at(a_listLoc);
					for (int itemIndex = 0; itemIndex < list.size(); itemIndex++) {
						ItemLocation itemLoc = make_pair(a_listLoc, list[itemIndex]);

						// Move on if the item has no children
						if (m_edges.find(itemLoc) == m_edges.end())
							continue;

						ListLocation targetList = m_edges.at(itemLoc);

						// Check for cycles in lists we haven't already seen
						if (visited.find(targetList) == visited.end() && _checkForCycles(targetList))
							return true;
						// A cycle exists if we spot a list that is in our current path
						if (stack.find(targetList) != stack.end())
							return true;
					}
				}
				// Remove from current path
				stack.erase(a_listLoc);
				return false;
			};

	return _checkForCycles("root");
}
