//
//  Items.hpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#ifndef Items_hpp
#define Items_hpp

#include "InventoryObject.hpp"

class GraveyardKey : public InventoryObject
{
public:
    inline GraveyardKey(const ValueMap& args) : GObject(args)
    {
    }

    virtual inline string imageSpritePath() const {return "sprites/items/small grey key.png";}
    virtual inline string itemName() const {return "GraveyardKey";}
    
    inline virtual bool canAcquire() { return true; }
//    inline virtual void onAcquire() {
        //TODO trigger dialog hinting that it opens the gate?
//    }
};

#endif /* Items_hpp */
