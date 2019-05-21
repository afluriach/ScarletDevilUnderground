//
//  graphics_context.hpp
//  Koumachika
//
//  Created by Toni on 5/21/19.
//
//

#ifndef graphics_context_hpp
#define graphics_context_hpp

#include "graphics_types.h"

class GAnimation;
class GScene;

class graphics_context
{
public:
	friend class GScene;

	inline graphics_context(GScene* scene) : scene(scene) {}

	SpriteID getSpriteID();
	LightID getLightID();

	void addLightSource(LightID id, CircleLightArea light);
	void addLightSource(LightID id, AmbientLightArea light);
	void addLightSource(LightID id, ConeLightArea light);
	void addLightSource(LightID id, SpriteLightArea light);
	void removeLightSource(LightID id);
	void setLightSourcePosition(LightID id, SpaceVect pos);
	void setLightSourceAngle(LightID id, SpaceFloat a);
	void setLightSourceColor(LightID id, Color4F color);
	void setLightSourceNoise(LightID id, perlin_light_state noise);
	void autoremoveLightSource(LightID id, float seconds);

	void createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createAgentSprite(SpriteID id, string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createDamageIndicator(float val, SpaceVect pos);
	void createAgentBodyShader(
		SpriteID id, GraphicsLayer layer,
		Color4F bodyColor, Color4F coneColor,
		float bodyRadius, float coneRadius,
		float thickness, Vec2 position
	);

	void loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation);
	void setAgentAnimationDirection(SpriteID id, Direction d);
	void setAgentAnimationFrame(SpriteID id, int frame);
	void setAgentOverlayShieldLevel(SpriteID id, float level);

	void clearDrawNode(SpriteID id);
	void drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color);
	void drawSolidCone(SpriteID id, Vec2 center, float radius, float startAngle, float endAngle, unsigned int segments, Color4F color);
	void drawSolidCircle(SpriteID id, Vec2 center, float radius, float angle, unsigned int segments, Color4F color);

	void runSpriteAction(SpriteID id, ActionGeneratorType generator);
	void stopSpriteAction(SpriteID id, cocos_action_tag action);
	void stopAllSpriteActions(SpriteID id);
	void removeSprite(SpriteID id);
	void removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator);
	void setSpriteVisible(SpriteID id, bool val);
	void setSpriteOpacity(SpriteID id, unsigned char op);
	void setSpriteTexture(SpriteID id, string path);
	void setSpriteAngle(SpriteID id, float cocosAngle);
	void setSpritePosition(SpriteID id, Vec2 pos);
	void setSpriteZoom(SpriteID id, float zoom);
	void setSpriteColor(SpriteID id, Color3B color);
	void spriteSpatialUpdate(vector<sprite_update> spriteUpdates);

	void clearSubroomMask(unsigned int roomID);
protected:
	template<class C>
	C* getSpriteAs(SpriteID id)
	{
		auto it = graphicsNodes.find(id);
		if (it == graphicsNodes.end()) return nullptr;
		else return dynamic_cast<C*>(it->second);
	}

	template<class C, typename... Params>
	void spriteAction(SpriteID id, void (C::*method)(Params...), Params... params)
	{
		C* c = getSpriteAs<C>(id);
		if (c) {
			(c->*method)(forward<Params>(params)...);
		}
	}

	Node* getSpriteAsNode(SpriteID id);
	void _removeSprite(SpriteID id);
	//Should only be called by GScene.
	void update();

	GScene* scene;

	atomic_uint nextLightID = 1;
	unordered_map<LightID, Node*> lightmapNodes;
	unordered_map<LightID, perlin_light_state> lightmapNoise;
	unordered_map<SpriteID, float> autoremoveLightTimers;
	noise::module::Perlin lightmapPerlinNoise;

	atomic_uint nextSpriteID = 1;
	unordered_map<SpriteID, Node*> graphicsNodes;
	unordered_map<SpriteID, GAnimation*> animationNodes;
	vector<DrawNode*> roomMasks;
};

#endif /* graphics_context_hpp */
