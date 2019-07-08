//
//  Graphics.cpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "Graphics.h"

const string defaultFont = "Arial";

const float fallAnimationTime = 1.0f;

const float objectFadeOutTime = 1.5f;
const float objectFadeInTime = 0.5f;
const GLubyte objectFadeOpacity = 0;

//Qualify Size for ambiguity.
cocos2d::CCSize getScreenSize()
{
    return Director::getInstance()->getVisibleSize();
}

Color4F toColor(const string& s)
{
	vector<string> t1 = splitString(s, ":");
	Color4F result;

	if (t1.size() > 1 && boost::iequals(t1[0], "3b"))
	{
		result = toColor4F(toColor3B(t1[1]));
	}
	else if (t1.size() > 1 && boost::iequals(t1[0],"hsv"))
	{
		result = hsva4F(t1[1]);
	}
	else
	{
		result = toColor4F(t1.back());
	}

	return result;
}

Color3B toColor3B(const string& s)
{
	vector<string> tokens = splitString(s, ",");

	if (tokens.size() != 3) {
		log("toColor3B: invalid string %s", s);
		return Color3B::BLACK;
	}

	return Color3B(
		boost::lexical_cast<int>(tokens[0]),
		boost::lexical_cast<int>(tokens[1]),
		boost::lexical_cast<int>(tokens[2])
	);
}

Color3B hsv3B(float h, float s, float v)
{
	return toColor3B(hsva4F(h, s, v));
}

Color4F hsva4F(float h, float s, float v, float a)
{
	float r1, g1, b1;
	float C = v * s;
	float hPrime = h / 60.0f;
	float x = C * (1.0f - abs(fmod(hPrime,2.0f) - 1.0f));
	float m = v - C;

	if (s == 0)
	{
		//hue is undefined and no color will be added
		r1 = g1 = b1 = 0;
	}
	else if (0 <= hPrime && hPrime < 1)
	{
		r1 = C;
		g1 = x;
		b1 = 0;
	}
	else if (1 <= hPrime && hPrime < 2)
	{
		r1 = x;
		g1 = C;
		b1 = 0;
	}
	else if (2 <= hPrime && hPrime < 3)
	{
		r1 = 0;
		g1 = C;
		b1 = x;
	}
	else if (3 <= hPrime && hPrime < 4)
	{
		r1 = 0;
		g1 = x;
		b1 = C;
	}
	else if (4 <= hPrime && hPrime < 5)
	{
		r1 = x;
		g1 = 0;
		b1 = C;
	}
	else if (5 <= hPrime && hPrime < 6)
	{
		r1 = C;
		g1 = 0;
		b1 = x;
	}
	else
	{
		log("Illegal hue given: %f", h);
		return Color4F();
	}

	return Color4F(r1 + m, g1 + m, b1 + m, a);
}

Color4F hsva4F(const string& s)
{
	vector<string> tokens = splitString(s, ",");

	if (tokens.size() != 3 && tokens.size() != 4) {
		log("hsva4F: invalid string %s", s);
		return Color4F::BLACK;
	}

	return hsva4F(
		boost::lexical_cast<float>(tokens[0]),
		boost::lexical_cast<float>(tokens[1]),
		boost::lexical_cast<float>(tokens[2]),
		tokens.size() == 3 ? 1.0f : boost::lexical_cast<float>(tokens[3])
	);
}

Color3B toColor3B(const Color4F& color)
{
	return Color3B(color.r * 255, color.g * 255, color.b * 255);
}

Color4F toColor4F(Color3B color)
{
	return Color4F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f);
}

Color4F toColor4F(const string& s)
{
	vector<string> tokens = splitString(s, ",");

	if (tokens.size() != 3 && tokens.size() != 4) {
		log("toColor4F: invalid string %s", s);
		return Color4F::BLACK;
	}

	return Color4F(
		boost::lexical_cast<float>(tokens[0]),
		boost::lexical_cast<float>(tokens[1]),
		boost::lexical_cast<float>(tokens[2]),
		tokens.size() == 3 ? 1.0f : boost::lexical_cast<float>(tokens[3])
	);
}

Color4F operator*(const Color4F& lhs, float rhs)
{
	return Color4F(lhs.r * rhs, lhs.g * rhs, lhs.b * rhs, lhs.a);
}

Color4F operator+(const Color4F& lhs, const Color4F& rhs)
{
	return Color4F(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a);
}

FiniteTimeAction* flickerTint(float interval, float length, Color3B tint)
{
	int nCycles = length / interval;

	Sequence* flicker = Sequence::createWithTwoActions(
		TintTo::create(interval / 2, tint.r, tint.g, tint.b),
		TintTo::create(interval / 2, 255, 255, 255)
	);

	Repeat* loop = Repeat::create(flicker, nCycles);

	return loop;
}

ActionGeneratorType indefiniteFlickerAction(float interval, unsigned char opacity1, unsigned char opacity2)
{
	return [=]() -> FiniteTimeAction* {
		Sequence* flicker = Sequence::createWithTwoActions(
			FadeTo::create(interval / 2, opacity1),
			FadeTo::create(interval / 2, opacity2)
		);
		RepeatForever* loop = RepeatForever::create(flicker);

		return loop;
	};
}

ActionGeneratorType indefiniteColorFlickerAction(float interval, pair<Color3B,Color3B> colors, cocos_action_tag tag)
{
	return [=]() -> FiniteTimeAction* {
		Sequence* flicker = Sequence::createWithTwoActions(
			TintTo::createRecursive(interval * 0.5f, colors.first),
			TintTo::createRecursive(interval * 0.5f, colors.second)
		);

		RepeatForever* loop = RepeatForever::create(flicker);
		loop->setTag(to_int(tag));

		return loop;
	};
}

ActionGeneratorType flickerAction(float interval, float length, unsigned char opacity)
{
	return [interval,length,opacity]() -> FiniteTimeAction* {
		int nCycles = length / interval;

		Sequence* flicker = Sequence::createWithTwoActions(FadeTo::create(interval / 2, opacity), FadeTo::create(interval / 2, 255));
		Repeat* loop = Repeat::create(flicker, nCycles);

		return loop;
	};
}

ActionGeneratorType flickerTintAction(float interval, float length, Color3B tint)
{
	return [interval,length,tint]() -> FiniteTimeAction* {
		int nCycles = length / interval;

		Sequence* flicker = Sequence::createWithTwoActions(
			TintTo::create(interval / 2, tint.r, tint.g, tint.b),
			TintTo::create(interval / 2, 255, 255, 255)
		);

		Repeat* loop = Repeat::create(flicker, nCycles);

		return loop;
	};
}

ActionGeneratorType comboFlickerTintAction()
{
	return indefiniteColorFlickerAction(
		0.25f,
		make_pair(Color3B(192, 160, 96), Color3B::WHITE),
		cocos_action_tag::combo_mode_flicker
	);
}

ActionGeneratorType spellcardFlickerTintAction()
{
	return indefiniteColorFlickerAction(
		0.4f,
		make_pair(Color3B(94, 145, 140), Color3B::GRAY),
		cocos_action_tag::end
	);
}

ActionGeneratorType darknessCurseFlickerTintAction()
{
	return indefiniteColorFlickerAction(
		0.4f,
		make_pair(hsv3B(285.0f, 0.5f, 0.5f), Color3B::WHITE),
		cocos_action_tag::darkness_curse
	);
}

FiniteTimeAction* tintTo(Color3B tint, float length)
{
	return TintTo::createRecursive(length, tint);
}

ActionGeneratorType tintToAction(Color3B tint, float length)
{
	return [tint, length]() -> FiniteTimeAction* {
		return TintTo::createRecursive(length, tint);
	};
}

ActionGeneratorType pitfallShrinkAction()
{
	return []() -> FiniteTimeAction* {
		return ScaleTo::create(fallAnimationTime, 0.0f);
	};
}

ActionGeneratorType bombAnimationAction(float expand_ratio, bool removeAfter)
{
	return [expand_ratio, removeAfter]() -> FiniteTimeAction* {

		FadeTo* fade = FadeTo::create(0.0f, 64);
		ScaleTo* expand = ScaleTo::create(0.125f, expand_ratio);

		Sequence* flicker = Sequence::createWithTwoActions(FadeTo::create(0.125f, 64), FadeTo::create(0.125f, 192));
		Repeat* loop = Repeat::create(flicker, 4);

		if (removeAfter) {
			return Sequence::create(
				fade,
				expand,
				loop,
				RemoveSelf::create(),
				nullptr
			);
		}
		else {
			return Sequence::create(
				fade,
				expand,
				loop,
				nullptr
			);
		}
	};
}

ActionGeneratorType motionBlurStretch(float duration, float angle, float opacity, float scale)
{
	return [duration,angle,opacity,scale]() -> FiniteTimeAction* {
		Sequence* sequence = Sequence::create(
			FadeTo::create(0.0f, opacity * 255),
			DelayTime::create(duration),
			FadeTo::create(0.0f, 255),
			nullptr
		);

		return sequence;
	};
}

ActionGeneratorType freezeEffectAction()
{
	return []() -> FiniteTimeAction* {
		FiniteTimeAction* action = TintTo::createRecursive(0.5f, Color3B(64, 64, 255));
		action->setTag(to_int(cocos_action_tag::freeze_status));
		return action;
	};
}

ActionGeneratorType freezeEffectEndAction()
{
	return []() -> FiniteTimeAction* {
		FiniteTimeAction* action = TintTo::createRecursive(0.5f, Color3B(255, 255, 255));
		action->setTag(to_int(cocos_action_tag::freeze_status));
		return action;
	};
}

ActionGeneratorType objectFadeOut(float duration, unsigned char targetOpacity)
{
	return [duration,targetOpacity]() -> FiniteTimeAction* {
		FiniteTimeAction* action = FadeTo::create(duration, targetOpacity);
		action->setTag(to_int(cocos_action_tag::object_fade));
		return action;
	};
}

ActionGeneratorType damageIndicatorAction(const Vec2& start_pos)
{
	Vec2 end_pos = start_pos + Vec2(0.0f, app::pixelsPerTile);
	return [end_pos]() -> FiniteTimeAction* {
		MoveTo* moveTo = MoveTo::create(1.0f, end_pos);
		FadeOut* fadeOut = FadeOut::create(0.5f);
		RemoveSelf* removeSelf = RemoveSelf::create();

		return Sequence::create(moveTo, fadeOut, removeSelf, nullptr);
	};
}

float getSpriteZoom(const sprite_properties& sprite, SpaceFloat agentRadius)
{
	float zoom = 1.0f * app::pixelsPerTile / sprite.dpi;
	if (sprite.referenceSize > 0)
		zoom *= agentRadius / sprite.referenceSize * 2.0f;

	return zoom;
}

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom)
{
    Sprite* node = Sprite::create(resPath);
    
    if(node)
        dest->positionAndAddNode(node,to_int(sceneLayer),pos, zoom);
    else
        log("loadImageSprite: sprite %s not loaded", resPath.c_str());

    return node;
}

Label* createTextLabel(const string& text, int size)
{
	return createTextLabel(text, size, "fonts/comfortaa.ttf");
}

Label* createTextLabel(const string& text, int size, const string& fontRes)
{
	return Label::createWithTTF(text, fontRes, size);
}

Color4F opacityScale(const Color4F& color, GLubyte opacity)
{
	return Color4F(color.r, color.g, color.b, color.a * opacity / 255);
}
