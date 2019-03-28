//
//  MapFragment.cpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#include "Prefix.h"

#include "GScene.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "macros.h"
#include "MapFragment.hpp"
#include "value_map.hpp"

 bool MapFragment::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int mapFragmentId = getIntOrDefault(args, "id", -1);
	if (mapFragmentId < 0 || mapFragmentId >= maxMapFragmentsPerChamber)
		return false;

	return !space->getState()->chamberStats.at(to_size_t(space->getCrntChamber())).mapFragments[mapFragmentId];
}

MapFragment::MapFragment(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
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

	setInitialAngle(float_pi / 2.0);
}

void MapFragment::onAcquire()
{
	space->applyMapFragment(mapFragmentId);
	space->removeObject(this);
}