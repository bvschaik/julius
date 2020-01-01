Bugs & idiosyncrasies
=====================

Julius recreates many bugs that you can find in original Caesar 3 game, to preserve 100% saves compatibility. This is an incomplete list of those bugs and some gameplay idiosyncrasies that you can encounter during the gameplay. Feel free to send a pull request with other bug reports that update this list.

## Warehouseman can't reach a warehouse with a road access

Each building in Caesar 3 is assigned to only one road network. That can result in a situation where a warehouseman cannot reach a given warehouse just because the warehouse has been assigned to another network than the building that's sending a warehouseman (see the image below). To avoid this problem make sure that all your warehouses are adjacent to only one road network.

![Bugs - two road networks](images/bugs_two_road_networks.png)

## Market trader doesn't take goods from the warehouse

Gardens split roads into separate road networks. Even though marker trader can use garden tiles as road tiles and pass by a given warehouse, that warehouse (even if full) might be ignored if it's assigned to another road network than market building (each building is assigned to a single road network in Caesar 3). To avoid this problem make sure that market traders traverse your city via road tiles rather than gardens.

![Bugs - two road networks](images/bugs_gardens.png)

## Market trader cannot access food from the warehouse

In original Caesar 3 game, market trader can get food only from granaries, never from warehouses (even though market traders get resources from a warehouse, such as pottery).
