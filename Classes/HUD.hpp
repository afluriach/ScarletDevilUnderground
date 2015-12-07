//
//  HUD.hpp
//  FlansBasement
//
//  Created by Toni on 12/7/15.
//
//

#ifndef HUD_hpp
#define HUD_hpp

#include "Player.hpp"

class HealthBar : public Node
{
public:
    static const int heartSize = 32;

    CREATE_FUNC(HealthBar);
    void setMax(int);
    void setValue(int);
protected:
    Vector<Sprite*> heartSprites;
    int crntVal = 0;
};

class HUD : public Layer
{
public:
    static const int height = 50;
    
    static const Color4F backgroundColor;

    CREATE_FUNC(HUD);
    HUD();
    //Not an override of Layer
    void update();
    virtual bool init();

    HealthBar* health;
protected:
    Player* player;
};

#endif /* HUD_hpp */
