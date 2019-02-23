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
	combo_mode_flicker,
	freeze_status,

	game_over_tint,
};

extern const float fallAnimationTime;

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

class ShaderNode : public Node
{
public:
	virtual bool init();
	virtual void setContentSize(const CCSize& size) override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;

	inline void setBlendFunc(BlendFunc b) { _blendFunc = b; }

	virtual string getShaderName() const = 0;
	virtual void initUniforms() = 0;
	virtual void updateUniforms() = 0;
protected:
	void onDraw(const Mat4& transform, uint32_t flags);

	Vec2 _center;
	Vec2 _vertices[4];
	CustomCommand _customCommand;
	BlendFunc _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
};

class RadialGradient : public ShaderNode
{
public:
	RadialGradient(const Color4F& startColor, const Color4F& endColor, float radius, const Vec2& center, float expand);

	inline virtual string getShaderName() const { return "radial_gradient"; }
	virtual void initUniforms();
	virtual void updateUniforms();

protected:
	Color4F _startColor, _endColor;
	float _radius;
	float _expand;

	GLint _uniformLocationStartColor;
	GLint _uniformLocationEndColor;
	GLint _uniformLocationCenter;
	GLint _uniformLocationRadius;
	GLint _uniformLocationExpand;
};

class Cone : public ShaderNode
{
public:
	Cone(const Color4F& fillColor, const Color4F& emptyColor, float radius, const Vec2& center, float angle);

	void setAngle(float angle);
	void setColors(Color4F fill, Color4F empty);

	inline virtual string getShaderName() const { return "cone"; }
	virtual void initUniforms();
	virtual void updateUniforms();

protected:
	Color4F _fillColor, _emptyColor;
	float _radius;
	float _angle;

	GLint _uniformLocationFillColor;
	GLint _uniformLocationEmptyColor;
	GLint _uniformLocationCenter;
	GLint _uniformLocationRadius;
	GLint _uniformLocationAngle;
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
ActionGeneratorType comboFlickerTintAction();
ActionGeneratorType tintToAction(Color3B tint, float length);
ActionGeneratorType motionBlurStretch(float duration, float angle, float opacity, float scale);
ActionGeneratorType pitfallShrinkAction();
ActionGeneratorType bombAnimationAction(float expand_ratio);
ActionGeneratorType freezeEffectAction();
ActionGeneratorType freezeEffectEndAction();
ActionGeneratorType objectFadeOut(float duration, unsigned char targetOpacity);
ActionGeneratorType damageIndicatorAction(const Vec2& start_pos);

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom);

Label* createTextLabel(const string& text, int size);
Label* createTextLabel(const string& text, int size, const string& fontRes);

cocos2d::CCSize getScreenSize();

Color4F opacityScale(const Color4F& color, GLubyte opacity);

#endif /* Graphics_h */
