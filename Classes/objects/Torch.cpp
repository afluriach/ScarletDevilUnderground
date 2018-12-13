//
//  Torch.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "GAnimation.hpp"
#include "SpaceLayer.h"
#include "Torch.hpp"

void Torch::initializeGraphics(SpaceLayer* layer)
{
//    Sprite* baseSprite = ::loadImageSprite(
//        "sprites/blue_torch.png",
//        sceneLayer(),
//        layer,
//        getInitialCenterPix(),
//        4.0
//    );

    Sprite* base = Sprite::create("sprites/blue_torch.png");
    
    flame = Node::ccCreate<TimedLoopAnimation>();
    flame->loadAnimation("blue_flame", 8, 1.0);
    
    sprite = Node::create();
    sprite->addChild(base, 1);
    sprite->addChild(flame, 2);
    
    layer->getLayer(sceneLayer())->positionAndAddNode(sprite, 1, getInitialCenterPix(), 4.0);
}

void Torch::setActive(bool active)
{
    isActive = active;
    
    flame->setOpacity(active ? 255 : 0);
}

bool Torch::getActive()
{
    return isActive;
}

void Torch::update()
{
    flame->update();
}
