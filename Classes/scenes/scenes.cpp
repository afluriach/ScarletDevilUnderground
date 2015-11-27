//
//  scenes.cpp
//  FlansBasement
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

GScene* GScene::crntScene;

void MapScene::loadObjectGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    gspace.addObjects(objects);
}

void MapScene::loadMapObjects(const TMXTiledMap& map)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    
    if(map.getObjectGroup("objects") == nullptr){
        log("Objects group missing.");
    }
    else{
        loadObjectGroup(map.getObjectGroup("objects"));
    }
}

void MapScene::loadMap()
{
    tileMap = TMXTiledMap::create(mapRes);
    
    if(tileMap)
        log("Map %s loaded.", mapRes.c_str());
    else
        log("Map %s not found.", mapRes.c_str());
    
    addChild(tileMap, GraphicsLayer::map);
    loadMapObjects(*tileMap);
}

GSpace* GScene::getSpace()
{
    GSpaceScene* scene = dynamic_cast<GSpaceScene*>(crntScene);
    
    if(scene) return &(scene->gspace);
    else return nullptr;
}

void GScene::move(const Vec2& v)
{
    setPosition(getPositionX()-v.x, getPositionY()-v.y);
}

void GScene::setUnitPosition(const SpaceVect& v)
{
    setPosition(-App::pixelsPerTile*v.x+App::width/2, -App::pixelsPerTile*v.y+App::height/2);
}
