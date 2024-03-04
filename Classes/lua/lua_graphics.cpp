//
//  lua_graphics.cpp
//  Koumachika
//
//  Created by Toni on 3/4/24.
//

#include "Prefix.h"

#include "Graphics.h"
#include "LuaAPI.hpp"

namespace Lua
{
	void Inst::addGraphics()
	{
		auto graphics = _state.create_table();
		_state["graphics"] = graphics;

		cFuncSame(graphics, indefiniteFlickerAction);
		cFuncSame(graphics, flickerAction);
		cFuncSame(graphics, flickerTintAction);
		cFuncSame(graphics, comboFlickerTintAction);
		cFuncSame(graphics, spellcardFlickerTintAction);
		cFuncSame(graphics, darknessCurseFlickerTintAction);
		cFuncSame(graphics, tintToAction);
		cFuncSame(graphics, motionBlurStretch);
		cFuncSame(graphics, bombAnimationAction);
		cFuncSame(graphics, freezeEffectAction);
		cFuncSame(graphics, freezeEffectEndAction);
		cFuncSame(graphics, objectFadeOut);
		cFuncSame(graphics, damageIndicatorAction);

#define _cls node_context
		auto nodecontext = _state.new_usertype<node_context>(
			"node_context",
			sol::constructors<node_context(GSpace*)>()
		);

		nodecontext["valid"] = &node_context::operator bool;
		addFuncSame(nodecontext, getID);

		addFuncSame(nodecontext, createSprite);
		addFuncSame(nodecontext, createLightSource);

		addFuncSame(nodecontext, runAction);
		addFuncSame(nodecontext, stopAction);

		addFuncSame(nodecontext, autoremoveLightSource);
		addFuncSame(nodecontext, removeWithAction);
		addFuncSame(nodecontext, removeSprite);
		addFuncSame(nodecontext, removeLightSource);

		addFuncSame(nodecontext, setVisible);
		addFuncSame(nodecontext, setPos);
		addFuncSame(nodecontext, setPosition);
		addFuncSame(nodecontext, setAngle);
		addFuncSame(nodecontext, setRotation);
		addFuncSame(nodecontext, setScale);
		addFuncSame(nodecontext, setOpacity);
		addFuncSame(nodecontext, setTexture);
		addFuncSame(nodecontext, setShader);

		addFuncSame(nodecontext, clearDrawNode);
		addFuncSame(nodecontext, drawCircle);
		addFuncSame(nodecontext, drawRectangle);

		nodecontext["setColor"] = sol::overload(
			static_cast<void (node_context::*)(Color3B)>(&node_context::setColor),
			static_cast<void (node_context::*)(Color4F)>(&node_context::setColor)
		);
	}

}
