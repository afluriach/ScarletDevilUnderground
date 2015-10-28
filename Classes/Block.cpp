//
//  Block.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Block.hpp"

void Block::initializeGraphics(cocos2d::Layer* layer)
{
    cp::Vect centerPix(initialCenter);
    centerPix *= App::pixelsPerTile;
    std::string resPath = "sprites/block "+letter+".png";
    cocos2d::Node* node = cocos2d::Sprite::create(resPath);
    node->setPosition(centerPix.x, centerPix.y);
    layer->addChild(node, PlayScene::Layer::ground);
    
    if(node == nullptr)
        log("%s sprite not loaded", name.c_str());
    else
        log("%s sprite %s added at %f,%f", name.c_str(), resPath.c_str(), centerPix.x, centerPix.y);
}
