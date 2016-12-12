#include "Tests.h"
#include "MultiListBox.h"
#include <iostream>

TEST_CASE("[MultiListBox]")
{
	synui::MultiListBox::Ptr mlb = std::make_shared<synui::MultiListBox>();

	synui::MultiListBox::List listA{"itemA1","itemA2"};
	synui::MultiListBox::List listB{"itemB1","itemB2"};
	synui::MultiListBox::List listC{"itemC1","itemC2"};

	REQUIRE( mlb->addList("root", listA) );
	REQUIRE( mlb->addList("listB", listB) );
	REQUIRE( mlb->addList("listC", listC) );

	SECTION("Self cycle") {
		REQUIRE_FALSE( mlb->addEdge({"root","itemA1"}, "root") );
	}

	SECTION("Longer cycle") {
		REQUIRE( mlb->addEdge({"root","itemA1"}, "listB") );
		REQUIRE( mlb->addEdge({"root","itemA2"}, "listB") );

		REQUIRE( mlb->addEdge({"listB","itemB1"}, "listC") );

		REQUIRE_FALSE( mlb->addEdge({"listC","itemC2"}, "root") );
	}

	SECTION("Copy constructor")
	{
		synui::MultiListBox::Ptr mlb_copy = std::static_pointer_cast<synui::MultiListBox>(mlb->clone());

		REQUIRE(mlb_copy->getWidgets().size() == mlb->getWidgets().size());

		auto origWidgetNames = mlb->getWidgetNames();
		auto copyWidgetNames = mlb_copy->getWidgetNames();
		for(int i=0;i<origWidgetNames.size();i++){
			std::string orig_name = origWidgetNames[i];
			std::string copy_name = copyWidgetNames[i];
			REQUIRE(orig_name == copy_name);
		}
	}
}
