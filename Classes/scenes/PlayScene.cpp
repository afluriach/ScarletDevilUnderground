//
//  PlayScene.cpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

#include "audio_context.hpp"
#include "Dialog.hpp"
#include "Door.hpp"
#include "FileIO.hpp"
#include "functional.hpp"
#include "Graphics.h"
#include "GScene.hpp"
#include "HUD.hpp"
#include "menu_layers.h"
#include "multifunction.h"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "value_map.hpp"

const Color3B PlayScene::fadeoutColor = Color3B(192,96,96);
const float PlayScene::fadeoutLength = 3.0f;

PlayScene::PlayScene(shared_ptr<area_properties> props, string start) :
GScene(props, start)
{
	multiInit.insertWithOrder(
		wrap_method(PlayScene,addHUD,this),
		to_int(initOrder::initHUD)
	);
	multiInit.insertWithOrder(
		wrap_method(PlayScene, showVisibleRooms, this),
		to_int(initOrder::showRooms)
	);
	multiInit.insertWithOrder(
		wrap_method(PlayScene, loadPlayer, this),
		to_int(initOrder::postLoadObjects)
	);

	control_listener->addPressListener(
		ControlAction::pause,
		[this]()-> void {
			this->onPausePressed();
		}
	);
	
	control_listener->addPressListener(
		ControlAction::inventory_menu,
		[this]()-> void {
			this->onInventoryPressed();
		}
	);

	control_listener->addPressListener(
		ControlAction::map_menu,
		[=]()-> void {onMapPressed(); }
	);

#if use_sound
	App::audioContext->resumeSounds();
#endif
}

PlayScene::~PlayScene()
{
}

void PlayScene::update(float dt)
{
	GScene::update(dt);

	if(!isPaused) hud->update();
}

void PlayScene::loadPlayer()
{
	App::getCrntState()->area = areaProps->sceneName;
	App::getCrntState()->entrance = start;

	Door * door = gspace->getObjectAs<Door>(start);
	SpaceVect player_start = gspace->getWaypoint(start);
	object_params params;

	if (door) {
		params.pos = door->getEntryPosition();
		params.angle = dirToPhysicsAngle(door->getEntryDirection());
		
		gspace->createObject<Player>(params, GSpace::playerCharacter);
	}
	else if (!player_start.isZero()){
		params.pos = player_start;
		params.angle = dirToPhysicsAngle(Direction::up);

		gspace->createObject<Player>(params, GSpace::playerCharacter);
	}
	else{
		log1("Scene %s, unknown player start!", getName());
	}
}

void PlayScene::showVisibleRooms()
{
	auto it = App::crntState->areaStats.find(getCurrentLevel());
	if (it != App::crntState->areaStats.end()) {
		setRoomsVisible(it->second.roomsVisited);
		setRoomsDiscovered(it->second.roomsMapped);
	}
}

void PlayScene::addHUD()
{
	hud = Node::ccCreate<HUD>(gspace);
	getLayer(sceneLayers::hud)->addChild(hud);
	hud->setPerformanceStats();
}

void PlayScene::onPausePressed()
{
	if (!isPaused) {
		enterPause();
	}
	else if (isPaused && menuStack.size() == 1) {
		exitPause();
	}
}

void PlayScene::onMapPressed()
{
	if (mapMenu)
		exitMap();
	else
		enterMap();
}

void PlayScene::enterPause()
{
	if (isShowingMenu)
		return;

	pauseAnimations();
#if use_sound
	App::audioContext->pauseSounds();
#endif
	setPaused(true);
	isShowingMenu = true;
	if(hud) hud->showHidden();

	waitForSpaceThread();

	Player* player = gspace->getPlayer();

	if (!player) {
		log0("no Player object!");
	}

	pauseMenu = Node::ccCreate<PauseMenu>(isOverworld, player);
	pushMenu(pauseMenu);
}

void PlayScene::exitPause()
{
	popMenu();
	pauseMenu = nullptr;
	resumeAnimations();
#if use_sound
	App::audioContext->resumeSounds();
#endif
	setPaused(false);
	isShowingMenu = false;
	hud->resetAutohide();
}

void PlayScene::pauseAnimations()
{
	getSpaceLayer()->pauseRecursive();
	getLayer(sceneLayers::hud)->pauseRecursive();
}

void PlayScene::resumeAnimations()
{
	getSpaceLayer()->resumeRecursive();
	getLayer(sceneLayers::hud)->resumeRecursive();
}

void PlayScene::showGameOverMenu()
{
	setPaused(true);

	waitForSpaceThread();
	logPerformance();

	showMenu(Node::ccCreate<GameOverMenu>());
}

void PlayScene::triggerGameOver()
{
	setPaused(true);

	setColorFilter(toColor4F(fadeoutColor));

	triggerMenu(&PlayScene::showGameOverMenu);
}

void PlayScene::enterMap()
{
	if (isShowingMenu || isOverworld)
		return;

	pauseAnimations();
#if use_sound
	App::audioContext->pauseSounds();
#endif
	setPaused(true);
	isShowingMenu = true;

	waitForSpaceThread();

	mapMenu = Node::ccCreate<MapMenu>(this);
	pushMenu(mapMenu);
}

void PlayScene::exitMap()
{
	popMenu();
	mapMenu = nullptr;
	resumeAnimations();
#if use_sound
	App::audioContext->resumeSounds();
#endif
	setPaused(false);
	isShowingMenu = false;
}

void PlayScene::onInventoryPressed()
{
	if (inventoryMenu)
		exitInventoryMenu();
	else
		enterInventoryMenu();
}

void PlayScene::enterInventoryMenu()
{
	pauseAnimations();
#if use_sound
	App::audioContext->pauseSounds();
#endif
	setPaused(true);
	isShowingMenu = true;

	waitForSpaceThread();

	inventoryMenu = Node::ccCreate<InventoryInfo>();
	inventoryMenu->setPosition(app::params.width * 0.25f, app::params.height * 0.5f);
	pushMenu(inventoryMenu);
}

void PlayScene::exitInventoryMenu()
{
	popMenu();
	inventoryMenu = nullptr;
	resumeAnimations();
#if use_sound
	App::audioContext->resumeSounds();
#endif
	setPaused(false);
	isShowingMenu = false;
}

void PlayScene::enterWorldSelect()
{
	if (isShowingMenu)
		return;

	pauseAnimations();
#if use_sound
	App::audioContext->pauseSounds();
#endif
	setPaused(true);
	isShowingMenu = true;

	waitForSpaceThread();

	pushMenu(Node::ccCreate<WorldSelect>(false));
}

void PlayScene::exitWorldSelect()
{
	popMenu();
	resumeAnimations();
#if use_sound
	App::audioContext->resumeSounds();
#endif
	setPaused(false);
	isShowingMenu = false;
}

GScene* PlayScene::getReplacementScene()
{
	return Node::ccCreate<PlayScene>(areaProps, start);
}

void PlayScene::showMenu(MenuLayer* menu)
{
	hud->setVisible(false);
	if (dialog)
		dialog->setVisible(false);

	pushMenu(menu);
}

void PlayScene::triggerMenu(void (PlayScene::*m)(void))
{
	if (isShowingMenu)
		return;

	isShowingMenu = true;

	Director::getInstance()->getScheduler()->schedule(
		bind(m, this),
		this,
		0.0f,
		0,
		fadeoutLength,
		false,
		"showMenu"
	);
}
