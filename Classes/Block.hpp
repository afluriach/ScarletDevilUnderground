//
//  Block.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef Block_hpp
#define Block_hpp

#include "cocos2d.h"

#include "App.h"
#include "PlayScene.hpp"
#include "GObject.hpp"
#include "util.h"

class Block : virtual public GObject, RectangleBody, ImageSprite
{
public:
    inline Block(const cocos2d::ValueMap& args) : GObject(args)
    {
        GObject::mass = 1;
        
        cocos2d::log("Block created.");
        auto it = args.find("letter");
        if(it != args.end())
            letter = it->second.asString();
        else
            cocos2d::log("%s: letter undefined", name.c_str());
    }
    no_op(init)
    
    virtual string imageSpritePath() const {return "sprites/block "+letter+".png";}
    virtual PlayScene::Layer sceneLayer() const {return PlayScene::Layer::ground;}

private:
    std::string letter;
};
#endif /* Block_hpp */
