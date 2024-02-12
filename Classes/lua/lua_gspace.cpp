//
//  lua_gspace.cpp
//  Koumachika
//
//  Created by Toni on 1/26/20.
//
//

#include "Prefix.h"

#include "Graphics.h"
#include "graphics_context.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"

namespace Lua{
  	
	void Inst::addGSpace()
	{
		auto attr = newType(AttributeSystem);
#define _cls AttributeSystem

		attr["get"] = sol::overload(
			&AttributeSystem::get,
			&AttributeSystem::operator[]
		);

		attr["set"] = sol::overload(
			static_cast<void(AttributeSystem::*)(string, float)>(&AttributeSystem::set),
			static_cast<void(AttributeSystem::*)(Attribute, float)>(&AttributeSystem::set)
		);

		attr["modifyAttribute"] = static_cast<void(AttributeSystem::*)(Attribute, float)>(&AttributeSystem::modifyAttribute);

		addFuncSame(attr, isZero);
		addFuncSame(attr, isNonzero);
		addFuncSame(attr, timerDecrement);
		addFuncSame(attr, timerIncrement);
		addFuncSame(attr, setFullHP);
		addFuncSame(attr, setFullMP);
		addFuncSame(attr, setFullStamina);

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

        auto gspace = _state.new_usertype<GSpace>(
            "GSpace",
            sol::no_constructor,
            "crntSpace", sol::property(&GSpace::getCrntSpace)
        );
#define _cls GSpace

		gspace["getObjectByName"] = static_cast<GObject * (GSpace::*)(const string&) const>(&GSpace::getObject);
        gspace["getObject"] = static_cast<GObject * (GSpace::*)(const string&) const>(&GSpace::getObject);      
		addFuncSame(gspace, createAreaSensor);
		addFuncSame(gspace, createBullet);
		addFuncSame(gspace, getPlayer);
		addFuncSame(gspace, getPlayerAsRef);
		addFuncSame(gspace, getFrame);
		addFuncSame(gspace, getObjectCount);
		addFuncSame(gspace, isObstacle);
		gspace["removeObject"] = sol::overload(
			static_cast<void(GSpace::*)(const string&)>(&GSpace::removeObject),
			static_cast<void(GSpace::*)(gobject_ref)>(&GSpace::removeObject),
			static_cast<void(GSpace::*)(GObject*)>(&GSpace::removeObject)
		);
		addFuncSame(gspace, getRandomInt);
		gspace["getRandomFloat"] = sol::overload(
			static_cast<float(GSpace::*)()>(&GSpace::getRandomFloat),
			static_cast<float(GSpace::*)(float, float)>(&GSpace::getRandomFloat)
		);

        addFuncSame(gspace, teleportPlayerToDoor);
        gspace["teleportToDoor"] = &GSpace::teleportPlayerToDoor;
        
		addFuncSame(gspace, getPath);
		addFuncSame(gspace, getWaypoint);
		addFuncSame(gspace, getRandomWaypoint);
		addFuncSame(gspace, getArea);

		addFuncSame(gspace, registerRoomMapped);

		gspace["createSprite"] = [](
			GSpace* _this,
			string sprite,
			GraphicsLayer layer,
			SpaceVect pos,
			float zoom
			) -> SpriteID {
			return _this->createSprite(
				&graphics_context::createSprite,
				string("sprites/" + sprite + ".png"),
				layer,
				toCocos(pos) * app::pixelsPerTile,
				to_float(zoom)
			);
		};
		gspace["removeSprite"] = [](GSpace* _this,SpriteID id){
			_this->addGraphicsAction(&graphics_context::removeSprite, id);
		};
        gspace["setSpriteVisible"] = [](GSpace* _this,SpriteID id, bool v){
            _this->graphicsNodeAction(&Node::setVisible, id, v);
		};
        gspace["setSpriteColor"] = [](GSpace* _this,SpriteID id, Color3B color){
            _this->graphicsNodeAction(&Node::setColor, id, color);
		};
		gspace["setRotation"] = [](
			GSpace* _this,
			SpriteID id,
			SpaceFloat angle
		) -> void {
			_this->graphicsNodeAction(
				&Node::setRotation,
				id,
				toCocosAngle(angle)
			);
		};

		gspace["createDialog"] = sol::overload(
			static_cast<void(GSpace::*)(string, bool)>(&GSpace::createDialog),
			static_cast<void(GSpace::*)(string, bool, zero_arity_function)>(&GSpace::createDialog)
		);
		addFuncSame(gspace, enterWorldSelect);
        addFuncSame(gspace, loadScene);

		auto gstate = newType(GState);
#define _cls GState

		gstate["addItem"] = sol::overload(
			static_cast<void(GState::*)(string)>(&GState::addItem),
			static_cast<void(GState::*)(string, unsigned int)>(&GState::addItem)
		);
		gstate["removeItem"] = sol::overload(
			static_cast<bool(GState::*)(string)>(&GState::removeItem),
			static_cast<bool(GState::*)(string, unsigned int)>(&GState::removeItem)
		);

		addFuncSame(gstate, hasItem);
		addFuncSame(gstate, getItemCount);

		addFuncSame(gstate, setAttribute);
		addFuncSame(gstate, getAttribute);
		addFuncSame(gstate, hasAttribute);
		addFuncSame(gstate, incrementAttribute);
		addFuncSame(gstate, subtractAttribute);
	}
}
