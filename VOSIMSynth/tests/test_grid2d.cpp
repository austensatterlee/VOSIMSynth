#include "Tests.h"
#include "vosimsynth/Grid.h"

using Point = synui::Grid2DPoint;
using Index = synui::Grid2DIndex;

TEST_CASE("Initialization", "[init]")
{
    synui::Grid2D<int> grid{ {2,2}, -1 };
    for (int i = 0; i < grid.getSize(); i++)
    {
        REQUIRE(grid.get(grid.unravel_index(i)) == -1);
    }
}

TEST_CASE("Resizing", "[resize]")
{
    synui::Grid2D<int> grid{ {2,2}, -1 };
    SECTION("Resize up")
    {
        grid.resize({ 3,3 });
        for (int i = 0; i < grid.getSize(); i++)
        {
            REQUIRE(grid.get(grid.unravel_index(i)) == -1);
        }
    }

    SECTION("Resize down")
    {
        grid.resize({ 3,3 });
        grid.resize({ 2,2 });
        for (int i = 0; i < grid.getSize(); i++)
        {
            REQUIRE(grid.get(grid.unravel_index(i)) == -1);
        }
    }

    SECTION("Preserves values")
    {
        grid.get({ 0,0 }) = 1;
        grid.get({ 1,1 }) = 2;
        grid.resize({ 3,3 });

        REQUIRE(grid.get({ 0,0 }) == 1);
        REQUIRE(grid.get({ 0,1 }) == -1);
        REQUIRE(grid.get({ 0,2 }) == -1);
        REQUIRE(grid.get({ 1,0 }) == -1);
        REQUIRE(grid.get({ 1,1 }) == 2);
        REQUIRE(grid.get({ 1,2 }) == -1);
        REQUIRE(grid.get({ 2,0 }) == -1);
        REQUIRE(grid.get({ 2,1 }) == -1);
        REQUIRE(grid.get({ 2,2 }) == -1);
    }
}

TEST_CASE("Replace values", "[assign]")
{
    synui::Grid2D<int> grid{ {2,2}, -1 };

    grid.replaceValue(-1, 1);
    REQUIRE(grid.get({ 0,0 }) == 1);
    REQUIRE(grid.get({ 0,1 }) == 1);
    REQUIRE(grid.get({ 1,0 }) == 1);
    REQUIRE(grid.get({ 1,1 }) == 1);

    grid.get({ 0,1 }) = 2;
    grid.get({ 1,0 }) = 2;

    grid.replaceValue(2, 3);
    REQUIRE(grid.get({ 0,0 }) == 1);
    REQUIRE(grid.get({ 0,1 }) == 3);
    REQUIRE(grid.get({ 1,0 }) == 3);
    REQUIRE(grid.get({ 1,1 }) == 1);
}

TEST_CASE("Set block", "[assign]")
{
    synui::Grid2D<int> grid{ {2,2}, -1 };

    REQUIRE(grid.setBlock({ 0,0 }, { 1,2 }, 1, true));
    REQUIRE(grid.get({ 0,0 }) == 1);
    REQUIRE(grid.get({ 0,1 }) == 1);
    REQUIRE(grid.get({ 1,0 }) == -1);
    REQUIRE(grid.get({ 1,1 }) == -1);

    REQUIRE(grid.setBlock({ 0,0 }, { 2,1 }, 2, false));
    REQUIRE(grid.get({ 0,0 }) == 2);
    REQUIRE(grid.get({ 0,1 }) == 1);
    REQUIRE(grid.get({ 1,0 }) == 2);
    REQUIRE(grid.get({ 1,1 }) == -1);

    REQUIRE(grid.setBlock({ 0,0 }, { 2,2 }, -1, false));
    REQUIRE(grid.get({ 0,0 }) == -1);
    REQUIRE(grid.get({ 0,1 }) == -1);
    REQUIRE(grid.get({ 1,0 }) == -1);
    REQUIRE(grid.get({ 1,1 }) == -1);

    REQUIRE(grid.setBlock({ 0,0 }, { 2,2 }, 3));
    REQUIRE(grid.get({ 0,0 }) == 3);
    REQUIRE(grid.get({ 0,1 }) == 3);
    REQUIRE(grid.get({ 1,0 }) == 3);
    REQUIRE(grid.get({ 1,1 }) == 3);

    REQUIRE(grid.setBlock({ 0,0 }, { 2,2 }, -1, false));
    REQUIRE(grid.setBlock({ 0,0 }, { 2,2 }, 3));
    REQUIRE_FALSE(grid.setBlock({ 0,0 }, { 2,2 }, -1));
}