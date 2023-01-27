//
//  lua_app.cpp
//  Koumachika
//
//  Created by Toni on 1/26/20.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "Bomb.hpp"
#include "GScene.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "SpellDescriptor.hpp"

namespace Lua{

	void Inst::addApp()
	{
		auto app = _state.new_usertype<App>(
			"App",
			"state", sol::property(&App::getCrntState)
		);
#define _cls App

        addFuncSame(app, loadScene);
		addFuncSame(app, runPlayScene);

		addFuncSame(app, getParams);
		addFuncSame(app, getCrntScene);
		addFuncSame(app, printGlDebug);
#if USE_TIMERS
		addFuncSame(app, printTimerInfo);
		addFuncSame(app, setLogTimers);
#endif
		addFuncSame(app, setFullscreen);
		addFuncSame(app, setVsync);
		addFuncSame(app, setResolution);
		addFuncSame(app, setFramerate);
		addFuncSame(app, setPlayer);
		addFuncSame(app, setUnlockAllEquips);
		addFuncSame(app, loadProfile);
		addFuncSame(app, saveProfile);

		addFuncSame(app, clearAllKeys);
		addFuncSame(app, clearKeyAction);
		addFuncSame(app, addKeyAction);
#if use_gamepad
		addFuncSame(app, clearAllButtons);
		addFuncSame(app, clearButtonAction);
		addFuncSame(app, addButtonAction);
#endif

		auto app_consts = _state.create_table();
		_state["app_constants"] = app_consts;

		cFuncSameNS(app_consts, app, baseWidth);
		cFuncSameNS(app_consts, app, baseHeight);
		cFuncSameNS(app_consts, app, pixelsPerTile);
		cFuncSameNS(app_consts, app, tilesPerPixel);
		cFuncSameNS(app_consts, app, viewWidth);
		cFuncSameNS(app_consts, app, Gaccel);

#define _cls app_params
		auto params = _state.new_usertype<app_params>(
			"app_params",
			"width", sol::property(&app_params::getWidth),
			"height", sol::property(&app_params::getHeight),
			"fullscreen", sol::property(&app_params::getFullscreen),
			"vsync", sol::property(&app_params::getVsync),
			"showTimers", sol::property(&app_params::getShowTimers),
			"difficultyScale", sol::property(&app_params::getDifficultyScale),
			"unlockAllEquips", sol::property(&app_params::getUnlockAll),
			"framesPerSecond", sol::property(&app_params::getFPS),
			"secondsPerFrame", sol::property(&app_params::getFrameInterval)
		);

		auto _app = _state.create_table();
		_state["app"] = _app;

		_app["addBullet"] = &app::addBullet;

		_app["getBomb"] = &app::getBomb;
		_app["getBullet"] = &app::getBullet;
		_app["getEffect"] = &app::getEffect;
		_app["getSpell"] = &app::getSpell;

		_app["getLight"] = &app::getLight;
		_app["getSprite"] = &app::getSprite;

		_app["log"] = &log_print<>;
	}
}
