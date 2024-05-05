//
//  graphics_context.hpp
//  Koumachika
//
//  Created by Toni on 5/21/19.
//
//

#ifndef graphics_context_hpp
#define graphics_context_hpp

class graphics_context
{
public:
	friend class GScene;
	friend class GSpace;
	friend class node_context;

	inline graphics_context(GScene* scene) : scene(scene) {}

	SpriteID getSpriteID();

	void addPolyLightSource(SpriteID id, shared_ptr<LightArea> light, SpaceVect pos, SpaceFloat angle);
	void addLightSource(SpriteID id, CircleLightArea light, SpaceVect pos, SpaceFloat angle);
	void addLightSource(SpriteID id, AmbientLightArea light, SpaceVect pos, SpaceFloat angle);
	void addLightSource(SpriteID id, ConeLightArea light, SpaceVect pos, SpaceFloat angle);
	void addLightSource(SpriteID id, SpriteLightArea light, SpaceVect pos, SpaceFloat angle);

	void removeLightSource(LightID id);
	void setLightSourceNoise(LightID id, perlin_light_state noise);
	void autoremoveLightSource(LightID id, float seconds);

	void createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createAgentSprite(
		SpriteID id,
		shared_ptr<sprite_properties> _sprite,
		SpaceFloat agentSize,
		GraphicsLayer sceneLayer,
		Vec2 pos
	);
	void createDamageIndicator(float val, SpaceVect pos);
	void createAgentBodyShader(
		SpriteID id, GraphicsLayer layer,
		Color4F bodyColor, Color4F coneColor,
		float bodyRadius, float coneRadius,
		float thickness, Vec2 position
	);
 	void createConeShader(
		SpriteID id, GraphicsLayer layer,
		const Color4F color, float radius,
		const Vec2 center, SpaceFloat coneWidth,
		SpaceFloat initialAngleRad
	);

	void runSpriteAction(SpriteID id, ActionGeneratorType generator);
	void removeSprite(SpriteID id);
	void removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator);
	
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

	template<class C, typename... Params, typename... DecayParams>
	void nodeAction(SpriteID id, void (C::*method)(Params...), DecayParams... params)
	{
		C* c = getSpriteAs<C>(id);
		if (c) {
			(c->*method)(params...);
		}
	}

	template<class C>
	bool _polyAddLight(LightID id, shared_ptr<LightArea> light, SpaceVect pos, SpaceFloat angle)
	{
		if (auto _l = dynamic_cast<C*>(light.get())) {
			addLightSource(id, *_l, pos, angle);
			return true;
		}
		return false;
	}

	Node* getSpriteAsNode(SpriteID id);
	Node* getLight(SpriteID id);
	void _removeSprite(SpriteID id);
	//Should only be called by GScene.
	void update();

	GScene* scene;

	unordered_map<SpriteID, Node*> lightmapNodes;
	unordered_map<SpriteID, perlin_light_state> lightmapNoise;
	unordered_map<SpriteID, float> autoremoveLightTimers;
	siv::PerlinNoise lightmapPerlinNoise;

	atomic_uint nextSpriteID = 1;
	unordered_map<SpriteID, Node*> graphicsNodes;
	unordered_map<SpriteID, Node*> animationNodes;
	vector<DrawNode*> roomMasks;
};

class node_context
{
public:
	inline node_context(GSpace* space) :
		space(space)
	{}

	template<typename... Params>
	inline void createNode(
		void (graphics_context::*create_method)(SpriteID, Params...),
		Params... params
	){
		id = getSpriteID();
		addSceneAction(bind(
			create_method,
			getGraphicsContext(),
			id,
			params...
		));
	}

	template<class C, typename... Params, typename... InputParams>
	void nodeMethod(void (C::*method)(Params...), InputParams... params)
	{
		graphics_context* graphicsContext = getGraphicsContext();
		SpriteID _id = id;
		addSceneAction([graphicsContext, _id, method, params...]() -> void {
			graphicsContext->nodeAction<C, Params...>(
				_id,
				method,
				static_cast<decay_t<Params>>(params)...
			);
		});
	}

	inline operator bool() const{ return id != 0;}
	inline SpriteID getID() const { return id; }

	void createSprite(string texture, GraphicsLayer layer, Vec2 pos, float zoom);
	void createLightSource(
		shared_ptr<LightArea> light,
		SpaceVect pos,
		SpaceFloat angle
	);

	void runAction(GraphicsAction action);
	void stopAction(cocos_action_tag tag);

	void autoremoveLightSource(float seconds);
	void removeWithAction(ActionGeneratorType action);
	void removeSprite();
	void removeLightSource();

	void setVisible(bool v);
	void setPos(SpaceVect p);
	void setPosition(Vec2 p);
	void setAngle(SpaceFloat a);
	void setRotation(float a);
	void setScale(float zoom);
	void setColor(Color3B color);
	void setColor(Color4F color);
	void setOpacity(unsigned char op);
	void setTexture(string s);
	void setShader(string s);

	void clearDrawNode();
	void drawCircle(
		Vec2 center, float radius, float angle,
		unsigned int segments, const Color4F& color
	);
	void drawRectangle(Vec2 lowerLeft, Vec2 upperRight, Color4F color);

protected:
	void addSceneAction(zero_arity_function f);
	graphics_context* getGraphicsContext();
	SpriteID getSpriteID();

	GSpace* space;
	SpriteID id = 0;
};

#endif /* graphics_context_hpp */
