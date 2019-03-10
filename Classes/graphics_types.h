//
//  graphic_types.h
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef graphics_types_h
#define graphics_types_h

#include "types.h"

typedef unsigned int LightID;
typedef unsigned int SpriteID;

typedef function<FiniteTimeAction*()> ActionGeneratorType;

enum class SceneUpdateOrder {
	//Update tick on GSpace and all objects, if applicable
	begin = 0,
	queueActions = 0,
	updateControls,
	runShellScript,
	spaceUpdate,
	//General scene update logic
	sceneUpdate,
	moveCamera,
	spriteUpdate,
	lightmapUpdate,
	renderSpace,
	hudUpdate,

	end
};

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

	game_over_tint,
};

struct AmbientLightArea
{
	SpaceVect origin, dimensions;
	Color4F color;
};

struct CircleLightArea
{
	SpaceVect origin;
	SpaceFloat radius;
	Color4F color;
	float flood;
};

struct ConeLightArea
{
	SpaceVect origin;
	SpaceFloat radius;
	Color4F color;
	float startAngle, endAngle;
};

struct SpriteLightArea
{
	SpaceVect origin;
	string texName;
	Color4F color = Color4F::WHITE;
	float scale = 1.0f;
};

#endif /* graphics_types_h */
