//
//  lua_scene.cpp
//  Koumachika
//
//  Created by Toni on 1/26/20.
//
//

#include "Prefix.h"

#include "GScene.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "PlayScene.hpp"

namespace Lua{
  
	void Inst::addScene()
	{
		auto gscene = newType(GScene);
#define _cls GScene

		addFuncSame(gscene, runScene);
		addFuncSame(gscene, getSpace);
		addFuncSame(gscene, setPaused);
		addFuncSame(gscene, stopDialog);
		addFuncSame(gscene, teleportToDoor);
		addFuncSame(gscene, setRoomVisible);
		addFuncSame(gscene, unlockAllRooms);

		gscene["createDialog"] = static_cast<void(GScene::*)(const string&, bool)>(&GScene::createDialog);

		auto playscene = _state.new_usertype<PlayScene>("PlayScene", sol::base_classes, sol::bases<GScene>());
#define _cls PlayScene

		auto hud = newType(HUD);
#define _cls HUD

		addFuncSame(hud, setMansionMode);
	}
}
