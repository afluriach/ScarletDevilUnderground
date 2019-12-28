//
//  Goal.hpp
//  Koumachika
//
//  Created by Toni on 12/5/18.
//
//

#ifndef Goal_hpp
#define Goal_hpp

class PlayScene;

class Goal : public GObject
{
public:
	MapObjCons(Goal);
    
	virtual void update();

    virtual string getSprite() const {return "goal";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual inline string interactionIcon(Player* p) { return "sprites/ui/goal.png"; }

	virtual void activate();
	virtual void deactivate();
protected:
	SpaceFloat audioTimer = 0.0;
	PlayScene * playScene = nullptr;
	bool isBlocked = false;
};

#endif /* Goal_hpp */
