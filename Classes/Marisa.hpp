//
//  Marisa.h
//  Koumachika
//
//  Created by Toni on 12/12/15.
//
//

#ifndef Marisa_h
#define Marisa_h

class Marisa :
virtual public GObject,
virtual ScriptedObject,
virtual RadarObject,
ScriptedRadar,
PatchConSprite,
CircleBody,
Spellcaster
{
public:
    inline Marisa(const ValueMap& args) :
    ScriptedObject("marisa"),
    PatchConSprite(args),
    GObject(args){
    }
    
    virtual inline float getRadarRadius() const {return 6;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    
    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 33.0;}
    virtual inline GType getType() const {return GType::enemy;}
    
    inline string imageSpritePath() const {return "sprites/marisa.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
};

#endif /* Marisa_h */
