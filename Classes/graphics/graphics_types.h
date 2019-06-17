//
//  graphic_types.h
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef graphics_types_h
#define graphics_types_h

typedef unsigned int LightID;
typedef unsigned int SpriteID;

typedef function<FiniteTimeAction*()> ActionGeneratorType;

enum class GraphicsLayer {
	begin = 1,
	map = 1,
	foliage,
	floor,
	ground,
	roomMask,
	agentOverlay,
	overhead,

	end
};

enum class cocos_action_tag
{
	illusion_dash,
	damage_flicker,
	object_fade,
	hit_protection_flicker,
	combo_mode_flicker,
	freeze_status,
	darkness_curse,

	game_over_tint,
	end,
};

class LightArea
{
public:
	SpaceVect origin;
	Color4F color;

	virtual type_index getType() const = 0;
};

class AmbientLightArea : public LightArea
{
public:
	static shared_ptr<AmbientLightArea> create(
		SpaceVect pos,
		SpaceVect dimensions,
		Color4F color
	);

	inline virtual type_index getType() const {
		return typeid(*this);
	};

	SpaceVect dimensions;
};

class CircleLightArea : public LightArea
{
public:
	static shared_ptr<CircleLightArea> create(
		SpaceVect pos,
		SpaceFloat radius,
		Color4F color,
		float flood
	);

	inline virtual type_index getType() const {
		return typeid(*this);
	};

	SpaceFloat radius;
	float flood;
};

class ConeLightArea : public LightArea
{
public:
	static shared_ptr<ConeLightArea> create(
		SpaceVect pos,
		SpaceFloat radius,
		SpaceFloat width,
		Color4F color
	);

	inline virtual type_index getType() const {
		return typeid(*this);
	};

	SpaceFloat radius;
	SpaceFloat angleWidth;
};

class SpriteLightArea : public LightArea
{
public:
	static shared_ptr<SpriteLightArea> create(
		SpaceVect pos,
		const string& spritePath,
		Color4F color,
		float scale = 1.0f
	);

	inline virtual type_index getType() const {
		return typeid(*this);
	};

	string texName;
	float scale = 1.0f;
};

struct perlin_light_state
{
	Color4F baseColor;
	float cycleInterval;
	float startAngle = 0.0f;
	float radius = 1.0f;
	float baseIntensity = 0.0f;

	float crntAngle = 0.0f;
};

struct sprite_update
{
	SpriteID spriteID;
	SpriteID drawNodeID;
	LightID lightID;

	Vec2 pos;
	float angle;

	bool fadeIn;
	bool fadeOut;
	bool rotateSprite;
	//in case update is not required
	bool valid;
};

#endif /* graphics_types_h */
