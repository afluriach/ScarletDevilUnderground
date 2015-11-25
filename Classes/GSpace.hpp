//
//  GSpace.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

#include "Prefix.h"

using namespace std;
USING_NS_CC;

class GObject;

class GSpace
{
public:
    enum Type{
        player=1,
        playerBullet,
        environment
    };
    
    //Layers are interpreted as a bitmask.
    //For now, multilayer physics is not being used.
    enum Layers{
        ground = 1,
    };

    //Type signifies the collision handler as well as the group, but the group can be overidden:
    //Normally objects only collide with those of non-matching group. If we want an object to collide
    //with everything, use 0 for its group instead of its (non-zero) type.
    static const set<Type> selfCollideTypes;

    static const bool logPhysics = false;

    inline GSpace(cocos2d::Layer* graphicsLayer) : graphicsLayer(graphicsLayer)
    {
        space.setGravity(cp::Vect(0,0));
    }

    void addObject(const ValueMap& obj);
    void addObject(GObject*);
    void addObjects(const ValueVector& objs);
    void processAdditions();
    
    void update();
    

    static std::shared_ptr<cp::Body> createCircleBody(
        cp::Space& space,
        const cp::Vect& center,
        float radius,
        float mass,
        GSpace::Type type,
        int layers,
        bool sensor,
        GObject* obj
    );
    static std::shared_ptr<cp::Body> createRectangleBody(
        cp::Space& space,
        const cp::Vect& center,
        const cp::Vect& dim,
        float mass,
        GSpace::Type type,
        int layers,
        bool sensor,
        GObject* obj
    );
private:
    cp::Space space;
    //The graphics destination to use for all objects constructed in this space.
    cocos2d::Layer* graphicsLayer;
    
    vector<GObject*> objects;
    map<string, GObject*> objByName;
    map<string, vector<GObject*>> objsByType;
    
    //Objects which have been queued for addition. Will be added at end of frame.
    vector<GObject*> toAdd;
    //Objects whose additions have been processsed last frame. Physics has been initialized but
    //init has not yet run; it will run at start of frame.
    vector<GObject*> addedLastFrame;
    
    void initObjects();
};

#endif /* GSpace_hpp */
