//
//  Graphics.cpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "Graphics.h"
#include "macros.h"

const string defaultFont = "Arial";

const float fallAnimationTime = 1.0f;

//Qualify Size for ambiguity.
cocos2d::CCSize getScreenSize()
{
    return Director::getInstance()->getVisibleSize();
}

RadialGradient::RadialGradient(const Color4F& startColor, const Color4F& endColor, float radius, const Vec2& center, float expand) :
	_startColor(startColor),
	_endColor(endColor),
	_radius(radius),
	_center(center),
	_expand(expand)
{
}

bool RadialGradient::init()
{
	_blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;

	for (int i = 0; i < 4; ++i)
		_vertices[i] = { 0.0f, 0.0f };

	Node::init();

	setGLProgramState(GLProgramState::getOrCreateWithGLProgramName("radial_gradient"));
	auto program = getGLProgram();
	_uniformLocationStartColor = program->getUniformLocation("u_startColor");
	_uniformLocationEndColor = program->getUniformLocation("u_endColor");
	_uniformLocationExpand = program->getUniformLocation("u_expand");
	_uniformLocationRadius = program->getUniformLocation("u_radius");
	_uniformLocationCenter = program->getUniformLocation("u_center");

	return true;
}

void RadialGradient::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	_customCommand.init(_globalZOrder, transform, flags);
	_customCommand.func = CC_CALLBACK_0(RadialGradient::onDraw, this, transform, flags);
	renderer->addCommand(&_customCommand);
}

void RadialGradient::setContentSize(const CCSize& size)
{
	_vertices[0] = Vec2(-size.width / 2, -size.height / 2);
	_vertices[1] = Vec2(size.width / 2, -size.height / 2);
	_vertices[2] = Vec2(-size.width / 2, size.height / 2);
	_vertices[3] = Vec2(size.width / 2, size.height / 2);

	Node::setContentSize(size);
}

void RadialGradient::onDraw(const Mat4& transform, uint32_t flags)
{
	auto program = getGLProgram();
	program->use();
	program->setUniformsForBuiltins(transform);
	program->setUniformLocationWith4f(_uniformLocationStartColor, _startColor.r,
		_startColor.g, _startColor.b, _startColor.a);
	program->setUniformLocationWith4f(_uniformLocationEndColor, _endColor.r,
		_endColor.g, _endColor.b, _endColor.a);
	program->setUniformLocationWith2f(_uniformLocationCenter, _center.x, _center.y);
	program->setUniformLocationWith1f(_uniformLocationRadius, _radius);
	program->setUniformLocationWith1f(_uniformLocationExpand, _expand);

	GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POSITION);

	//
	// Attributes
	//
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, _vertices);

	GL::blendFunc(_blendFunc.src, _blendFunc.dst);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 4);
}

const Color4F Cursor::colors[6] = {
	Color4F(1.0f,0.0f,0.0f,1.0f),
	Color4F(0.8f,0.4f,0.0f,1.0f),
	Color4F(0.8f,0.8f,0.0f,1.0f),
	Color4F(0.0f,0.8f,0.0f,1.0f),
	Color4F(0.0f,0.0f,1.0f,1.0f),
	Color4F(0.6f,0.0f,0.6f,1.0f)
};

void Cursor::onEnter()
{
    Node::onEnter();
    scheduleUpdate();
    
    
    drawNode = DrawNode::create();
    addChild(drawNode);
    
    drawShape();
}

void Cursor::update(float dt)
{
    float scaleDelta = dt / interval;
    
    if(expanding)
    {
        scale += scaleDelta;
        
        if(scale >= 1)
        {
            expanding = false;
            scale = 1 - (scale -1);
        }
    }
    else
    {
        scale -= scaleDelta;
        
        if(scale <= 0)
        {
            expanding = true;
            scale = 0 - scale;
            
            ++crntColor;
            crntColor %= 6;
            
            drawShape();
        }
    }
    setScaleX(scale);
}
void Cursor::reset()
{
    scale = 1;
    crntColor = 0;
    expanding = false;
    drawShape();
}

int Cursor::scaledSize()
{
	return halfSize * App::getScale();
}

const float DiamondCursor::ratio = 1.5f;

void DiamondCursor::onEnter()
{
    Cursor::onEnter();
    drawNode->setRotation(45);
    setScaleY(ratio);
}

void DiamondCursor::drawShape()
{
	float hSize = scaledSize();
    drawNode->clear();
    drawNode->drawSolidRect(Vec2(-hSize,-hSize), Vec2(hSize, hSize), colors[crntColor]);
}

void DownTriangleCursor::drawShape()
{
	float hSize = scaledSize();
	drawNode->clear();
    
    Vec2 left(-hSize, hSize);
    Vec2 right(hSize, hSize);
    Vec2 bottom(0,-hSize);
    
    drawNode->drawTriangle(left,right,bottom,colors[crntColor]);
}

Color4F toColor4F(Color3B color)
{
	return Color4F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f);
}

Color4F operator*(const Color4F& lhs, float rhs)
{
	return Color4F(lhs.r * rhs, lhs.g * rhs, lhs.b * rhs, lhs.a);
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
    return Label::createWithTTF(text, "fonts/comfortaa.ttf", size);
}

Label* createTextLabel(const string& text, int size, const string& fontRes)
{
	return Label::createWithTTF(text, fontRes, size);
}

Color4F opacityScale(const Color4F& color, GLubyte opacity)
{
	return Color4F(color.r, color.g, color.b, color.a * opacity / 255);
}
