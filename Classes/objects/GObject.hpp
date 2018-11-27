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
#include "multifunction.h"
#include "types.h"

class Player;
class Bullet;
class GSpace;
namespace Lua{class Inst;}
class Spell;
class FloorSegment;

class GObject
{
public:
    typedef function<GObject*( const ValueMap&) > AdapterType;
    
    static constexpr bool logCreateObjects = false;
	//Map each class name to a constructor adapter function.
	static const unordered_map<string, AdapterType> adapters;
	static unsigned int nextUUID;

	static GObject* constructByType(const string& type, const ValueMap& args);
    
    template<typename T>
    inline bool isType(){
        return dynamic_cast<T*>(this) != nullptr;
    }
	
	//Representation as a map object
    GObject(const ValueMap& args);
	GObject(const ValueMap& args, bool anonymous);
	GObject(const string& name, const SpaceVect& pos, bool anonymous);
	GObject(const string& name, const SpaceVect& pos, float angle, bool anonymous);
    
    virtual ~GObject();

	//object identification, init, and update
	const string name;
    const bool anonymous = false;
	const unsigned int uuid;

	util::multifunction<void(void)> multiInit;
	util::multifunction<void(void)> multiUpdate;

	inline string getName() const {
		return name;
	}

	inline unsigned int getUUID() const {
		return uuid;
	}

	inline static void resetObjectUUIDs() {
		nextUUID = 1;
	}

	//Called on the first frame after it has been added, before update is called on it or any other
	//objects in the same frame
	void init();
	void update();
    
	//BEGIN PHYSICS

	shared_ptr<Body> body;
	shared_ptr<Body> radar;

    //Posiition where the object was loaded
    SpaceVect initialCenter;
	//Tracks setting of physics properties, they will be applied before physics step.
	PhysicsProperties physicsPropertiesToApply;

	FloorSegment* crntFloor = nullptr;
	//Set by the physics callback, effect will apply on next update.
	FloorSegment* nextFloor = nullptr;

    void setInitialVelocity(const SpaceVect&& v);
    void setInitialAngle(float a);
    void setInitialAngularVelocity(float w);
    Vec2 getInitialCenterPix();

	void applyPhysicsProperties();
	void updateFloorSegment();

    SpaceVect getPos() const;
    void setPos(float x, float y);

    virtual void setAngle(float a);
    float getAngle() const;
    
    void rotate(float a);
    SpaceVect getFacingVector() const;
    virtual void setDirection(Direction d);
    
    SpaceVect getVel() const;
    void setVel(SpaceVect v);
    
    float getAngularVel()const;
    void setAngularVel(float w);
    
    //Apply a force as impulse where t = frame length.
    void applyForceForSingleFrame(SpaceVect f);
    void applyImpulse(float mag, float angle);

	void setLayers(PhysicsLayers layers);

	//A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
	virtual float getMass() const = 0;
	virtual float getRadius() const = 0;
    virtual float getMomentOfInertia() const = 0;
	virtual GType getType() const = 0;
	virtual inline bool getSensor() const { return false; }
	virtual inline PhysicsLayers getLayers() const { return enum_bitwise_or(PhysicsLayers,floor,ground) ; }

	inline virtual float getMaxSpeed() const { return 0; }
	inline virtual float getMaxAcceleration() const { return 0; }

	//Called before adding the the object to space.
	virtual void initializeBody(GSpace& space) = 0;
	inline virtual void initializeRadar(GSpace& space) {};

	//Create Node which graphically reprensets this object and adds it to Layer
	virtual void initializeGraphics(Layer* layer) = 0;

	//END PHYSICS
    
	//BEGIN LUA
	
    #define if_lua_ctx if(!anonymous && ctx) 
    
	unique_ptr<Lua::Inst> ctx;

	inline virtual string getScriptName() const { return ""; }

    string getScriptVal(string name);
    void setScriptVal(string field, string val);
    string _callScriptVal(string field, string args);
    void runLuaInit();
    void runLuaUpdate();
	void setupLuaContext();

	//END LUA
    
    //BEGIN GRAPHICS
    
    Node* sprite = nullptr;
    
        //The Z-order used by Cocos2D.
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    inline int sceneLayerAsInt(){return static_cast<int>(sceneLayer());};
    inline virtual float zoom() const {return 1.0f;}
    void updateSprite();

    inline virtual void setSpriteShader(const string& shaderName){
        log("GObject::setSpriteShader: virtual base, no implementation for %s!", name.c_str());
    }

    //END GRAPHICS

	//BEGIN SPELLS

	void cast(unique_ptr<Spell> spell);
	void cast(const string& name, const ValueMap& args);
	//For Luabridge
	void castByName(string name, const ValueMap& args);

	void stopSpell();
	void updateSpells();

	inline bool isSpellActive() const {
		return static_cast<bool>(crntSpell);
	}

	unique_ptr<Spell> crntSpell;

	//END SPELLS

	//BEGIN SENSORY

	bool isInvisible = false;

	inline bool getInvisible() { return isInvisible; }
	inline void setInvisible(bool val) { isInvisible = val; }
	
	//END SENSORY
};

#endif /* GObject_hpp */
