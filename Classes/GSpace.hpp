//
//  GSpace.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

class GObject;

//GSpace.hpp is already included in PCH

class GSpace
{
public:
    //Type signifies the collision handler as well as the group, but the group can be overidden:
    //Normally objects only collide with those of non-matching group. If we want an object to collide
    //with everything, use 0 for its group instead of its (non-zero) type.
    static const set<GType> selfCollideTypes;

    static const bool logPhysics = false;

    inline GSpace(Layer* graphicsLayer) : graphicsLayer(graphicsLayer)
    {
        space.setGravity(SpaceVect(0,0));
        addCollisionHandlers();
    }

    void addObject(const ValueMap& obj);
    void addObject(GObject*);
    void addObjects(const ValueVector& objs);
    void processAdditions();
    
    inline GObject* getObject(const string& name){
        auto it = objByName.find(name);
        return it != objByName.end() ? it->second : nullptr;
    }
    
    void removeObject(const string& name);
    void removeObject(GObject* obj);
    void processRemovals();
    
    void update();
    
    GObject* getObjectByName(const string& name);

    static shared_ptr<Body> createCircleBody(
        Space& space,
        const SpaceVect& center,
        float radius,
        float mass,
        GType type,
        int layers,
        bool sensor,
        GObject* obj
    );
    static shared_ptr<Body> createRectangleBody(
        Space& space,
        const SpaceVect& center,
        const SpaceVect& dim,
        float mass,
        GType type,
        int layers,
        bool sensor,
        GObject* obj
    );
private:
    Space space;
    //The graphics destination to use for all objects constructed in this space.
    Layer* graphicsLayer;

    map<unsigned int, GObject*> objByUUID;
    map<string, GObject*> objByName;
    
    //Objects which have been queued for addition. Will be added at end of frame.
    vector<GObject*> toAdd;
    //Objects whose additions have been processsed last frame. Physics has been initialized but
    //init has not yet run; it will run at start of frame.
    vector<GObject*> addedLastFrame;

    //Objects which have been queued for removal. Will be removed at end of frame.
    vector<GObject*> toRemove;
    
    void initObjects();
    void addCollisionHandlers();
};

#endif /* GSpace_hpp */
