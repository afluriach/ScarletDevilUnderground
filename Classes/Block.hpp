//
//  Block.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef Block_hpp
#define Block_hpp

#include "GObject.hpp"
#include "scenes.h"

class Block : virtual public GObject, RectangleBody, ImageSprite
{
public:
    inline Block(const cocos2d::ValueMap& args) : GObject(args)
    {        
        auto it = args.find("letter");
        if(it != args.end())
            letter = it->second.asString();
        else
            cocos2d::log("%s: letter undefined", name.c_str());
    }
    no_op(init)
    
    virtual string imageSpritePath() const {return "sprites/block "+letter+".png";}
    virtual GScene::Layer sceneLayer() const {return GScene::Layer::ground;}
    
    virtual inline float getMass() const { return 1;}
    virtual inline GSpace::Type getType() const {return GSpace::Type::environment;}
    
    virtual inline cp::Vect getDimensions() const {return cp::Vect(1,1);}
private:
    std::string letter;
};
#endif /* Block_hpp */
