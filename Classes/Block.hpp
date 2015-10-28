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
#include "chipmunk.h"

#include "AppDelegate.h"
#include "GameplayScene.hpp"
#include "GObject.hpp"
#include "util.h"

class Block : public GObject
{
public:
    inline Block(const cocos2d::ValueMap& args) : GObject(args)
    {
        mass = 1;
        
        cocos2d::log("Block created.");
        auto it = args.find("letter");
        if(it != args.end())
            letter = it->second.asString();
        else
            cocos2d::log("%s: letter undefined", name.c_str());
        
        cp::Vect centerPix(initialCenter);
        centerPix *= AppDelegate::pixelsPerTile;
        std::string resPath = "sprites/block "+letter+".png";
        cocos2d::Node* node = cocos2d::Sprite::create(resPath);
        node->setPosition(centerPix.x, centerPix.y);
        GameplayScene::inst->addChild(node, GameplayScene::Layer::ground);
        
        if(node == nullptr)
            log("%s sprite not loaded", name.c_str());
        else
            log("%s sprite %s added at %f,%f", name.c_str(), resPath.c_str(), centerPix.x, centerPix.y);
    }
    virtual std::shared_ptr<cp::Body> initializeBody(cp::Space& space);
private:
    std::string letter;
};
#endif /* Block_hpp */
