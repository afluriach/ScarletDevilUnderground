//
//  lua_scene.cpp
//  Koumachika
//
//  Created by Toni on 1/26/20.
//
//

#include "Prefix.h"

#include "Graphics.h"
#include "GScene.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "PlayScene.hpp"

namespace Lua{
  
	void Inst::addScene()
	{
		auto graphics = _state.create_table();
		_state["graphics"] = graphics;
		
		cFuncSame(graphics, pitfallShrinkAction);
	
        auto gscene = _state.new_usertype<GScene>(
            "GScene",
            sol::no_constructor,
            "crntScene", sol::property(&GScene::getCrntScene)
        );
        
#define _cls GScene

		addFuncSame(gscene, getSpace);
		addFuncSame(gscene, setPaused);
		addFuncSame(gscene, stopDialog);
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
