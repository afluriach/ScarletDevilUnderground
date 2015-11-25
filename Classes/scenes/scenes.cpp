//
//  scenes.cpp
//  FlansBasement
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

#include "scenes.h"

GScene* GScene::crntScene;

void MapScene::loadObjectGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    gspace.addObjects(objects);
    //This isn't really necessary. If this is not added, nothing will happen the first frame
    //since there will be no objects, and all objects will be added at the end of the first frame.
    gspace.processAdditions();
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
    
    addChild(tileMap, GScene::Layer::map);
    loadMapObjects(*tileMap);
}

GSpace* GScene::getSpace()
{
    GSpaceScene* scene = dynamic_cast<GSpaceScene*>(crntScene);
    
    if(scene) return &(scene->gspace);
    else return nullptr;
}

