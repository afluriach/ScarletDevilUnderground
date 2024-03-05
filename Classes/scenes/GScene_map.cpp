//
//  GScene_map.cpp
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "Door.hpp"
#include "functional.hpp"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GScene.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

const IntVec2 GScene::getRoomOffset(IntVec2 roomSize, int roomGridX, int roomGridY)
{
	return IntVec2(roomGridX * roomSize.first, roomGridY * roomSize.second);
}

SpaceVect GScene::getMapSize()
{
	return SpaceVect(dimensions.first, dimensions.second);
}

const vector<SpaceRect>& GScene::getMapAreas()
{
	return mapAreas;
}

const vector<bool>& GScene::getMapAreasVisited()
{
	return mapAreasVisited;
}

const vector<bool>& GScene::getAreasVisibleOnMap()
{
	return mapAreasVisibleOnMap;
}

int GScene::getMapLocation(SpaceRect r)
{
	return getAreaIndex(mapAreas, r);
}

bool GScene::isInCameraArea(SpaceRect r)
{
	return cameraArea.intersectsRect(r);
}

void GScene::eraseTile(int mapID, IntVec2 pos, string layer)
{
	TMXTiledMap* map = tilemaps.at(mapID);

	TMXLayer* _layer = map->getLayer(layer);

	_layer->setTileGID(0, Vec2(pos.first, pos.second));
}

void GScene::loadMaps()
{
	for(MapEntry entry : areaProps->maps)
	{
		loadMap(entry);
	}

	gspace->setSize(dimensions.first, dimensions.second);

	if (dimensions.first > 0 && dimensions.second > 0) {
		gspace->addWallBlock(SpaceVect(-1.0, 0.0), SpaceVect(0.0, dimensions.second));
		gspace->addWallBlock(SpaceVect(dimensions.first, 0.0), SpaceVect(dimensions.first + 1.0, dimensions.second));
		gspace->addWallBlock(SpaceVect(0.0, dimensions.second), SpaceVect(dimensions.first, dimensions.second + 1.0));
		gspace->addWallBlock(SpaceVect(0.0, -1.0), SpaceVect(dimensions.first, 0.0));
	}
}

void GScene::loadMap(const MapEntry& mapEntry)
{
	string mapResPath = "maps/" + mapEntry.first + ".tmx";
	TMXTiledMap* tileMap = nullptr;

	if (FileUtils::getInstance()->isFileExist(mapResPath)) {
		tileMap = TMXTiledMap::create(mapResPath);
	}

	if (tileMap) {
		log1("Map %s loaded.", mapResPath.c_str());
	}
	else {
		log1("Map %s not found.", mapResPath.c_str());
		return;
	}

	Vec2 llCorner = toCocos(mapEntry.second);
	CCSize mapSize = tileMap->getMapSize();
	SpaceRect mapRect(llCorner.x, llCorner.y, mapSize.width, mapSize.height);

	tilemaps.pushBack(tileMap);

	getSpaceLayer()->positionAndAddNode(
		tileMap,
		to_int(GraphicsLayer::map),
		llCorner * app::pixelsPerTile,
		1.0f
	);

	loadPaths(*tileMap, mapEntry.second);
	loadWaypoints(*tileMap, mapEntry.second);
	loadAreas(*tileMap, mapEntry.second);
	loadFloorSegments(*tileMap, mapEntry.second);
	loadPitfalls(*tileMap, mapEntry.second);
    loadDoors(*tileMap, mapEntry.second);
	loadSensors(*tileMap, mapEntry.second);
	loadMapObjects(*tileMap, mapEntry.second);
	loadDynamicLoadObjects(*tileMap, mapEntry.second);
	loadSubrooms(*tileMap, mapEntry.second);
	loadWalls(*tileMap, mapEntry.second);
	loadLights(*tileMap, mapEntry.second);

	cocos2d::CCSize size = tileMap->getMapSize();

	dimensions = IntVec2(
		max(dimensions.first, to_int(size.width) + mapEntry.second.first),
		max(dimensions.second, to_int(size.height) + mapEntry.second.second)
	);

	const ValueMap& props = tileMap->getProperties();
	string roomType = getStringOrDefault(props, "room_type", "");

	if (areaProps->maps.size() > 1) {
		mapAreas.push_back(mapRect);
		gspace->addMapArea(mapRect);
		mapAreasVisited.push_back(false);
		mapAreasVisibleOnMap.push_back(false);

		loadRoomFromMap(mapRect, tilemaps.size() - 1, props);
	}
	else if (areaProps->maps.size() == 1) {
		loadRoomsLayer(*tileMap);
	}
}

void GScene::loadDynamicLoadObjects(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* group = map.getObjectGroup("dynamic_load");
	if (!group) return;

	const ValueVector& objects = group->getObjects();
	for (const Value& obj : objects)
	{
		ValueMap objAsMap = obj.asValueMap();
		convertToUnitSpace(objAsMap, offset);
		gspace->addDynamicLoadObject(objAsMap);
	}
}

void GScene::loadMapObjects(const TMXTiledMap& map, IntVec2 offset)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    
    if(map.getObjectGroup("objects") == nullptr){
        log0("Objects group missing.");
    }
    else{
        loadObjectGroup(map.getObjectGroup("objects"), offset);
    }
}

const ValueVector& getPoints(const ValueMap& map, const string& name)
{
    auto it1 = map.find("polylinePoints");
    auto it2 = map.find("points");
    
    if(it1 != map.end())
        return it1->second.asValueVector();
    else if(it2 != map.end())
        return it2->second.asValueVector();
    else
        logAndThrowError("Error parsing path object %s!", name);
}

void GScene::loadPaths(const TMXTiledMap& map, IntVec2 offset)
{
	Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();

	if (!map.getObjectGroup("paths")) {
		return;
	}

	ValueVector paths = map.getObjectGroup("paths")->getObjects();

	for(const Value& value: paths)
	{
		Path crntPath;
		ValueMap asMap = value.asValueMap();

		string name = asMap.at("name").asString();
		const ValueVector& points = getPoints(asMap, name);
		SpaceVect origin(asMap.at("x").asFloat(), asMap.at("y").asFloat());

		for(auto const& point: points)
		{
			crntPath.push_back(SpaceVect(
				(origin.x + point.asValueMap().at("x").asFloat()) / app::pixelsPerTile + offset.first,
				(origin.y - point.asValueMap().at("y").asFloat()) / app::pixelsPerTile + offset.second
			));
		}
		gspace->addPath(name, crntPath);
	}
}

void GScene::loadWaypoints(const TMXTiledMap& map, IntVec2 offset)
{
	Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();

	if (!map.getObjectGroup("waypoints")) {
		return;
	}

	ValueVector waypoints = map.getObjectGroup("waypoints")->getObjects();

	for (const Value& value : waypoints)
	{
		ValueMap asMap = value.asValueMap();
		SpaceRect rect = getUnitspaceRectangle(asMap, offset);

		string name = asMap.at("name").asString();
		string _tagstr = getStringOrDefault(asMap, "tags", "");
		vector<string> tags = splitString(_tagstr, ",");

		gspace->addWaypoint(name, tags, rect.center);
	}
}

void GScene::loadAreas(const TMXTiledMap& map, IntVec2 offset)
{
	Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();

	if (!map.getObjectGroup("areas")) {
		return;
	}

	ValueVector waypoints = map.getObjectGroup("areas")->getObjects();

	for (const Value& value : waypoints)
	{
		ValueMap asMap = value.asValueMap();
		SpaceRect rect = getUnitspaceRectangle(asMap, offset);

		string name = asMap.at("name").asString();

		gspace->addArea(name, rect);
	}
}

void GScene::loadFloorSegments(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* floor = map.getObjectGroup("floor");
	if (!floor)
		return;

	for (const Value& obj : floor->getObjects())
	{
		ValueMap objAsMap = obj.asValueMap();
        string type = objAsMap.at("type").asString();
        auto props = app::getFloor(type);
        if(props){
            convertToUnitSpace(objAsMap, offset);
            gspace->createObject<FloorSegment>(objAsMap, props);
        }
        else{
            log1("Unknown floor type %s!", type);
        }
	}
}

void GScene::loadPitfalls(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* pitfalls = map.getObjectGroup("pitfalls");
	if (!pitfalls)
		return;

	for (const Value& obj : pitfalls->getObjects())
	{
		ValueMap objAsMap = obj.asValueMap();
		convertToUnitSpace(objAsMap, offset);
		gspace->createObject<Pitfall>(objAsMap);
	}
}

void GScene::loadSensors(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* sensors = map.getObjectGroup("sensors");
	if (!sensors)
		return;

	loadObjectGroup(sensors, offset);
}

void GScene::loadObjectGroup(TMXObjectGroup* group, IntVec2 offset)
{
	const ValueVector& objects = group->getObjects();

	for(const Value& obj: objects)
	{
		ValueMap objAsMap = obj.asValueMap();
		string _type = getStringOrDefault(objAsMap, "type", "");
		if (_type == "Player") {
			log0("warning: Player type map object");
			gspace->addWaypoint("player_start", getUnitspaceRectangle(objAsMap, offset).center);
		}
		else {
			convertToUnitSpace(objAsMap, offset);
			gspace->createObject(objAsMap);
		}
	}
}

void GScene::loadSubrooms(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* subrooms = map.getObjectGroup("subrooms");
	if (!subrooms)
		return;

	for (const Value& obj : subrooms->getObjects())
	{
		ValueMap objAsMap = obj.asValueMap();
		SpaceRect area = getUnitspaceRectangle(objAsMap, offset);

		objAsMap.insert_or_assign("id", to_int(graphicsContext->roomMasks.size()));
		convertToUnitSpace(objAsMap, offset);
		gspace->createObject<HiddenSubroomSensor>(objAsMap);

		DrawNode* dn = DrawNode::create();

		dn->drawSolidRect(
			toCocos(area.getLLCorner()) * app::pixelsPerTile,
			toCocos(area.getURCorner()) * app::pixelsPerTile,
			Color4F::BLACK
		);

		getSpaceLayer()->positionAndAddNode(
			dn,
			to_int(GraphicsLayer::roomMask),
			Vec2::ZERO,
			1.0f
		);

		graphicsContext->roomMasks.push_back(dn);
	}
}


void GScene::loadWalls(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* walls = map.getObjectGroup("walls");
	if (!walls)
		return;

	for(const Value& obj: walls->getObjects())
	{
		ValueMap objAsMap = obj.asValueMap();
		string type = objAsMap.at("type").asString();
		if(type.empty()) type = "Wall";
		auto props = app::getWall(type);

		if(props){
			convertToUnitSpace(objAsMap, offset);
			object_params params(objAsMap);
			gspace->createObject<Wall>(params, props);
		}
		else{
			log1("Unknown Wall type %s!", type);
		}
	}
}

void GScene::loadDoors(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* doors = map.getObjectGroup("doors");
	if (!doors)
		return;

	for(const Value& obj: doors->getObjects())
	{
		ValueMap objAsMap = obj.asValueMap();
		convertToUnitSpace(objAsMap, offset);
		gspace->createObject<Door>(objAsMap);
	}
}

void GScene::loadRoomFromMap(const SpaceRect& mapBounds, int roomID, const ValueMap& properties)
{
	gspace->createObject<RoomSensor>(
		SpaceRect(mapBounds.center, mapBounds.dimensions - SpaceVect(1.0, 1.0)),
		roomID,
		properties
	);
}

void GScene::loadRoomsLayer(const TMXTiledMap& map)
{
	TMXObjectGroup* rooms = map.getObjectGroup("rooms");
	if (!rooms)
		return;

	for (const Value& obj : rooms->getObjects())
	{
		ValueMap objAsMap = obj.asValueMap();
		SpaceRect area = getUnitspaceRectangle(objAsMap, make_pair(0,0));

		gspace->createObject<RoomSensor>(
			SpaceRect(area.center,area.dimensions - SpaceVect(1.0, 1.0)),
			objAsMap.at("name").asInt(),
			objAsMap
		);

		mapAreas.push_back(area);
		gspace->addMapArea(area);
		mapAreasVisited.push_back(false);
		mapAreasVisibleOnMap.push_back(false);
	}
}

void GScene::loadLights(const TMXTiledMap& map, IntVec2 offset)
{
	string ambient = getStringOrDefault(map.getProperties(), "ambient_light", "");
	Color4F color;
	
	if (!ambient.empty()) {
		color = toColor4F(toColor3B(ambient));
	}
	else {
		color = getDefaultAmbientLight();
	}

	SpriteID lightID = graphicsContext->getSpriteID();
	SpaceVect dimensions = toSpaceVect(map.getMapSize());
	SpaceVect center = toSpaceVect(offset) + dimensions / 2.0;

	graphicsContext->addPolyLightSource(
		lightID,
		AmbientLightArea::create( dimensions, color),
		center,
		0.0
	);
}

void GScene::updateMapVisibility(int roomID)
{
	updateRoomsVisited(roomID);
}

void GScene::updateMultimapVisibility(SpaceVect playerPos)
{
	for (int i = 0; i < tilemaps.size() && mapAreas.size(); ++i){
		tilemaps.at(i)->setVisible(
			isInCameraArea(mapAreas.at(i)) &&
			mapAreasVisited.at(i)
		);
	}
}

void GScene::updateRoomsVisited(int roomID)
{
	crntMap = roomID;
	mapAreasVisited.at(roomID) = true;
}

void GScene::setRoomDiscovered(size_t idx)
{
	mapAreasVisibleOnMap.at(idx) = true;
}

void GScene::setRoomsDiscovered(rooms_bitmask rooms)
{
	for_irange(i, 0, rooms.size()) {
		if (rooms[i]) setRoomDiscovered(i);
	}
}

void GScene::setRoomVisible(size_t idx)
{
	mapAreasVisited.at(idx) = true;
}

void GScene::setRoomsVisible(rooms_bitmask rooms)
{
	for_irange(i, 0, rooms.size()) {
		if (rooms[i]) setRoomVisible(i);
	}
}

void GScene::unlockAllRooms()
{
	for_irange(i, 0, mapAreasVisited.size())
	{
		mapAreasVisited.at(i) = true;
	}
}
