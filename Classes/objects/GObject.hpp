//
//  GObject.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

#include "enum.h"
#include "graphics_types.h"
#include "multifunction.h"
#include "object_ref.hpp"
#include "object_params.hpp"

class Player;
class Bullet;
class GSpace;
class Spell;
class MagicEffect;
class FloorSegment;

#define MapObjCons(cls) cls(GSpace* space, ObjectIDType id, const ValueMap& args)
#define MapObjForwarding(cls) cls(space,id,args)
#define MapObjParams() GObject(make_shared<object_params>(space,id,args))
#define ParamsCons(cls) cls(shared_ptr<object_params> params)
#define ParamsForwarding(cls) cls(params)

class GObject
{
public:
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
	static unordered_map<type_index, string> typeNameMap;

	static const float objectFadeInTime;
	static const float objectFadeOutTime;
	static const GLubyte objectFadeOpacity;

	static GObject* constructByType(GSpace* space, ObjectIDType id, const string& type, const ValueMap& args);
	static ObjectGeneratorType factoryMethodByType(const string& type, const ValueMap& args);
	static string properNameByType(type_index t);
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

	template<class ObjectCls>
	static inline ObjectGeneratorType params_object_factory(shared_ptr<object_params> params)
	{
		return[params](GSpace* space, ObjectIDType id)->GObject* {
			params->space = space;
			params->id = id;

			return new ObjectCls(params);
		};
	}

	static inline ObjectGeneratorType null_object_factory()
	{
		return[](GSpace* space, ObjectIDType id)->GObject* {
			return nullptr;
		};
	}

	GObject(shared_ptr<object_params> params);
    virtual ~GObject();

	void removePhysicsObjects();
	void removeGraphics(bool removeSprite);

	//object identification, init, and update

	string getTypeName() const;
	string getProperName() const;

	inline string getName() const {
		return (name.empty()) ? getTypeName() : name;
	}

	inline ObjectIDType getUUID() const {
		return uuid;
	}

	inline bool isAnonymous() const { return name.empty(); }
	inline bool isHidden() const { return hidden; }

	//BEGIN LOGIC

	//Called on the first frame after it has been added, before update is called on it or any other
	//objects in the same frame
	void init();
	virtual void update();

	virtual void onPitfall();
	inline virtual void onRemove() {}

	//Activation
	inline virtual void activate() {};
	inline virtual void deactivate() {};

	//Interaction
	inline virtual bool canInteract(Player* p) { return false; }
	inline virtual void interact(Player* p) {} 
	inline virtual string interactionIcon(Player* p) { return ""; }

	//END LOGIC

	//BEGIN PHYSICS
	
	void launch();
    void setInitialVelocity(const SpaceVect& v);
    void setInitialAngle(SpaceFloat a);
	void setInitialDirectionOrDefault(const ValueMap& args, Direction d);
    void setInitialAngularVelocity(SpaceFloat w);
    Vec2 getInitialCenterPix();

	virtual inline void teleport(SpaceVect pos) { setPos(pos); }

	bool isOnFloor() const;
	SpaceVect getFloorVelocity() const;
	void updateFloorSegment();
	void updateFriction(float _uk);
	void onContactFloorSegment(object_ref<FloorSegment> fs);
	void onEndContactFloorSegment(object_ref<FloorSegment> fs);

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

	PhysicsLayers getCrntLayers() const;
	void setLayers(PhysicsLayers layers);

	inline bool getBodySensor() { return cpShapeGetSensor(bodyShape); }
	inline void setBodySensor(bool val) { cpShapeSetSensor(bodyShape, val); }

	//A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
	virtual SpaceFloat getMass() const = 0;
	virtual SpaceFloat getRadius() const = 0;
	inline virtual SpaceFloat uk() const { return 0.0; }
	virtual GType getType() const = 0;
	virtual inline bool getSensor() const { return false; }
	virtual inline PhysicsLayers getLayers() const { return enum_bitwise_or(PhysicsLayers,floor,ground) ; }

	inline virtual SpaceFloat getMaxSpeed() const { return 0; }
	inline virtual SpaceFloat getMaxAcceleration() const { return 0; }

	//Called before adding the the object to space.
	virtual void initializeBody(GSpace& space) = 0;
	inline virtual void initializeRadar(GSpace& space) {};

	void updateRadarPos();

	//END PHYSICS
    
    //BEGIN GRAPHICS
    
	bool isGraphicsObject() const;
        //The Z-order used by Cocos2D.
	virtual GraphicsLayer sceneLayer() const;
	inline virtual string getSprite() const { return ""; }
	inline virtual shared_ptr<LightArea> getLightSource() const { return nullptr; }
	int sceneLayerAsInt() const;
    sprite_update updateSprite();
	void initLightSource();
	//If "id" is not provided, the object's defaut sprite, spriteID, will be
	//used, assuming it is valid.
	void addGraphicsAction(ActionGeneratorType gen, SpriteID id = 0);
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
	
	ALuint playSoundSpatial(
		const string& path,
		float volume = 1.0f,
		bool loop = false,
		float yPos = 0.0f
	);
	void stopSound(ALuint source);

	//END AUDIO

	//BEGIN SPELLS

	virtual bool cast(shared_ptr<Spell> spell);

	void stopSpell();
	virtual void updateSpells();

	inline void setInhibitSpellcasting(bool v) { inhibitSpellcasting = v; }

	inline bool isSpellActive() const {
		return static_cast<bool>(crntSpell);
	}

	virtual void addMagicEffect(shared_ptr<MagicEffect> effect);
	void updateMagicEffects();

	template<class T>
	inline bool hasMagicEffect()
	{
		for (auto it = magicEffects.begin(); it != magicEffects.end(); ++it) {
			if (dynamic_cast<T*>(it->get())) {
				return true;
			}
		}
		return false;
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

	util::multifunction<void(void)> multiInit;

//physics
	cpBody* body = nullptr;
	cpBody* radar = nullptr;

	cpShape* bodyShape = nullptr;
	cpShape* radarShape = nullptr;

	//Position where the object was loaded
	SpaceVect initialCenter;
	SpaceVect prevPos = SpaceVect::zero;
	SpaceFloat prevAngle = 0.0;

	object_ref<FloorSegment> crntFloorCenterContact;
	unordered_set<object_ref<FloorSegment>> crntFloorContacts;

//graphics
	SpriteID spriteID = 0;
	SpriteID drawNodeID = 0;
	LightID lightID = 0;

//spells
	shared_ptr<Spell> crntSpell;

	list<shared_ptr<MagicEffect>> magicEffects;
	list<shared_ptr<MagicEffect>> magicEffectsToAdd;
	list<shared_ptr<MagicEffect>> magicEffectsToRemove;
};

#endif /* GObject_hpp */
