//
//  SpaceLayer.h
//  Koumachika
//
//  Created by Toni on 12/12/18.
//
//

#ifndef SpaceLayer_h
#define SpaceLayer_h

#include "types.h"

class SpaceLayer : public Layer
{
public:
	SpaceLayer();
	virtual bool init();

	Layer* getLayer(GraphicsLayer);
protected:
	//Make sure to use a cocos map so cocos refcounting works.
	cocos2d::Map<int, Layer*> layers;
};

#endif /* SpaceLayer_h */
