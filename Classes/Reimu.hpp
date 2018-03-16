//
//  Reimu.hpp
//  Koumachika
//
//  Created by Toni on 3/15/16.
//
//

#ifndef Reimu_h
#define Reimu_h

class Reimu : virtual public GObject, PatchConSprite, CircleBody, DialogEntity
{
public:
    inline Reimu(const ValueMap& args) :
    GObject(args),
    PatchConSprite(args)
    {}
    
    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return -1;}
    virtual inline GType getType() const {return GType::enemy;}
    
    inline string imageSpritePath() const {return "sprites/reimu.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    inline virtual bool isDialogAvailable(){ return true;}
    inline virtual string getDialog(){
        return "dialogs/warning_about_rumia";
    }
};

#endif /* Reimu_h */
