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

class GObject
{
public:
	friend class GSpace;
	friend class Spell;

	typedef function<GObject*(GSpace*, ObjectIDType, const ValueMap&) > AdapterType;

	struct object_info
	{
		AdapterType consAdapter;
		type_index type;
	};

    static constexpr bool logCreateObjects = false;
	static const unordered_set<type_index> trackedTypes;
	//Map each class name to a constructor adapter function.
	static unordered_map<string, object_info> objectInfo;
	//Map named object subtypes to the function that creates the corresponding object.
	static unordered_map<string, AdapterType> namedObjectTypes;
	static unordered_map<type_index, string> typeNameMap;

	static const float objectFadeInTime;
	static const float objectFadeOutTime;
	static const GLubyte objectFadeOpacity;

	static GObject* constructByType(GSpace* space, ObjectIDType id, const string& type, const ValueMap& args);
	static ObjectGeneratorType factoryMethodByType(const string& type, const ValueMap& args);
	static bool isValidObjectType(string typeName);
	static const object_info* getObjectInfo(string name);
	static const object_info* getObjectInfo(type_index t);
	static type_index getTypeIndex(string name);
	static void initNameMap();
	static void initObjectInfo();

	template<class ObjectCls, typename... ConsArgs, typename... Params>
	static inline GObject* create(GSpace* space, ObjectIDType id, Params... params)
	{
		return allocator_new<ObjectCls>(space,id,std::forward<ConsArgs>(params)...);
	}

	template<class ObjectCls, typename... ConsArgs>
	static inline ObjectGeneratorType make_object_factory(ConsArgs... args)
	{
		return [args...](GSpace* space, ObjectIDType id) -> GObject* {
			return create<ObjectCls, ConsArgs...>(space, id, args...);
		};
	}

	static inline ObjectGeneratorType null_object_factory()
	{
		return[](GSpace* space, ObjectIDType id)->GObject* {
			return nullptr;
		};
	}

	template<typename... Args>
	void makeInitMessage(void (GObject::*m)(Args...), gobject_ref ref, Args... args)
	{
		space->addInitAction([m, ref, args...]() -> void {
			(ref.get()->*m)(args...);
		});
	}

	template<typename... Args>
	inline void runVoidScriptMethod(string name, Args... args)
	{
		scriptObj[name](scriptObj, args...);
	}

	template<typename R, typename... Args>
	inline R runScriptMethod(string name, Args... args)
	{
		return scriptObj[name](scriptObj, args...);
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

	inline string getProperName() const { return props->properName; }
	inline string getClsName() const { return props->clsName; }

	inline virtual int getLevel() const { return 0; }

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

	//Interaction
	inline virtual bool canInteract(Player* p) { return false; }
	inline virtual void interact(Player* p) {} 
	inline virtual string interactionIcon(Player* p) { return ""; }

	//physics
	inline virtual void onPlayerContact(Player* p) {}

	//sensory
	inline virtual bool isInvisible() const { return false; }

	//StateMachine
	void updateFSM();
	void printFSM();
	void setFrozen(bool val);

	//Lua
	void scriptInitialize();
	bool hasMethod(const string& name);

	virtual bool hit(DamageInfo damage, SpaceVect n);
	inline virtual bool applyInitialSpellCost(const spell_cost& cost) { return true; }
	inline virtual bool applyOngoingSpellCost(const spell_cost& cost) { return true; }

	//Bullets
	virtual bullet_attributes getBulletAttributes(local_shared_ptr<bullet_properties> props) const;
	bool isBulletObstacle(SpaceVect pos, SpaceFloat radius);

	//Used by Spell, to override bullet attributes.
	gobject_ref GObject::_spawnBullet(
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
	bool isOnFloor() const;
	SpaceVect getFloorVelocity() const;
	void updateFloorSegment();
	void updateFriction(float _uk);
	void onContactFloorSegment(FloorSegment* fs);
	void onEndContactFloorSegment(FloorSegment* fs);

	SpaceRect getBoundingBox() const;
	SpaceVect getDimensions() const;
	virtual SpaceFloat getTraction() const;
	SpaceFloat getMomentOfInertia() const;

    SpaceVect getPos() const;
	SpaceVect getDeltaPos() const;
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
	void setLayers(PhysicsLayers layers);

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

	inline virtual SpaceFloat getMaxSpeed() const { return 0; }
	inline virtual SpaceFloat getMaxAcceleration() const { return 0; }

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
	void initLightSource();
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

	unsigned int cast(const SpellDesc* desc);
	bool isSpellActive(const SpellDesc* desc);
	unsigned int applyMagicEffect(const MagicEffectDescriptor* effect, effect_attributes attr);

	//END SPELLS

	GSpace *const space;
	const ObjectIDType uuid;

protected:
	bool active = false;
	bool hidden = false;
	bool rotateSprite = false;
	bool isInFade = false;
	bool isFrozen = false;
	bool sensor = false;

//logic
	sol::table scriptObj;
	local_shared_ptr<object_properties> props;
	unique_ptr<ai::StateMachine> fsm;
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

	FloorSegment* crntFloorCenterContact = nullptr;
	unordered_set<FloorSegment*> crntFloorContacts;

//graphics
	SpriteID spriteID = 0;
	SpriteID drawNodeID = 0;
	LightID lightID = 0;
};

#endif /* GObject_hpp */
