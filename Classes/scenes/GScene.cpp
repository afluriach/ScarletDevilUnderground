//
//  GScene.cpp
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "Dialog.hpp"
#include "FileIO.hpp"
#include "functional.hpp"
#include "graphics_context.hpp"
#include "GScene.hpp"
#include "LuaAPI.hpp"
#include "LuaShell.hpp"
#include "menu.h"
#include "PlayScene.hpp"
#include "value_map.hpp"

const Color4F GScene::defaultAmbientLight = Color4F(0.5f, 0.5f, 0.5f, 1.0f);
const int GScene::dialogEdgeMargin = 30;
const bool GScene::scriptLog = false;

string GScene::crntSceneName;
bool GScene::suppressGameOver = false;

GScene* GScene::runScene(const string& name)
{
    auto it = adapters.find(name);
	area_properties props;
	props = app::getArea(name);

	//Run built-in scene adapter
	if (it != adapters.end()) {
		crntSceneName = name;
		return it->second();
    }
	//Action scene from area_properties
	else if (!props.sceneName.empty()) {
		crntSceneName = name;
		return App::createAndRunScene<PlayScene>(props);
	}
	//Action scene with default settings from map file.
	else if (FileUtils::getInstance()->isFileExist("maps/" + name + ".tmx")) {
		crntSceneName = name;
		log("Warning, creating scene %s from map file instead of properties!", name);
		return App::createAndRunScene<PlayScene>(name);
	}
	else {
		log("runScene: %s not found", name.c_str());
		return nullptr;
	}
}

void GScene::restartScene()
{
	runScene(crntSceneName);
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

    //Create the sublayers at construction (so they are available to mixins at construction time).
    //But do not add sublayers until init time.
    for_irange(i,sceneLayers::begin,sceneLayers::end){
        Layer* l = Layer::create();
        layers.insert(i, l);
    }

	isExit = false;
	isPaused = false;

	graphicsContext = make_unique<graphics_context>(this);
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
	if constexpr(GSPACE_MULTITHREAD) {
		spaceUpdateCondition.notify_one();
		spaceUpdateThread->join();
	}
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
	lightmapBackground->drawSolidRect(Vec2::ZERO, Vec2(app::params.width, app::params.height), Color4F::WHITE);

	colorFilterDraw = DrawNode::create();
	setColorFilter(Color4F::WHITE);
	getLayer(sceneLayers::screenspaceColorFilter)->addChild(colorFilterDraw);

	//Apply zoom to adjust viewable area size.
	float baseViewWidth = app::params.width * app::tilesPerPixel;
	spaceZoom = baseViewWidth / app::viewWidth;

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
    
	string current = getCurrentLevel();
	gspace->setRandomSeed(to_uint(time(nullptr)));
	gspace->crntChamber = current;

	multiInit();

	gspace->isMultiMap = isMultiMap();

	if constexpr (GSPACE_MULTITHREAD) {
		spaceUpdateToRun.store(false);
		spaceUpdateThread = make_unique<thread>(&GScene::spaceUpdateMain, this);
	}

    return true;
}

void GScene::update(float dt)
{	
	if (!isPaused) {
		ControlInfo info = App::control_register->getControlInfo();
		gspace->addObjectAction(bind(
			&GSpace::setControlInfo,
			gspace,
			info
		));

		if constexpr (GSPACE_MULTITHREAD) {
			spaceUpdateToRun.store(true);
			spaceUpdateCondition.notify_one();
		}
		else {
			gspace->update();
		}

		runActions();
		checkPendingScript();
		runScriptUpdate();
		renderSpace();
	}
}

void GScene::onExit()
{
	while (!menuStack.empty()) popMenu();

	waitForSpaceThread();

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
    return Vec2(app::params.width/2, Dialog::height/2 + dialogEdgeMargin);
}

bool GScene::isDialogActive()
{
	return dialog != nullptr;
}

void GScene::processAdditions()
{
	gspace->processAdditions();
}

void GScene::addActions(const vector<zero_arity_function>& _actions)
{
	actionsMutex.lock();

	for (auto entry : _actions) {
		actionsToRun.push_back(entry);
	}

	actionsMutex.unlock();
}

void GScene::runActions()
{
	vector<zero_arity_function> _actions;

	actionsMutex.lock();
	swap(_actions, actionsToRun);
	actionsMutex.unlock();

	for (auto f : _actions) f();
}

void GScene::setUnitPosition(const SpaceVect& v)
{
	cameraArea = calculateCameraArea(v);
		
	getSpaceLayer()->setPosition(
		(-app::pixelsPerTile*v.x + app::params.width / 2)*spaceZoom - app::params.width/2,
		(-app::pixelsPerTile*v.y + app::params.height / 2)*spaceZoom - app::params.height/2
	);
	getLayer(sceneLayers::lightmap)->setPosition(
		(-app::pixelsPerTile*v.x + app::params.width / 2)*spaceZoom - app::params.width/2,
		(-app::pixelsPerTile*v.y + app::params.height / 2)*spaceZoom - app::params.height/2
	);

	if (tilemaps.size() > 1) {
		updateMultimapVisibility(v);
	}
}

SpaceRect GScene::getCameraArea()
{
	return cameraArea;
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
			try {
				gspace->update();
			}
			catch (sol::error e){
				log("%s", e.what());
				exit(0);
			}
			spaceUpdateToRun.store(false);
			spaceUpdateCondition.notify_one();
		}
	}
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

void GScene::waitForSpaceThread()
{
	if constexpr (!GSPACE_MULTITHREAD) return;

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
		frames*app::params.secondsPerFrame,
		us * 1e-3 / frames
	);
}
