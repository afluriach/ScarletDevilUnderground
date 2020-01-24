//
//  MapFragment.cpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#include "Prefix.h"

#include "GScene.hpp"
#include "MapFragment.hpp"
#include "value_map.hpp"

 bool MapFragment::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int mapFragmentId = getIntOrDefault(args, "id", -1);
	if (mapFragmentId < 0 || mapFragmentId >= maxMapFragmentsPerChamber)
		return false;

	ChamberStats& stats = space->getCrntChamberStats();
	return !stats.mapFragments[mapFragmentId];
}

MapFragment::MapFragment(GSpace* space, ObjectIDType id, const ValueMap& args) :
	InventoryObject(MapParamsPointUp(), physics_params(GType::playerPickup, PhysicsLayers::ground, 0.5, -1.0, true)),
	mapFragmentId(getIntOrDefault(args, "id", -1))
{
	if (mapFragmentId == -1) {
		log("MapFragment: unknown ID");
	}
	else if (space->getScene()->getMapFragmentsList().empty()) {
		log("MapFragment: Scene does not have map fragment info!");
	}
	else if (mapFragmentId >= space->getScene()->getMapFragmentsList().size()) {
		log("MapFragment: invalid ID");
	}
}

void MapFragment::onAcquire(Player* player)
{
	space->applyMapFragment(mapFragmentId);
	space->removeObject(this);
}
