//
//  Sign.hpp
//  Koumachika
//
//  Created by Toni on 2/17/19.
//
//

#ifndef Sign_hpp
#define Sign_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Sign :
	virtual public GObject,
	public RectangleBody,
	public DialogEntity
{
public:
	MapObjCons(Sign);

	inline virtual string getSprite() const { return "sign"; }
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
	inline virtual PhysicsLayers getLayers() const { return PhysicsLayers::all; }

	inline virtual bool isDialogAvailable() { return true; }
	inline virtual string getDialog() { return "dialogs/"+dialogRes; }
protected:
	string dialogRes;
};

#endif /* Sign_hpp */
