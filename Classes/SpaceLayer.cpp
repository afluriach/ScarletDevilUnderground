//
//  SpaceLayer.cpp
//  Koumachika
//
//  Created by Toni on 12/12/18.
//
//

#include "Prefix.h"

#include "macros.h"
#include "SpaceLayer.h"

SpaceLayer::SpaceLayer()
{
}

bool SpaceLayer::init()
{
	Layer::init();

	for_irange(i, 1, GraphicsLayer::end) {
		Layer* l = Layer::create();
		layers.insert(i, l);
		addChild(l,i);
	}

	return true;
}

Layer* SpaceLayer::getLayer(GraphicsLayer layer)
{
	auto it = layers.find(to_int(layer));
	if (it == layers.end()) return nullptr;
	return it->second;
}
