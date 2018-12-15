//
//  Pyramid.cpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#include "Prefix.h"

#include "Pyramid.hpp"
#include "SpaceLayer.h"

Pyramid::Pyramid(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject)
{
}

PhysicsLayers Pyramid::getLayers() const{
    return PhysicsLayers::ground;
}

void Pyramid::initializeGraphics(SpaceLayer* layer)
{
	Sprite* s = Sprite::create(imageSpritePath());

	layer->getLayer(GraphicsLayer::ground)->positionAndAddNode(
		s,
		1,
		getInitialCenterPix(),
		1.0f
	);

	sprite = s;
}