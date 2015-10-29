//
//  Block.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Block.hpp"
#include "util.h"

void Block::initializeGraphics(cocos2d::Layer* layer)
{
    std::string resPath = "sprites/block "+letter+".png";
    loadImageSprite(resPath, PlayScene::Layer::ground, layer);
}
