//
//  GObject.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

#include "graphics_types.h"
#include "multifunction.h"
#include "object_params.hpp"

class Player;
class Bullet;
class GSpace;
class Spell;
class MagicEffect;
class FloorSegment;
class RoomSensor;

#define MapObjCons(cls) cls(GSpace* space, ObjectIDType id, const ValueMap& args)
#define MapObjForwarding(cls) cls(space,id,args)
#define MapObjParams() GObject(make_shared<object_params>(space,id,args))
#define ParamsCons(cls) cls(shared_ptr<object_params> params)
#define ParamsForwarding(cls) cls(params)

class GObject
{
public:
	friend class GSpace;

	typedef function<GObject*(GSpace*, ObjectIDType, const ValueMap&) > AdapterType;

	enum class initOrder {
		base=1,
		loadAttributes,
		postLoadAttributes,
		initFSM
	};

	struct object_info
	{
		AdapterType consAdapter;
		type_index type;
		string properName;
	};

    static constexpr bool logCreateObjects = false;
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
	static string properNameByType(type_index t);
	static bool isValidObjectType(string typeName);
	static const object_info* getObjectInfo(string name);
	static const object_info* getObjectInfo(type_index t);
	static type_index getTypeIndex(string name);
	static void initNameMap();
	static void initObjectInfo();

	template<class ObjectCls, typename... ConsArgs>
	static inline GObject* create(GSpace* space, ObjectIDType id, ConsArgs...args)
	{
		return new ObjectCls(space,id,args...);
	}

	template<class ObjectCls, typename... ConsArgs>
	static inline ObjectGeneratorType make_object_factory(ConsArgs... args)
	{
		return [args...](GSpace* space, ObjectIDType id) -> GObject* {
			return create<ObjectCls, ConsArgs...>(space, id, args...);
		};
	}

	template<class ObjectCls, typename... ConsArgs>
	static inline ObjectGeneratorType params_object_factory(shared_ptr<object_params> params, ConsArgs... args)
	{
		return[params, args...](GSpace* space, ObjectIDType id)->GObject* {
			params->space = space;
			params->id = id;

			return new ObjectCls(params, args...);
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

	GObject(shared_ptr<object_params> params, const physics_params& phys);
    virtual ~GObject();

	virtual void removePhysicsObjects();
	void removeGraphics(bool removeSprite);

	//object identification, init, and update

	string getTypeName() const;
	virtual string getProperName() const;

	inline string getName() const {
		return (name.empty()) ? getTypeName() : name;
	}

	inline ObjectIDType getUUID() const {
		return uuid;
	}

	template<typename T>
	inline T* getAs(){
		return dynamic_cast<T*>(this);
	}

	inline bool isAnonymous() const { return name.empty(); }
	inline bool isHidden() const { return hidden; }

	//BEGIN LOGIC

	//Called on the first frame after it has been added, before update is called on it or any other
	//objects in the same frame
	virtual void init();
	virtual void update();

	virtual void onPitfall();
	inline virtual void onRemove() {}

	inline RoomSensor* getCrntRoom() const { return crntRoom; }
	int getCrntRoomID() const;
	virtual void setCrntRoom(RoomSensor* room);
	void updateRoomQuery();

	//Activation
	inline virtual void activate() {};
	inline virtual void deactivate() {};

	//Interaction
	inline virtual bool canInteract(Player* p) { return false; }
	inline virtual void interact(Player* p) {} 
	inline virtual string interactionIcon(Player* p) { return ""; }

	//StateMachine
	void updateFSM();
	shared_ptr<ai::Thread> addThread(shared_ptr<ai::Function> threadMain);
	void removeThread(shared_ptr<ai::Thread> t);
	void removeThread(const string& name);
	void printFSM();
	void setFrozen(bool val);

	virtual bool hit(DamageInfo damage);

	//END LOGIC

	//BEGIN PHYSICS
	
	void launch();
	void launchAtTarget(GObject* target);
    void setInitialVelocity(const SpaceVect& v);
    void setInitialAngle(SpaceFloat a);
    void setInitialAngularVelocity(SpaceFloat w);
    Vec2 getInitialCenterPix();

	virtual inline void teleport(SpaceVect pos) { setPos(pos); }

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
	inline virtual SpaceFloat uk() const { return 0.0; }
	inline GType getType() const { return type; }
	virtual inline bool getSensor() const { return false; }
	inline PhysicsLayers getLayers() const { return layers; }

	inline virtual SpaceFloat getMaxSpeed() const { return 0; }
	inline virtual SpaceFloat getMaxAcceleration() const { return 0; }

	//Called before adding the the object to space.
	virtual void initializeBody();
	inline virtual void initializeRadar(GSpace& space) {};

	void updateParametricMove();

	//END PHYSICS
    
    //BEGIN GRAPHICS
    
	bool isGraphicsObject() const;
        //The Z-order used by Cocos2D.
	virtual GraphicsLayer sceneLayer() const;
	inline virtual string getSprite() const { return ""; }
	inline virtual shared_ptr<LightArea> getLightSource() const { return nullptr; }
	int sceneLayerAsInt() const;
    virtual sprite_update updateSprite();
	void initLightSource();
	//If "id" is not provided, the object's defaut sprite, spriteID, will be
	//used, assuming it is valid.
	void addGraphicsAction(GraphicsAction action, SpriteID id = 0);
	void stopGraphicsAction(cocos_action_tag tag, SpriteID id = 0);
	void setSpriteZoom(float zoom);
	void setSpriteOpacity(unsigned char op);

	//Create Node which graphically reprensets this object and adds it to Layer
	virtual void initializeGraphics();
	
	inline virtual void setSpriteShader(const string& shaderName){
        log("GObject::setSpriteShader: virtual base, no implementation for %s!", getName());
    }

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

	virtual bool cast(shared_ptr<Spell> spell);
	void addMagicEffect(shared_ptr<MagicEffect> effect);

	void stopSpell();
	virtual void updateSpells();

	inline void setInhibitSpellcasting(bool v) { inhibitSpellcasting = v; }

	inline bool isSpellActive() const {
		return static_cast<bool>(crntSpell);
	}

	//END SPELLS

	//BEGIN SENSORY

	inline bool getInvisible() { return isInvisible; }
	inline void setInvisible(bool val) { isInvisible = val; }
	
	//END SENSORY

	GSpace *const space;
	const ObjectIDType uuid;
	const string name;

protected:
	bool hidden = false;
	bool rotateSprite = false;
	bool isInvisible = false;
	bool isInFade = false;
	bool inhibitSpellcasting = false;
	bool isFrozen = false;

//logic
	unique_ptr<ai::StateMachine> fsm;
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

	parametric_space_function parametric_f;
	SpaceFloat parametric_t = -1.0;
	parametric_type parametric_move = parametric_type::none;

	FloorSegment* crntFloorCenterContact = nullptr;
	unordered_set<FloorSegment*> crntFloorContacts;

//graphics
	SpriteID spriteID = 0;
	SpriteID drawNodeID = 0;
	LightID lightID = 0;

//spells
	shared_ptr<Spell> crntSpell;
};

#endif /* GObject_hpp */
