//
//  Sign.hpp
//  Koumachika
//
//  Created by Toni on 2/17/19.
//
//

#ifndef Sign_hpp
#define Sign_hpp

class Sign : public GObject
{
public:
	MapObjCons(Sign);

	inline virtual string getSprite() const { return "sign"; }
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	//Dialog interaction.
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual string interactionIcon(Player* p);

protected:
	string dialogRes;
};

#endif /* Sign_hpp */
