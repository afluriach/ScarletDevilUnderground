//
//  Block.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef Block_hpp
#define Block_hpp

class Block : virtual public GObject, RectangleBody, ImageSprite
{
public:
    inline Block(const ValueMap& args) : GObject(args)
    {        
        auto it = args.find("letter");
        if(it != args.end())
            letter = it->second.asString();
        else
            log("%s: letter undefined", name.c_str());
    }
    
    virtual string imageSpritePath() const {return "sprites/block "+letter+".png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline float getMass() const { return 1;}
    virtual inline GType getType() const {return GType::environment;}
    
    virtual inline cp::Vect getDimensions() const {return cp::Vect(1,1);}
private:
    string letter;
};
#endif /* Block_hpp */
