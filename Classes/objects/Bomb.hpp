//
//  Bomb.hpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#ifndef Bomb_hpp
#define Bomb_hpp

class Bomb : public GObject
{
public:
	static const SpaceFloat explosionSpriteRadius;

	Bomb(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<bomb_properties> props
	);
	~Bomb();

	virtual inline GraphicsLayer sceneLayer() const;

	virtual void init();
	virtual void update();
	void detonate();

	//Interface
	DamageInfo getDamageInfo() const;
	SpaceFloat getFuseTime() const;
	SpaceFloat getBlastRadius() const;

	string getExplosionSound() const;
	virtual shared_ptr<sprite_properties> getSprite() const;
    
	virtual bool hit(DamageInfo damage, SpaceVect n);
protected:
	local_shared_ptr<bomb_properties> props;

	SpaceFloat countdown;
	ALuint fuseSound = 0;
	bool detonated = false;
};

#endif /* Bomb_hpp */
