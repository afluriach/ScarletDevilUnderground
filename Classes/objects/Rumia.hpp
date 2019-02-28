//
//  Rumia.hpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#ifndef Rumia_hpp
#define Rumia_hpp

#include "Enemy.hpp"

class Rumia : public Enemy, public BaseAttributes<Rumia>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(Rumia);

    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 20.0;}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
    inline string imageSpritePath() const {return "sprites/rumia.png";}
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine(ai::StateMachine& fsm);
};

#endif /* Rumia_hpp */
