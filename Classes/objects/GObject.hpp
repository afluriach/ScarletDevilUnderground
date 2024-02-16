//
//  GObject.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

#define MapObjCons(cls) cls(GSpace* space, ObjectIDType id, const ValueMap& args)
#define MapObjForwarding(cls) cls(space,id,args)
#define MapObjParams() GObject(object_params(args))

struct parametric_motion
{
	parametric_space_function parametric_f;
	SpaceFloat parametric_t = -1.0;
	parametric_type parametric_move = parametric_type::none;
};

typedef function<GObject*(GSpace*, ObjectIDType, const object_params& params, local_shared_ptr<object_properties> props)> PropsAdapter;

class GObject
{
public:
	friend class GSpace;
	friend class Spell;

    static const unordered_map<type_index, PropsAdapter> propsAdapters;
	static const unordered_set<type_index> trackedTypes;

	static const float objectFadeInTime;
	static const float objectFadeOutTime;
	static const GLubyte objectFadeOpacity;

    static bool conditionalLoad(GSpace* space, local_shared_ptr<object_properties> props, const object_params& params);

	template<typename... Args>
	inline void runMethodIfAvailable(string name, Args... args)
	{
        if(!scriptObj || !hasMethod(name)) return;
 
		sol::function_result result = scriptObj[name](scriptObj, args...);
        if(!result.valid()){
            log1("Error running object method %s", name);
            sol::printErrorMessage(scriptObj.lua_state());
        }
	}

	template<typename R, typename... Args>
	inline R runScriptMethod(string name, Args... args)
	{
        if(!scriptObj)
            logAndThrowError("error calling %s GObject does not haec a scriptObj", name);

        if(!hasMethod(name))
            logAndThrowError("error calling script function %s. %s does not have this method", name, getName());


		sol::function_result result = scriptObj[name](scriptObj, args...);
  
        if(!result.valid()){
            logAndThrowError("Lua error running method %s:", name);
            sol::printErrorMessage(scriptObj.lua_state());
        }
        
        return result;
	}
 
    template<typename T>
    inline T getFieldOrDefault(string name, T _default)
    {
        if(!scriptObj)
            return _default;
            
        sol::object field = scriptObj[name];
        
        if(field.valid())
            return field;
        else
            return _default;
    }
    
    template<class FuncCls, typename... Params>
	inline local_shared_ptr<FuncCls> make(Params... params) {
		return make_local_shared<FuncCls>(this, params...);
	}

	GObject(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		const physics_params& phys,
		local_shared_ptr<object_properties> props
	);
    virtual ~GObject();

	virtual void removePhysicsObjects();
	void removeGraphics(bool removeSprite);

	//object identification, init, and update

	inline GSpace* getSpace() const { return space; }
	inline ObjectIDType getUUID() const { return uuid; }
    local_shared_ptr<object_properties> getProps() const;

	string getProperName() const;
	string getClsName() const;
	string getScriptClsName() const;

	inline virtual int getLevel() const { return 0; }

	gobject_ref getRef() const;

	string getName() const;
	string getTypeIndexName() const;
	string toString() const;

	template<typename T>
	inline T* getAs(){
		return dynamic_cast<T*>(this);
	}

	inline bool isHidden() const { return hidden; }

	//BEGIN LOGIC

	//Called on the first frame after it has been added, before update is called on it or any other
	//objects in the same frame
	virtual void init();
	virtual void update();

	virtual void onPitfall();
	virtual void onRemove();

	inline RoomSensor* getCrntRoom() const { return crntRoom; }
	int getCrntRoomID() const;
	virtual void setCrntRoom(RoomSensor* room);
	void updateRoomQuery();

	//Activation
	inline bool getActive() const { return active; }
	virtual void activate();
	virtual void deactivate();
    void toggleActive();

	//Interaction
	inline virtual bool canInteract(Player* p) { return false; }
	inline virtual void interact(Player* p) {} 
	inline virtual string interactionIcon(Player* p) { return ""; }

	//physics
	inline virtual void onPlayerContact(Player* p) {}

	//sensory
	inline virtual bool isInvisible() const { return false; }

	//Lua
	void scriptInitialize();
	bool hasMethod(const string& name);
    sol::object getScriptField(const string& name);
    inline void _setScriptObj(sol::table obj) { scriptObj = obj; }

	virtual bool hit(DamageInfo damage, SpaceVect n);
	inline virtual bool applyInitialSpellCost(const spell_cost& cost) { return true; }
	inline virtual bool applyOngoingSpellCost(const spell_cost& cost) { return true; }

	//Bullets
	virtual bullet_attributes getBulletAttributes(local_shared_ptr<bullet_properties> props) const;
	bool isBulletObstacle(SpaceVect pos, SpaceFloat radius);

	//Used by Spell, to override bullet attributes.
	gobject_ref _spawnBullet(
		const bullet_attributes& attributes,
		local_shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceVect velocity,
		SpaceFloat angle,
		SpaceFloat angularVelocity
	);
	gobject_ref _launchBullet(
		const bullet_attributes& attributes,
		local_shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceFloat angle,
		SpaceFloat angularVelocity,
		bool obstacleCheck
	);

	gobject_ref spawnBullet(
		local_shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceVect velocity,
		SpaceFloat angle,
		SpaceFloat angularVelocity
	);
	gobject_ref launchBullet(
		local_shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceFloat angle,
		SpaceFloat angularVelocity = 0.0,
		bool obstacleCheck = true
	);
	gobject_ref parametricBullet(
		local_shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		parametric_space_function f,
		SpaceFloat angle,
		SpaceFloat angularVelocity = 0.0,
		bool obstacleCheck = true
	);

	//END LOGIC

	//BEGIN PHYSICS
	
	void launch();
	void launchAtTarget(GObject* target);
    Vec2 getInitialCenterPix();

	bool teleport(SpaceVect pos);
	//query if this object moving to the given position would encounter obstacle(s)
	bool isObstacle(SpaceVect pos);
	SpaceVect getFloorVelocity() const;
	void updateFloorSegment();
	void updateFriction(float _uk);

	SpaceRect getBoundingBox() const;
	SpaceVect getDimensions() const;
	virtual SpaceFloat getTraction() const;
	SpaceFloat getMomentOfInertia() const;

    SpaceVect getPos() const;
    void setPos(SpaceVect p);

    virtual void setAngle(SpaceFloat a);
	SpaceFloat getAngle() const;

    void rotate(SpaceFloat a);
    SpaceVect getFacingVector() const;
    virtual void setDirection(Direction d);
    
    SpaceVect getVel() const;
    void setVel(SpaceVect v);
    
	SpaceFloat getAngularVel()const;
    void setAngularVel(SpaceFloat w);
    
    //Apply a force as impulse where t = frame length.
    void applyForceForSingleFrame(SpaceVect f);
	void applyImpulse(SpaceVect i);
	void applyImpulse(SpaceFloat mag, SpaceFloat angle);

	void setParametricMove(
		parametric_space_function f,
		parametric_type move_type = parametric_type::position
	);
	void removeParametricMove();

	PhysicsLayers getCrntLayers() const;
	void setIsOnFloor(bool v);
	bool getIsOnFloor() const;
	
	void onContactFloor(FloorSegment* fs);
	void onEndContactFloor(FloorSegment* fs);

	inline bool getBodySensor() { return bodyShape->IsSensor(); }
	inline void setBodySensor(bool val) { bodyShape->SetSensor(val); }

	//A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
	inline SpaceFloat getMass() const { return mass; }
	SpaceFloat getRadius() const;
	SpaceFloat uk() const;
	inline GType getType() const { return getBaseType(type); }
	inline GType getFullType() const { return type; }
	inline bool getSensor() const { return sensor; }
	inline PhysicsLayers getLayers() const { return layers; }

	inline virtual SpaceFloat getMaxSpeed() const { return 0.0; }
	inline virtual SpaceFloat getMaxAcceleration() const { return 0.0; }

	void initializeBody();

	void updateParametricMove();

	//END PHYSICS
    
    //BEGIN GRAPHICS
    
	bool isGraphicsObject() const;
        //The Z-order used by Cocos2D.
	virtual GraphicsLayer sceneLayer() const;
	virtual shared_ptr<sprite_properties> getSprite() const;
	virtual shared_ptr<LightArea> getLightSource() const;
	int sceneLayerAsInt() const;
    virtual sprite_update updateSprite();
	void createLight();
    void removeLight();
    void setLightSourceAngle(SpaceFloat angle);
	//If "id" is not provided, the object's defaut sprite, spriteID, will be
	//used, assuming it is valid.
	void addGraphicsAction(GraphicsAction action);
	void addGraphicsAction(GraphicsAction action, SpriteID id);
	void stopGraphicsAction(cocos_action_tag tag);
	void stopGraphicsAction(cocos_action_tag tag, SpriteID id);
	void setSpriteZoom(float zoom);
	void setSpriteOpacity(unsigned char op);
	void setSpriteVisible(bool val);
	void setSpriteTexture(const string& texture);

	//Create Node which graphically reprensets this object and adds it to Layer
	virtual void initializeGraphics();
	
	inline SpriteID getSpriteID() const { return spriteID; }
    LightID getLightID() const { return lightID; }

    //END GRAPHICS

	//BEGIN AUDIO
	
	virtual ALuint playSoundSpatial(
		const string& path,
		float volume = 1.0f,
		bool loop = false,
		float yPos = 0.0f
	);
	void stopSound(ALuint source);

	//END AUDIO

	//BEGIN SPELLS

	local_shared_ptr<Spell> cast(const SpellDesc* desc);
	local_shared_ptr<MagicEffect> applyMagicEffect(const MagicEffectDescriptor* effect, effect_attributes attr);
	void updateEffect(MagicEffect* effect);
	void updateEffects();
    void removeEffects();

	//END SPELLS

    const string name;
	GSpace *const space;
	const ObjectIDType uuid;

protected:
	bool active = false;
	bool hidden = false;
	bool rotateSprite = false;
	bool isInFade = false;
	bool isFrozen = false;
	bool sensor = false;
	bool isOnFloor = false;

//logic
	sol::table scriptObj;
    list<local_shared_ptr<MagicEffect>> effects;
	local_shared_ptr<object_properties> props;
	unique_ptr<parametric_motion> parametricMotion;
	RoomSensor* crntRoom = nullptr;

//physics
	b2Body* body = nullptr;
	b2Fixture* bodyShape = nullptr;

	SpaceVect dimensions;
	SpaceFloat mass;
	GType type;
	PhysicsLayers layers;

	SpaceVect prevPos = SpaceVect::zero;
	SpaceFloat prevAngle = 0.0;
	SpaceVect startingVel = SpaceVect::zero;
	SpaceFloat startingAngularVel = 0.0;

	FloorSegment* crntFloorCenterContact = nullptr;
	//Floor segments that the object is overlapping with. The start/end contact
	//handler will be called for these when setting isOnFloor.
	list<FloorSegment*> crntFloorContacts;

//graphics
	SpriteID spriteID = 0;
	SpriteID drawNodeID = 0;
	LightID lightID = 0;
};

#endif /* GObject_hpp */
