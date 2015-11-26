//
//  Player.hpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Player_hpp
#define Player_hpp

class Player : virtual public GObject, PatchConSprite, CircleBody, RegisterUpdate<Player>
{
public:
    static constexpr float fireDist = 1;

    inline Player(const ValueMap& args) : GObject(args), RegisterUpdate<Player>(this) {
    }
    
    inline float getSpeed() const{
        return 3;
    };

    inline float getFireInterval() const {
        return 0.6;
    }
    
    inline void update(){
        updateFireTime();
        checkControls();
    }

    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 1;}
    virtual inline GType getType() const {return GType::player;}
    
    inline string imageSpritePath() const {return "sprites/flandre.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    void setDirection(Direction);
    void updateFireTime();
    void checkControls();
    void fireIfPossible();
    void fire();
protected:
    float lastFireTime = 0;
};

#endif /* Player_hpp */
