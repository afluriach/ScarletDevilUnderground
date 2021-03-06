//
//  Graphics.h
//  Koumachika
//
//  Created by Toni on 10/7/15.
//
//

#ifndef Graphics_h
#define Graphics_h

extern const float fallAnimationTime;

extern const float objectFadeOutTime;
extern const float objectFadeInTime;
extern const GLubyte objectFadeOpacity;

Color3B toColor3B(const Color4F& color);
Color3B toColor3B(const string& s);
Color3B hsv3B(float h, float s, float v);
Color4F hsva4F(float h, float s, float v, float a = 1.0f);
Color4F hsva4F(const string& s);
Color4F toColor4F(Color3B);
Color4F toColor4F(const string& s);
//This function accepts a color string in a number of formats.
//By default it is assumed to be 3F/4F, unless prefixed with
//"3b:" or "hsv:".
Color4F toColor(const string& s);

Color4F operator*(const Color4F& lhs, float rhs);
Color4F operator+(const Color4F& lhs, const Color4F& rhs);

FiniteTimeAction* tintTo(Color3B tint, float length);
FiniteTimeAction* flickerTint(float interval, float length, Color3B tint);

GraphicsAction indefiniteFlickerAction(float interval, unsigned char opacity1, unsigned char opacity2);
GraphicsAction indefiniteColorFlickerAction(float interval, pair<Color3B, Color3B> colors, cocos_action_tag tag);
GraphicsAction flickerAction(float interval, float length, unsigned char opacity);
GraphicsAction flickerTintAction(float interval, float length, Color3B tint);
GraphicsAction comboFlickerTintAction();
GraphicsAction spellcardFlickerTintAction();
GraphicsAction darknessCurseFlickerTintAction();
GraphicsAction tintToAction(Color3B tint, float length);
GraphicsAction motionBlurStretch(float duration, float angle, float opacity, float scale);
GraphicsAction pitfallShrinkAction();
GraphicsAction bombAnimationAction(float expand_ratio, bool removeAfter);
GraphicsAction freezeEffectAction();
GraphicsAction freezeEffectEndAction();
GraphicsAction objectFadeOut(float duration, unsigned char targetOpacity);
GraphicsAction damageIndicatorAction(const Vec2& start_pos);

float getSpriteZoom(shared_ptr<sprite_properties> sprite, SpaceFloat agentRadius);
Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom);

Label* createTextLabel(const string& text, int size);
Label* createTextLabel(const string& text, int size, const string& fontRes);

cocos2d::CCSize getScreenSize();

Color4F opacityScale(const Color4F& color, GLubyte opacity);

#endif /* Graphics_h */
