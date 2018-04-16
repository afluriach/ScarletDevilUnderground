//
//  Block.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef Block_hpp
#define Block_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Block : virtual public GObject, RectangleBody, ImageSprite, FrictionObject
{
public:
    const bool isStatic;
    const float mass = 40.0;
    inline Block(const ValueMap& args) :
    GObject(args),
    isStatic(args.find("static") != args.end())
    {
        auto it = args.find("letter");
        if(it != args.end())
            letter = it->second.asString();
//        else
//            log("%s: letter undefined", name.c_str());
    }
    
    virtual string imageSpritePath() const {return !letter.empty() ? "sprites/block "+letter+".png"  : "sprites/block.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline float getMass() const { return isStatic ? -1 : mass;}
    virtual inline float uk() const {return 0.5;}
    virtual inline GType getType() const {return GType::environment;}
    
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}
private:
    string letter;
};
#endif /* Block_hpp */
