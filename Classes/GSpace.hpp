//
//  GSpace.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

#include "types.h"

class FloorSegment;
class GObject;

#define OBJS_FROM_ARB \
    GObject* a = static_cast<GObject*>(arb.getBodyA().getUserData()); \
    GObject* b = static_cast<GObject*>(arb.getBodyB().getUserData());


class GSpace
{
public:
    GSpace(Layer* graphicsLayer);    
    ~GSpace();
    
    inline IntVec2 getSize() const {return spaceSize;}
    void setSize(int x, int y);
    
    inline unsigned int getFrame(){ return frame;}
 
    void update();
    void processAdditions();
private:
    //The graphics destination to use for all objects constructed in this space.
    Layer* graphicsLayer;
    unsigned int frame = 1;
    IntVec2 spaceSize;
//BEGIN OBJECT MANIPULATION
public:
    static const bool logObjectArgs;

    GObject* addObject(const ValueMap& obj);
    GObject* addObject(GObject*);
    void addObjects(const ValueVector& objs);
    void addWallBlock(SpaceVect ll,SpaceVect ur);
    
    template<typename T>
    inline void addObjects(const vector<T*>& objects){
        static_assert(
            is_base_of<GObject, T>(),
            "addObjects: not a GObject type"
        );

        BOOST_FOREACH(T* o, objects)
            addObject(o);
    }
    
    bool isValid(unsigned int uuid) const;
    vector<string> getObjectNames() const;
    unordered_map<int,string> getUUIDNameMap() const;
    inline int getObjectCount() const { return objByUUID.size();}
    
    GObject* getObject(const string& name) const;
    GObject* getObject(unsigned int uuid) const;
    
    template<typename T>
    inline T* getObject(const string& name) const{
        static_assert(
            is_base_of<GObject, T>(),
            "getObject: not a GObject type"
        );
        return dynamic_cast<T*>(getObject(name));
    }
    
    void removeObject(const string& name);
    void removeObject(GObject* obj);

	void removeObjectWithAnimation(GObject* obj, FiniteTimeAction* action);
    
private:
    void processRemovals();
    void initObjects();
    void processRemoval(GObject* obj, bool removeSprite);
    
    unordered_map<unsigned int, GObject*> objByUUID;
    unordered_map<string, GObject*> objByName;
    
    //Objects which have been queued for addition. Will be added at end of frame.
    vector<GObject*> toAdd;
    //Objects whose additions have been processsed last frame. Physics has been initialized but
    //init has not yet run; it will run at start of frame.
    vector<GObject*> addedLastFrame;

    //Objects which have been queued for removal. Will be removed at end of frame.
    vector<GObject*> toRemove;
	vector<pair<GObject*, FiniteTimeAction*>> toRemoveWithAnimation;
//END OBJECT MANIPULATION

//BEGIN NAVIGATION
public:
    bool isObstacle(IntVec2) const;
    void addNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions);

    vector<SpaceVect> pathToTile(IntVec2 begin, IntVec2 end);
    void addPath(string name, Path p);
	const Path* getPath(string name) const;

	void addRoom(cocos2d::CCRect rect);
	vector< cocos2d::CCRect> rooms;

	FloorSegment* floorSegmentPointQuery(SpaceVect pos);

    inline boost::dynamic_bitset<>* getNavMask() const { return navMask;}
private:
    void markObstacleTile(int x, int y);
    bool isObstacleTile(int x, int y) const;
    
	unordered_map<string, Path> paths;
    boost::dynamic_bitset<>* navMask = nullptr;
//END NAVIGATION
    
//BEGIN PHYSICS
public:
	typedef pair<GObject*, GObject*> object_pair;
	typedef pair<GType, GType> collision_type;
	typedef pair<object_pair, collision_type> contact;

    static const set<GType> selfCollideTypes;

    static const bool logBodyCreation;
    static const bool logPhysicsHandlers;
    
    shared_ptr<Body> createCircleBody(
        const SpaceVect& center,
        float radius,
        float mass,
        GType type,
        PhysicsLayers layers,
        bool sensor,
        GObject* obj
    );
    shared_ptr<Body> createRectangleBody(
        const SpaceVect& center,
        const SpaceVect& dim,
        float mass,
        GType type,
        PhysicsLayers layers,
        bool sensor,
        GObject* obj
    );
private:
    Space space;

	boost::unordered_map<GObject*,list<contact>> currentContacts;
	boost::unordered_map<collision_type, int(GSpace::*)(GObject*, GObject*)> beginContactHandlers;
	boost::unordered_map<collision_type, int(GSpace::*)(GObject*, GObject*)> endContactHandlers;

	void addContact(contact c);
	void removeContact(contact c);
    void addCollisionHandlers();
    void processRemovalEndContact(GObject* obj);
    
    template<GType TypeA, GType TypeB>
	inline int beginContact(Arbiter arb, Space& space)
	{
		OBJS_FROM_ARB

		auto it = beginContactHandlers.find(collision_type(TypeA, TypeB));

		//No collide;
		if(it == beginContactHandlers.end())
			return 0;

		if (a && b && it->second) {
			int(GSpace::*begin_method)(GObject*, GObject*) = it->second;
			(this->*begin_method)(a, b);
			contact c = contact(
				object_pair(a,b),
				collision_type(TypeA,TypeB)
			);
			addContact(c);
		}

		return 1;
	}

	template<GType TypeA, GType TypeB>
	inline int endContact(Arbiter arb, Space& space)
	{
		OBJS_FROM_ARB

		auto it = endContactHandlers.find(collision_type(TypeA, TypeB));

		//No collide
		if (it == endContactHandlers.end())
			return 0;

		if (a && b && it->second) {
			int(GSpace::*end_method)(GObject*, GObject*) = it->second;
			(this->*end_method)(a, b);
			contact c = contact(
				object_pair(a, b),
				collision_type(TypeA, TypeB)
			);
			removeContact(c);
		}

		return 1;
	}

	template<GType TypeA, GType TypeB>
	inline void AddHandler(int(GSpace::*begin)(GObject*, GObject*), int(GSpace::*end)(GObject*, GObject*))
	{
		space.addCollisionHandler(
			static_cast<CollisionType>(TypeA), 
			static_cast<CollisionType>(TypeB), 
			bind(&GSpace::beginContact<TypeA, TypeB>, this, placeholders::_1, placeholders::_2), 
			nullptr, 
			nullptr, 
			bind(&GSpace::endContact<TypeA, TypeB>, this, placeholders::_1, placeholders::_2) 
		); 

		beginContactHandlers[collision_type(TypeA, TypeB)] = begin;
		endContactHandlers[collision_type(TypeA,TypeB)] = end;
	}
    
    void logHandler(const string& base, Arbiter& arb);
    void logHandler(const string& name, GObject* a, GObject* b);
    
	int playerEnemyBegin(GObject* a, GObject* b);
	int playerEnemyEnd(GObject* a, GObject* b);
	int playerEnemyBulletBegin(GObject* playerObj, GObject* bullet);
	int playerBulletEnemyBegin(GObject* a, GObject* b);
	int playerFlowerBegin(GObject* a, GObject* b);
    int playerCollectibleBegin(GObject* a, GObject* b);
	int bulletEnvironment(GObject* a, GObject* b);
	int noCollide(GObject* a, GObject* b);
	int collide(GObject* a, GObject* b);
	int bulletWall(GObject* bullet, GObject* unused);
	int sensorStart(GObject* radarAgent, GObject* target);
	int sensorEnd(GObject* radarAgent, GObject* target);

	int floorObjectBegin(GObject* floorSegment, GObject* obj);
	int floorObjectEnd(GObject* floorSegment, GObject* obj);
	//END PHYSICS

//BEGIN SENSORS
public:
	GObject * pointQuery(SpaceVect pos, GType type, PhysicsLayers layers);

    float distanceFeeler(const GObject * agent, SpaceVect feeler, GType gtype) const;
    float obstacleDistanceFeeler(const GObject * agent, SpaceVect feeler) const;
    float wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const;
    
    bool feeler(const GObject * agent, SpaceVect feeler, GType gtype) const;
    bool feeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const;
    bool obstacleFeeler(const GObject * agent, SpaceVect feeler) const;
    bool wallFeeler(const GObject * agent, SpaceVect feeler) const;
    
    bool lineOfSight(const GObject * agent, const GObject * target) const;
//END SENSORS
};

#endif /* GSpace_hpp */
