//
//  Patchouli.hpp
//  FlansBasement
//
//  Created by Toni on 11/27/15.
//
//

#ifndef Patchouli_hpp
#define Patchouli_hpp

class Patchouli : virtual public GObject, PatchConSprite, CircleBody
{
public:
    inline Patchouli(const ValueMap& args) : GObject(args){
        multiUpdate += bind(&Patchouli::update, this);
    }
    
    void update();
    
    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 1;}
    virtual inline GType getType() const {return GType::enemy;}
    
    inline string imageSpritePath() const {return "sprites/patchouli.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
protected:
    float timeSinceFire = 0;
};

#endif /* Patchouli_hpp */
