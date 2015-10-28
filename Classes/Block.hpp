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

#include "AppDelegate.h"
#include "GameplayScene.hpp"
#include "GObject.hpp"
#include "util.h"

class Block : public GObject
{
public:
    inline Block(const cocos2d::ValueMap& args) : GObject(args)
    {
        cocos2d::log("Block created.");
        auto it = args.find("letter");
        if(it != args.end())
            letter = it->second.asString();
        else
            cocos2d::log("%s: letter undefined", name.c_str());
        
        coord.first = args.at("x").asFloat();
        coord.second = args.at("y").asFloat();

        std::string resPath = "sprites/block "+letter+".png";
        cocos2d::Node* node = cocos2d::Sprite::create(resPath);
        node->setPosition(coord.first, coord.second);
        GameplayScene::inst->addChild(node, GameplayScene::Layer::ground);
        
        if(node == nullptr)
            log("%s sprite not loaded", name.c_str());
        else
            log("%s sprite %s added at %f,%f", name.c_str(), resPath.c_str(), coord.first, coord.second);
    }
private:
    std::string letter;
    std::pair<float,float> coord;
};
#endif /* Block_hpp */
