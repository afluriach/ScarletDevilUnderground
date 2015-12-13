//
//  Marisa.h
//  FlansBasement
//
//  Created by Toni on 12/12/15.
//
//

#ifndef Marisa_h
#define Marisa_h

class Marisa : virtual public GObject, PatchConSprite, CircleBody, Spellcaster, ScriptedObject
{
public:
    inline Marisa(const ValueMap& args) :
    ScriptedObject("marisa"),
    GObject(args){
    }
    
    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 1;}
    virtual inline GType getType() const {return GType::enemy;}
    
    inline string imageSpritePath() const {return "sprites/marisa.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
};

#endif /* Marisa_h */
