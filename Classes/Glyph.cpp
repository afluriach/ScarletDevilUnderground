//
//  Glyph.cpp
//  FlansBasement
//
//  Created by Toni on 10/28/15.
//
//

#include "Glyph.hpp"
#include "PlayScene.hpp"

void Glyph::initializeGraphics(cocos2d::Layer* layer)
{
    loadImageSprite("sprites/glyph.png", PlayScene::Layer::ground, layer);
}