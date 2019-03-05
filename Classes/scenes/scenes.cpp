//
//  scenes.cpp
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"


#include "App.h"
#include "AreaSensor.hpp"
#include "controls.h"
#include "Dialog.hpp"
#include "FileIO.hpp"
#include "functional.hpp"
#include "GAnimation.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "menu.h"
#include "PlayScene.hpp"
#include "scenes.h"
#include "types.h"
#include "value_map.hpp"

const int GScene::dialogEdgeMargin = 30;
const bool GScene::scriptLog = false;

string GScene::crntSceneName;
string GScene::crntReplayName;
bool GScene::suppressGameOver = false;

GScene* GScene::runScene(const string& name)
{
    auto it = adapters.find(name);
    
    if(it == adapters.end()){
        log("runScene: %s not found", name.c_str());
		return nullptr;
    }
    else
    {
		crntSceneName = name;
		return it->second();
    }
}

void GScene::runSceneWithReplay(const string& replayName)
{
	unique_ptr<ControlReplay> controlReplay = io::getControlReplay(replayName);

	if (controlReplay) {
		GScene* _scene = runScene(controlReplay->scene_name);
		PlayScene* ps = dynamic_cast<PlayScene*>(_scene);

		if (ps) {
			ps->loadReplayData(move(controlReplay));
			crntReplayName = replayName;
		}
	}
}

void GScene::restartScene()
{
	runScene(crntSceneName);
}

void GScene::restartReplayScene()
{
	runSceneWithReplay(crntReplayName);
}

vector<GScene::MapEntry> GScene::singleMapEntry(const string& mapName)
{
	return {
		{mapName, IntVec2(0,0)}
	};
}

const IntVec2 GScene::getRoomOffset(IntVec2 roomSize, int roomGridX, int roomGridY)
{
	return IntVec2(roomGridX * roomSize.first, roomGridY * roomSize.second);
}

GScene::GScene(const string& sceneName, const vector<MapEntry>& maps) :
maps(maps),
sceneName(sceneName),
ctx(make_unique<Lua::Inst>("scene")),
control_listener(make_unique<ControlListener>())
{
	
	multiInit.insertWithOrder(
		wrap_method(GScene, installLuaShell, this),
		to_int(initOrder::core)
	);
	//Updater has to be scheduled at init time.
    multiInit.insertWithOrder(
        wrap_method(Node,scheduleUpdate,this),
        to_int(initOrder::core)
    );
	multiInit.insertWithOrder(
		wrap_method(GScene, loadMaps, this),
		to_int(initOrder::mapLoad)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, processAdditions, this),
		to_int(initOrder::loadObjects)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, runScriptInit, this),
		to_int(initOrder::postLoadObjects)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, initEnemyStats, this),
		to_int(initOrder::postLoadObjects)
	);

	multiUpdate.insertWithOrder(
		wrap_method(GScene, queueActions, this),
		to_int(updateOrder::queueActions)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, checkPendingScript, this),
		to_int(updateOrder::runShellScript)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, runScriptUpdate, this),
		to_int(updateOrder::sceneUpdate)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::spriteUpdate),
		to_int(updateOrder::spriteUpdate)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::lightmapUpdate),
		to_int(updateOrder::lightmapUpdate)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, renderSpace, this),
		to_int(updateOrder::renderSpace)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::sceneUpdate),
		to_int(updateOrder::sceneUpdate)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::hudUpdate),
		to_int(updateOrder::hudUpdate)
	);

	enum_foreach(updateOrder, order, begin, end)
	{
		actions.insert_or_assign(order, vector<zero_arity_function>());
	}

    //Create the sublayers at construction (so they are available to mixins at construction time).
    //But do not add sublayers until init time.
    for_irange(i,sceneLayers::begin,sceneLayers::end){
        Layer* l = Layer::create();
        layers.insert(i, l);
    }

	isExit = false;
	isPaused = false;

	gspace = new GSpace(this);

	if (!sceneName.empty())
	{
		string scriptPath = "scripts/scenes/" + sceneName + ".lua";

		if (!FileUtils::getInstance()->isFileExist(scriptPath) && scriptLog) {
			log("GScene: %s script does not exist.", sceneName.c_str());
		}
		else {
			ctx->runFile(scriptPath);
		}
	}

	control_listener->addPressListener(ControlAction::displayMode, bind(&GScene::cycleDisplayMode, this));
}

GScene::~GScene()
{
	isExit.store(true);
	spaceUpdateCondition.notify_one();
	spaceUpdateThread->join();

	delete gspace;
}

bool GScene::init()
{
    Scene::init();

	lightmapRender = initRenderTexture(sceneLayers::lightmap, BlendFunc{ GL_DST_COLOR,GL_ONE_MINUS_SRC_ALPHA });
	colorFilterRender = initRenderTexture(sceneLayers::screenspaceColorFilter, BlendFunc{ GL_DST_COLOR,GL_ONE_MINUS_SRC_ALPHA });
	spaceRender = initRenderTexture(sceneLayers::space);

	lightmapDrawNode = DrawNode::create();
	lightmapDrawNode->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	getLayer(sceneLayers::lightmap)->addChild(lightmapDrawNode);

	lightmapBackground = DrawNode::create();
	getLayer(sceneLayers::lightmapBackground)->addChild(lightmapBackground);
	lightmapBackground->setVisible(false);
	lightmapBackground->drawSolidRect(Vec2::ZERO, Vec2(App::width, App::height), Color4F::WHITE);

	colorFilterDraw = DrawNode::create();
	setColorFilter(Color4F::WHITE);
	getLayer(sceneLayers::screenspaceColorFilter)->addChild(colorFilterDraw);

	//Apply zoom to adjust viewable area size.
	float baseViewWidth = App::width * App::tilesPerPixel;
	spaceZoom = baseViewWidth / App::viewWidth;

	getLayer(sceneLayers::space)->setScale(spaceZoom);
	getLayer(sceneLayers::lightmap)->setScale(spaceZoom);

    for_irange(i,to_int(sceneLayers::begin),sceneLayers::end){
		if (i != to_int(sceneLayers::space) && i != to_int(sceneLayers::lightmap) && i != to_int(sceneLayers::screenspaceColorFilter)) {
			addChild(layers.at(i), i);
		}
		else {
			layers.at(i)->setActionManager(Director::getInstance()->getActionManager());
			layers.at(i)->setEventDispatcher(Director::getInstance()->getEventDispatcher());
			layers.at(i)->setScheduler(Director::getInstance()->getScheduler());
		}
	}
    
    multiInit();
    
	spaceUpdateToRun.store(false);
	spaceUpdateThread = make_unique<thread>(&GScene::spaceUpdateMain, this);

    return true;
}

void GScene::update(float dt)
{	
	if (!isPaused) {
		gspace->addObjectAction(bind(
			&GSpace::setControlInfo,
			gspace,
			App::control_register->getControlInfo()
		));
		spaceUpdateToRun.store(true);
		spaceUpdateCondition.notify_one();
		multiUpdate();
	}
}

void GScene::onExit()
{
	while (!menuStack.empty()) popMenu();

	while (spaceUpdateToRun.load()) {
		unique_lock<mutex> mlock(spaceUpdateConditionMutex);
		spaceUpdateCondition.wait(
			mlock,
			[this]() -> bool {return !spaceUpdateToRun.load();
		});
	}

	Node::onExit();
}

GScene* GScene::getReplacementScene()
{
	return Node::ccCreate<GScene>(sceneName, maps);
}

GSpace* GScene::getSpace()
{
	return gspace;
}

void GScene::setPaused(bool p){
    isPaused = p;
}

void GScene::createDialog(const string& res, bool autoAdvance)
{
	createDialog(res, autoAdvance, nullptr);
}

void GScene::createDialog(const string& res, bool autoAdvance, zero_arity_function f)
{
	stopDialog();

	dialog = Node::ccCreate<Dialog>();
	dialog->setDialog(res);
	dialog->setPosition(dialogPosition());
	getLayer(sceneLayers::dialog)->addChild(dialog);

	//This options are not actually mutually exclusive, but for simplicity just use a flag
	//to choose one.
	dialog->setAutoAdvance(autoAdvance);
	dialog->setManualAdvance(!autoAdvance);

	isPaused = !autoAdvance;

	dialog->setEndHandler([this, f]() -> void {
		stopDialog();

		if (f) {
			gspace->addObjectAction(f);
		}
	});
}

void GScene::stopDialog()
{
	if (dialog) {
		getLayer(sceneLayers::dialog)->removeChild(dialog);
		dialog = nullptr;
		isPaused = false;
	}
}

Vec2 GScene::dialogPosition()
{
    return Vec2(App::width/2, Dialog::height/2 + dialogEdgeMargin);
}

bool GScene::isDialogActive()
{
	return dialog != nullptr;
}

void GScene::processAdditions()
{
	gspace->processAdditions();
}

void GScene::addActions(const vector<pair<zero_arity_function, updateOrder>>& _actions)
{
	actionsMutex.lock();

	for (auto entry : _actions) {
		actionsToAdd.push_back(entry);
	}

	actionsMutex.unlock();
}

SpriteID GScene::getSpriteID()
{
	return nextSpriteID.fetch_add(1);
}

LightID GScene::getLightID()
{
	return nextLightID.fetch_add(1);
}

void GScene::addLightSource(LightID id, CircleLightArea light)
{
	RadialGradient* g = Node::ccCreate<RadialGradient>(
		light.color,
		Color4F::BLACK,
		light.radius * App::pixelsPerTile,
		Vec2::ZERO,
		light.flood
	);

	g->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	g->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	g->setContentSize(CCSize(light.radius * 2.0f * App::pixelsPerTile, light.radius * 2.0f * App::pixelsPerTile));
	getLayer(sceneLayers::lightmap)->addChild(g);

	circleLights.insert_or_assign(id,light);
	lightmapRadials.insert_or_assign(id, g);
}

void GScene::addLightSource(LightID id, AmbientLightArea light)
{
	ambientLights.insert_or_assign(id,light);
}

void GScene::addLightSource(LightID id, ConeLightArea light)
{
	ConeShader* cs = Node::ccCreate<ConeShader>(
		light.color,
		light.radius * App::pixelsPerTile,
		Vec2::ZERO,
		light.startAngle,
		light.endAngle
	);

	cs->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	cs->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	cs->setContentSize(CCSize(light.radius * 2.0f * App::pixelsPerTile, light.radius * 2.0f * App::pixelsPerTile));
	getLayer(sceneLayers::lightmap)->addChild(cs);

	coneLights.insert_or_assign(id, light);
	lightmapCones.insert_or_assign(id, cs);
}

void GScene::addLightSource(LightID id, SpriteLightArea light)
{
	Sprite* s = Sprite::create(light.texName);

	s->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	s->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	s->setScale(light.scale);
	s->setColor(toColor3B(light.color));
	getLayer(sceneLayers::lightmap)->addChild(s);

	spriteLights.insert_or_assign(id, light);
	lightmapSprites.insert_or_assign(id, s);
}

void GScene::updateLightSource(LightID id, ConeLightArea light)
{
	coneLights.insert_or_assign(id, light);

	auto it = lightmapCones.find(id);
	if (it != lightmapCones.end()) {
		it->second->setPosition(toCocos(light.origin) * App::pixelsPerTile);
		it->second->setAngles(light.startAngle, light.endAngle);
		it->second->setColor(light.color);
	}
}

void GScene::removeLightSource(LightID id)
{
	auto it = lightmapRadials.find(id);
	if (it != lightmapRadials.end()) {
		getLayer(sceneLayers::lightmap)->removeChild(it->second);
		lightmapRadials.erase(it);
	}

	auto it1 = lightmapSprites.find(id);
	if (it1 != lightmapSprites.end()) {
		getLayer(sceneLayers::lightmap)->removeChild(it1->second);
		lightmapSprites.erase(it1);
	}

	auto it2 = lightmapCones.find(id);
	if (it2 != lightmapCones.end()) {
		getLayer(sceneLayers::lightmap)->removeChild(it2->second);
		lightmapCones.erase(it2);
	}

	circleLights.erase(id);
	ambientLights.erase(id);
	coneLights.erase(id);
	spriteLights.erase(id);
}

void GScene::setLightSourcePosition(LightID id, SpaceVect pos)
{
	{
		auto it = circleLights.find(id);
		if (it != circleLights.end()) {
			it->second.origin = pos;
			lightmapRadials.at(it->first)->setPosition(toCocos(pos) * App::pixelsPerTile);
		}
	}
	{
		auto it = coneLights.find(id);
		if (it != coneLights.end()) {
			it->second.origin = pos;
			lightmapCones.at(it->first)->setPosition(toCocos(pos) * App::pixelsPerTile);
		}
	}
	{
		auto it = spriteLights.find(id);
		if (it != spriteLights.end()) {
			it->second.origin = pos;
			lightmapSprites.at(it->first)->setPosition(toCocos(pos) * App::pixelsPerTile);
		}
	}
	{
		auto it = ambientLights.find(id);
		if (it != ambientLights.end()) {
			it->second.origin = pos;
		}
	}
}

void GScene::setLightSourceAngle(LightID id, SpaceFloat a)
{
	{
		auto it = spriteLights.find(id);
		if (it != spriteLights.end()) {
			lightmapSprites.at(it->first)->setRotation(90 - toDegrees(a));
		}
	}
}

void GScene::createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	Sprite* s = Sprite::create(path);
	getSpaceLayer()->positionAndAddNode(s, to_int(sceneLayer), pos, zoom);
	crntSprites.insert_or_assign(id, s);
}

void GScene::createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	TimedLoopAnimation* anim = Node::ccCreate<TimedLoopAnimation>();
	anim->loadAnimation(name, frameCount, duration);
	getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
	animationSprites.insert_or_assign(id, anim);
}

void GScene::createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	DrawNode* dn = DrawNode::create();
	drawNodes.insert_or_assign(id, dn);
	getSpaceLayer()->positionAndAddNode(dn, to_int(sceneLayer), pos, zoom);
}

void GScene::createAgentSprite(SpriteID id, string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	PatchConAnimation* anim = Node::ccCreate<PatchConAnimation>();
	anim->loadAnimation(path, isAgentAnimation);
	agentSprites.insert_or_assign(id, anim);
	getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
}

void GScene::createDamageIndicator(float val, SpaceVect pos)
{
	float roundedVal = floor(val * 4.0f) / 4.0f;

	Label* label = createTextLabel(boost::lexical_cast<string>(roundedVal), 18);
	Vec2 startPos = toCocos(pos + SpaceVect(0.0, 0.5)) * App::pixelsPerTile;
	label->setPosition(startPos);
	label->setScale(2.0f);

	getSpaceLayer()->addChild(label, to_uint(GraphicsLayer::agentOverlay));
	label->runAction(damageIndicatorAction(startPos)());
}

void GScene::loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->loadAnimation(path, isAgentAnimation);
	}
}

void GScene::setAgentAnimationDirection(SpriteID id, Direction d)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->setDirection(d);
	}
}

void GScene::setAgentAnimationFrame(SpriteID id, int frame)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->setFrame(frame);
	}
}

void GScene::clearDrawNode(SpriteID id)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->clear();
	}
}

void GScene::drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidRect(lowerLeft, upperRight, color);
	}
}

void GScene::drawSolidCone(SpriteID id, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, const Color4F &color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidCone(center, radius, startAngle, endAngle, segments, color);
	}
}

void GScene::drawSolidCircle(SpriteID id, const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidCircle(center, radius, angle, segments, color);
	}
}

void GScene::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(generator());
	}
}

void GScene::stopSpriteAction(SpriteID id, cocos_action_tag action)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->stopActionByTag(to_int(action));
	}
}

void GScene::stopAllSpriteActions(SpriteID id)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->stopAllActions();
	}
}

void GScene::removeSprite(SpriteID id)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->removeFromParent();
		_removeSprite(id);
	}
}

void GScene::removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(Sequence::createWithTwoActions(generator(), RemoveSelf::create()));
	}
}

void GScene::setSpriteVisible(SpriteID id, bool val)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setVisible(val);
	}
}

void GScene::setSpriteOpacity(SpriteID id, unsigned char op)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setOpacity(op);
	}
}

void GScene::setSpriteTexture(SpriteID id, string path)
{
	auto it = crntSprites.find(id);
	if (it != crntSprites.end()) {
		Sprite* s = dynamic_cast<Sprite*>(it->second);
		if (s) {
			s->setTexture(path);
		}
	}
}

void GScene::setSpriteAngle(SpriteID id, float cocosAngle)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setRotation(cocosAngle);
	}
}

void GScene::setSpritePosition(SpriteID id, Vec2 pos)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setPosition(pos);
	}
}

void GScene::setSpriteZoom(SpriteID id, float zoom)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setScale(zoom);
	}
}

void GScene::setSpriteColor(SpriteID id, Color3B color)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setColorRecursive(color);
	}
}

void GScene::clearSubroomMask(unsigned int roomID)
{
	roomMasks.at(roomID)->setVisible(false);
}

Node* GScene::getSpriteAsNode(SpriteID id)
{
	{
		auto it = crntSprites.find(id);
		if (it != crntSprites.end()) {
			return it->second;
		}
	}
	{
		auto it = drawNodes.find(id);
		if (it != drawNodes.end()) {
			return it->second;
		}
	}
	{
		auto it = animationSprites.find(id);
		if (it != animationSprites.end()) {
			return it->second;
		}
	}
	{
		auto it = agentSprites.find(id);
		if (it != agentSprites.end()) {
			return it->second;
		}
	}
	return nullptr;
}

void GScene::_removeSprite(SpriteID id)
{
	crntSprites.erase(id);
	drawNodes.erase(id);
	animationSprites.erase(id);
	agentSprites.erase(id);
}

void GScene::setUnitPosition(const SpaceVect& v)
{
	cameraArea = calculateCameraArea(v);
		
	getSpaceLayer()->setPosition(
		(-App::pixelsPerTile*v.x + App::width / 2)*spaceZoom - App::width/2,
		(-App::pixelsPerTile*v.y + App::height / 2)*spaceZoom - App::height/2
	);
	getLayer(sceneLayers::lightmap)->setPosition(
		(-App::pixelsPerTile*v.x + App::width / 2)*spaceZoom - App::width/2,
		(-App::pixelsPerTile*v.y + App::height / 2)*spaceZoom - App::height/2
	);
}

SpaceVect GScene::getMapSize()
{
	return SpaceVect(dimensions.first, dimensions.second);
}

SpaceRect GScene::getCameraArea()
{
	return cameraArea;
}

const vector<SpaceRect>& GScene::getMapAreas()
{
	return mapAreas;
}

const vector<bool>& GScene::getMapAreasVisited()
{
	return mapAreasVisited;
}

int GScene::getMapLocation(SpaceRect r)
{
	return getAreaIndex(mapAreas, r);
}

bool GScene::isInCameraArea(SpaceRect r)
{
	return cameraArea.intersectsRect(r);
}

bool GScene::isInPlayerRoom(SpaceVect v)
{
	return isInArea(mapAreas, v, crntMap);
}

int GScene::getPlayerRoom()
{
	return crntMap;
}

void GScene::eraseTile(int mapID, IntVec2 pos, string layer)
{
	TMXTiledMap* map = tilemaps.at(mapID);

	TMXLayer* _layer = map->getLayer(layer);

	_layer->setTileGID(0, Vec2(pos.first, pos.second));
}

void GScene::teleportToDoor(string name)
{
	gspace->addObjectAction([=]()->void {
		gspace->teleportPlayerToDoor(name);
	});
}

Layer* GScene::getLayer(sceneLayers layer)
{
	auto it = layers.find(to_int(layer));
	if (it == layers.end()) return nullptr;
	return it->second;
}

void GScene::pushMenu(MenuLayer* layer)
{
	if (!menuStack.empty()) {
		menuStack.back()->setVisible(false);
		menuStack.back()->setControlsActive(false);
	}

	menuStack.push_back(layer);
	getLayer(sceneLayers::menu)->addChild(layer);
}

void GScene::popMenu()
{
	if (!menuStack.empty()) {
		Layer* l = menuStack.back();
		getLayer(sceneLayers::menu)->removeChild(l);
		menuStack.pop_back();
	}

	if (!menuStack.empty()) {
		menuStack.back()->setVisible(true);
		menuStack.back()->setControlsActive(true);
	}
}

void GScene::popMenuIfNonroot()
{
	if (menuStack.size() > 1) {
		Layer* l = menuStack.back();
		getLayer(sceneLayers::menu)->removeChild(l);
		menuStack.pop_back();
	}

	menuStack.back()->setVisible(true);
	menuStack.back()->setControlsActive(true);
}

void GScene::loadMaps()
{
	for(MapEntry entry : maps)
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
		log("Map %s loaded.", mapResPath.c_str());
	}
	else {
		log("Map %s not found.", mapResPath.c_str());
		return;
	}

	Vec2 llCorner = toCocos(mapEntry.second);
	CCSize mapSize = tileMap->getMapSize();
	SpaceRect mapRect(llCorner.x, llCorner.y, mapSize.width, mapSize.height);

	tilemaps.pushBack(tileMap);

	getSpaceLayer()->positionAndAddNode(
		tileMap,
		to_int(GraphicsLayer::map),
		llCorner * App::pixelsPerTile,
		1.0f
	);

	loadPaths(*tileMap, mapEntry.second);
	loadWaypoints(*tileMap, mapEntry.second);
	loadFloorSegments(*tileMap, mapEntry.second);
	loadMapObjects(*tileMap, mapEntry.second);
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

	if (maps.size() > 1) {
		mapAreas.push_back(mapRect);
		gspace->addMapArea(mapRect);
		mapAreasVisited.push_back(false);

		loadRoomFromMap(mapRect, tilemaps.size() - 1, props);
	}
	else if (maps.size() == 1) {
		loadRoomsLayer(*tileMap);
	}
}

void GScene::loadMapObjects(const TMXTiledMap& map, IntVec2 offset)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    
    if(map.getObjectGroup("objects") == nullptr){
        log("Objects group missing.");
    }
    else{
        loadObjectGroup(map.getObjectGroup("objects"), offset);
    }
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
		ValueVector points = asMap.at("polylinePoints").asValueVector();
		SpaceVect origin(asMap.at("x").asFloat(), asMap.at("y").asFloat());

		for(auto const& point: points)
		{
			crntPath.push_back(SpaceVect(
				(origin.x + point.asValueMap().at("x").asFloat()) / App::pixelsPerTile + offset.first,
				(origin.y - point.asValueMap().at("y").asFloat()) / App::pixelsPerTile + offset.second
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

		gspace->addWaypoint(name, rect.center);
	}
}

void GScene::loadFloorSegments(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* floor = map.getObjectGroup("floor");
	if (!floor)
		return;

	loadObjectGroup(map.getObjectGroup("floor"), offset);
}

void GScene::loadObjectGroup(TMXObjectGroup* group, IntVec2 offset)
{
	const ValueVector& objects = group->getObjects();

	for(const Value& obj: objects)
	{
		ValueMap objAsMap = obj.asValueMap();
		convertToUnitSpace(objAsMap, offset);
		gspace->createObject(objAsMap);
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

		objAsMap.insert_or_assign("type", "HiddenSubroomSensor");
		objAsMap.insert_or_assign("id", to_int(roomMasks.size()));

		convertToUnitSpace(objAsMap, offset);
		gspace->createObject(objAsMap);

		DrawNode* dn = DrawNode::create();

		dn->drawSolidRect(
			toCocos(area.getLLCorner()) * App::pixelsPerTile,
			toCocos(area.getURCorner()) * App::pixelsPerTile,
			Color4F::BLACK
		);

		getSpaceLayer()->positionAndAddNode(
			dn,
			to_int(GraphicsLayer::roomMask),
			Vec2::ZERO,
			1.0f
		);

		roomMasks.push_back(dn);
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
		string _type = objAsMap.at("type").asString();

		if (_type.empty()) {
			objAsMap.insert_or_assign("type", "Wall");
		}

		convertToUnitSpace(objAsMap, offset);
		gspace->createObject(objAsMap);
	}
}

void GScene::loadRoomFromMap(const SpaceRect& mapBounds, int roomID, const ValueMap& properties)
{
	gspace->createObject(GObject::make_object_factory<RoomSensor>(
		mapBounds.center,
		mapBounds.dimensions,
		roomID,
		properties
	));
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
		convertToUnitSpace(objAsMap, make_pair(0,0));

		gspace->createObject(GObject::make_object_factory<RoomSensor>(objAsMap));

		mapAreas.push_back(area);
		gspace->addMapArea(area);
		mapAreasVisited.push_back(false);
	}
}

void GScene::loadLights(const TMXTiledMap& map, IntVec2 offset)
{
	string ambient = getStringOrDefault(map.getPropertiesConst(), "ambient_light", "");
	Color4F color;
	
	if (!ambient.empty()) {
		color = toColor4F(toColor3B(ambient));
	}
	else {
		color = getDefaultAmbientLight();
	}

	LightID id = getLightID();

	SpaceVect dimensions = toChipmunk(map.getMapSize());
	SpaceVect center = toChipmunk(offset) + dimensions / 2.0;

	addLightSource(id, AmbientLightArea{ center, dimensions, color});
	ambientMapLights.push_back(id);
}

void GScene::initEnemyStats()
{
	gspace->setInitialObjectCount();
}

void GScene::spaceUpdateMain()
{
	while (!isExit)
	{
		unique_lock<mutex> mlock(spaceUpdateConditionMutex);
		spaceUpdateCondition.wait(
			mlock,
			[this]() -> bool { return isExit.load() || spaceUpdateToRun.load(); }
		);

		if (!isExit) {
			gspace->update();
			spaceUpdateToRun.store(false);
			spaceUpdateCondition.notify_one();
		}
	}
}

void GScene::updateMapVisibility(SpaceVect playerPos)
{
	if (tilemaps.size() > 1) {
		updateMultimapVisibility(playerPos);
	}
	updateRoomsVisited(playerPos);
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

void GScene::updateRoomsVisited(SpaceVect playerPos)
{
	for (int i = 0; i < mapAreas.size(); ++i) {
		if (mapAreas.at(i).containsPoint(playerPos)) {
			crntMap = i;
			mapAreasVisited.at(i) = true;
		}
	}
}

void GScene::unlockAllRooms()
{
	for_irange(i, 0, mapAreasVisited.size())
	{
		mapAreasVisited.at(i) = true;
	}
}

void GScene::renderSpace()
{
	spaceRender->setVisible(display != displayMode::lightmap);
	lightmapRender->setVisible(display != displayMode::base);
	lightmapBackground->setVisible(display == displayMode::lightmap);

	for (TimedLoopAnimation* anim : animationSprites | boost::adaptors::map_values) {
		anim->update();
	}

	redrawLightmap();
}

void GScene::redrawLightmap()
{
	lightmapDrawNode->clear();

	CCRect cameraPix = getCameraArea().toPixelspace();

	if (display == displayMode::lightmap) {
		lightmapDrawNode->drawSolidRect(
			cameraPix.getLowerCorner(),
			cameraPix.getUpperCorner(),
			Color4F::BLACK
		);
	}

	for (pair<unsigned int,CircleLightArea> lightEntry : circleLights)
	{
		Vec2 originPix = toCocos(lightEntry.second.origin) * App::pixelsPerTile;
		float radiusPix = lightEntry.second.radius * App::pixelsPerTile;

		lightmapRadials.at(lightEntry.first)->setVisible(
			cameraPix.intersectsCircle(originPix, radiusPix)
		);
	}

	for (AmbientLightArea light : ambientLights | boost::adaptors::map_values)
	{
		Color4F color = light.color;
		Vec2 halfDim = toCocos(light.dimensions) / 2.0f * App::pixelsPerTile;
		Vec2 center = toCocos(light.origin) * App::pixelsPerTile;

		if (cameraPix.intersectsRect(CCRect(center.x - halfDim.x, center.y - halfDim.y, halfDim.x * 2.0f, halfDim.y * 2.0f))) {
			lightmapDrawNode->drawSolidRect(center - halfDim, center + halfDim, color);
		}
	}

	for (pair<unsigned int, ConeLightArea> lightEntry : coneLights)
	{
		Vec2 originPix = toCocos(lightEntry.second.origin) * App::pixelsPerTile;
		float radiusPix = lightEntry.second.radius * App::pixelsPerTile;

		lightmapCones.at(lightEntry.first)->setVisible(
			cameraPix.intersectsCircle(originPix, radiusPix)
		);
	}
}

void GScene::cycleDisplayMode()
{
	enum_increment(displayMode, display);
	if (display == displayMode::end) {
		display = displayMode::begin;
	}
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer, BlendFunc blend)
{
	RenderTexture* rt = initRenderTexture(sceneLayer);
	rt->getSprite()->setBlendFunc(blend);
	return rt;
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer)
{
	RenderTexture* rt = RenderTexture::create(App::width, App::height);
	rt->setPosition(App::width / 2.0f, App::height / 2.0f);
	addChild(rt, to_int(sceneLayer));
	rt->addChild(getLayer(sceneLayer));
	rt->setClearColor(Color4F(0.0f, 0.0f, 0.0f, 0.0f));
	rt->setAutoDraw(true);
	rt->setClearFlags(GL_COLOR_BUFFER_BIT);
	return rt;
}

void GScene::installLuaShell()
{
	luaShell = Node::ccCreate<LuaShell>();
	luaShell->setVisible(false);

	control_listener->addPressListener(
		ControlAction::scriptConsole,
		[=]() -> void {luaShell->toggleVisible(); }
	);
	
	control_listener->addPressListener(
		ControlAction::enter,
		[=]() -> void {if (luaShell->isVisible()) pendingScript = luaShell->getText(); }
	);

	getLayer(GScene::sceneLayers::luaShell)->addChild(luaShell, 1);
}

void GScene::checkPendingScript()
{
	if (!pendingScript.empty()) {
		App::lua->runString(pendingScript);
		pendingScript.clear();
	}
}

void GScene::runScriptInit()
{
    ctx->callIfExistsNoReturn("init");
}
void GScene::runScriptUpdate()
{
    ctx->callIfExistsNoReturn("update");
}

void GScene::queueActions()
{
	actionsMutex.lock();

	for (auto entry : actionsToAdd)
	{
		actions.at(entry.second).push_back(entry.first);
	}
	actionsToAdd.clear();

	actionsMutex.unlock();
}

void GScene::waitForSpaceThread()
{
	unique_lock<mutex> mlock(spaceUpdateConditionMutex);
	spaceUpdateCondition.wait(
		mlock,
		[this]()-> bool { return !spaceUpdateToRun.load(); }
	);
}

void GScene::logPerformance()
{
	unsigned int frames = gspace->getFrame();
	unsigned long us = gspace->getTimeUsed();
	log(
		"Replay: %.3f s processing time, %.3f s elapsed time, %.3f ms per frame.",
		us * 1e-6,
		frames*App::secondsPerFrame,
		us * 1e-3 / frames
	);
}

void GScene::runActionsWithOrder(updateOrder order)
{
	vector<zero_arity_function>& _actions = actions.at(order);

	for (auto it = _actions.begin(); it != _actions.end();++it)
	{
		(*it)();
	}
	_actions.clear();
}

void GScene::setColorFilter(const Color4F& color)
{
	colorFilterDraw->clear();
	colorFilterDraw->drawSolidRect(-1.0f*Vec2(App::width/2, App::height/2), Vec2(App::width/2, App::height/2), color);
}
