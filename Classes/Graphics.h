//
//  Graphics.h
//  Koumachika
//
//  Created by Toni on 10/7/15.
//
//

#ifndef Graphics_h
#define Graphics_h

#include "types.h"

enum class cocos_action_tag
{
	illusion_dash,
	damage_flicker,
	object_fade,
	hit_protection_flicker,
	freeze_status,

	game_over_tint,
};

extern const float fallAnimationTime;

struct AmbientLightArea
{
	SpaceVect origin, dimensions;
	Color3B color;
	float intensity;
};

struct CircleLightArea
{
	SpaceVect origin;
	SpaceFloat radius;
	Color3B color;
	float intensity;
	float flood;
};

struct ConeLightArea
{
	SpaceVect origin;
	SpaceFloat radius;
	Color3B color;
	float intensity;
	float startAngle, endAngle;
};

class RadialGradient : public Node
{
public:
	RadialGradient(const Color4F& startColor, const Color4F& endColor, float radius, const Vec2& center, float expand);

	virtual bool init();
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
	virtual void setContentSize(const CCSize& size) override;

	inline void setBlendFunc(BlendFunc b) { _blendFunc = b; }

protected:
	void onDraw(const Mat4& transform, uint32_t flags);

	Vec2 _center;
	Vec2 _vertices[4];
	Color4F _startColor, _endColor;
	float _radius;
	float _expand;

	CustomCommand _customCommand;
	BlendFunc _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;

	GLint _uniformLocationStartColor;
	GLint _uniformLocationEndColor;
	GLint _uniformLocationCenter;
	GLint _uniformLocationRadius;
	GLint _uniformLocationExpand;
};

class Cursor : public Node
{
public:
	static const Color4F colors[6];

    virtual void onEnter();
    virtual void update(float dt);
    void reset();
protected:
    virtual void drawShape() = 0;
    //The length of time for the diamond to expand or shrink fully.
    const float interval = 0.5;
    //Base size of shape.
    const int halfSize = 20;

	int scaledSize();
        
    DrawNode * drawNode;
    bool expanding=false;
    float scale = 1;
    int crntColor=0;
};

class DiamondCursor : public Cursor
{
public:
	//Y scale, with is also y/x ratio
	static const float ratio;
    virtual void onEnter();
protected:
    virtual void drawShape();
};

class DownTriangleCursor : public Cursor
{
public:
    virtual void drawShape();
};

Color3B toColor3B(const Color4F& color);
Color4F toColor4F(Color3B);

Color4F operator*(const Color4F& lhs, float rhs);

FiniteTimeAction* tintTo(Color3B tint, float length);
FiniteTimeAction* flickerTint(float interval, float length, Color3B tint);

ActionGeneratorType flickerAction(float interval, float length, unsigned char opacity);
ActionGeneratorType flickerTintAction(float interval, float length, Color3B tint);
ActionGeneratorType tintToAction(Color3B tint, float length);
ActionGeneratorType motionBlurStretch(float duration, float angle, float opacity, float scale);
ActionGeneratorType pitfallShrinkAction();
ActionGeneratorType freezeEffectAction();
ActionGeneratorType freezeEffectEndAction();
ActionGeneratorType objectFadeOut(float duration, unsigned char targetOpacity);

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom);

Label* createTextLabel(const string& text, int size);
Label* createTextLabel(const string& text, int size, const string& fontRes);

cocos2d::CCSize getScreenSize();

Color4F opacityScale(const Color4F& color, GLubyte opacity);

#endif /* Graphics_h */
