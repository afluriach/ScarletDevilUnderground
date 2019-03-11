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

bool ShaderNode::init()
{
	Node::init();

	setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(getShaderName()));
	auto program = getGLProgram();
	initUniforms();

	return true;
}

void ShaderNode::setContentSize(const CCSize& size)
{
	_vertices[0] = Vec2(-size.width / 2, -size.height / 2);
	_vertices[1] = Vec2(size.width / 2, -size.height / 2);
	_vertices[2] = Vec2(-size.width / 2, size.height / 2);
	_vertices[3] = Vec2(size.width / 2, size.height / 2);

	Node::setContentSize(size);
}

void ShaderNode::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	_customCommand.init(_globalZOrder, transform, flags);
	_customCommand.func = CC_CALLBACK_0(ShaderNode::onDraw, this, transform, flags);
	renderer->addCommand(&_customCommand);
}

void ShaderNode::onDraw(const Mat4& transform, uint32_t flags)
{
	auto program = getGLProgram();
	program->use();
	program->setUniformsForBuiltins(transform);

	updateUniforms();

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


RadialGradient::RadialGradient(const Color4F& startColor, const Color4F& endColor, float radius, const Vec2& center, float expand) :
	_startColor(startColor),
	_endColor(endColor),
	_radius(radius),
	_expand(expand)
{
	_center = center;
}

void RadialGradient::initUniforms()
{
	auto program = getGLProgram();
	_uniformLocationStartColor = program->getUniformLocation("u_startColor");
	_uniformLocationEndColor = program->getUniformLocation("u_endColor");
	_uniformLocationExpand = program->getUniformLocation("u_expand");
	_uniformLocationRadius = program->getUniformLocation("u_radius");
	_uniformLocationCenter = program->getUniformLocation("u_center");
}

void RadialGradient::updateUniforms()
{
	auto program = getGLProgram();
	program->setUniformLocationWith4f(_uniformLocationStartColor, _startColor.r,
		_startColor.g, _startColor.b, _startColor.a);
	program->setUniformLocationWith4f(_uniformLocationEndColor, _endColor.r,
		_endColor.g, _endColor.b, _endColor.a);
	program->setUniformLocationWith2f(_uniformLocationCenter, _center.x, _center.y);
	program->setUniformLocationWith1f(_uniformLocationRadius, _radius);
	program->setUniformLocationWith1f(_uniformLocationExpand, _expand);

}

RadialMeterShader::RadialMeterShader(const Color4F& fillColor, const Color4F& emptyColor, float radius, const Vec2& center, float angle) :
	_fillColor(fillColor),
	_emptyColor(emptyColor),
	_radius(radius),
	_angle(angle)
{
	_center = center;
}

void RadialMeterShader::initUniforms()
{
	auto program = getGLProgram();
	_uniformLocationFillColor = program->getUniformLocation("u_fillColor");
	_uniformLocationEmptyColor = program->getUniformLocation("u_emptyColor");
	_uniformLocationAngle = program->getUniformLocation("u_angle");
	_uniformLocationRadius = program->getUniformLocation("u_radius");
	_uniformLocationCenter = program->getUniformLocation("u_center");
}

void RadialMeterShader::updateUniforms()
{
	auto program = getGLProgram();
	program->setUniformLocationWith4f(_uniformLocationFillColor, _fillColor.r,
		_fillColor.g, _fillColor.b, _fillColor.a);
	program->setUniformLocationWith4f(_uniformLocationEmptyColor, _emptyColor.r,
		_emptyColor.g, _emptyColor.b, _emptyColor.a);
	program->setUniformLocationWith2f(_uniformLocationCenter, _center.x, _center.y);
	program->setUniformLocationWith1f(_uniformLocationRadius, _radius);
	program->setUniformLocationWith1f(_uniformLocationAngle, _angle);
}

void RadialMeterShader::setAngle(float angle)
{
	_angle = angle;
}

void RadialMeterShader::setColors(Color4F fill, Color4F empty)
{
	_fillColor = fill;
	_emptyColor = empty;
}

ConeShader::ConeShader(const Color4F& color, float radius, const Vec2& center, float startAngle, float endAngle) :
	_color(color),
	_radius(radius),
	_startAngle(startAngle),
	_endAngle(endAngle)
{
	_center = center;
}

void ConeShader::initUniforms()
{
	auto program = getGLProgram();
	_uniformLocationColor = program->getUniformLocation("u_color");
	_uniformLocationStartAngle = program->getUniformLocation("u_startAngle");
	_uniformLocationEndAngle = program->getUniformLocation("u_endAngle");
	_uniformLocationRadius = program->getUniformLocation("u_radius");
	_uniformLocationCenter = program->getUniformLocation("u_center");
}

void ConeShader::updateUniforms()
{
	auto program = getGLProgram();
	program->setUniformLocationWith4f(_uniformLocationColor, _color.r,
		_color.g, _color.b, _color.a);
	program->setUniformLocationWith2f(_uniformLocationCenter, _center.x, _center.y);
	program->setUniformLocationWith1f(_uniformLocationRadius, _radius);
	program->setUniformLocationWith1f(_uniformLocationStartAngle, _startAngle);
	program->setUniformLocationWith1f(_uniformLocationEndAngle, _endAngle);
}

void ConeShader::setAngles(float startAngle, float endAngle)
{
	_startAngle = startAngle;
	_endAngle = endAngle;
}

void ConeShader::setColor(Color4F color)
{
	_color = color;
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

Color3B toColor3B(const Color4F& color)
{
	return Color3B(color.r * 255, color.g * 255, color.b * 255);
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
	return []() -> FiniteTimeAction* {

		Sequence* flicker = Sequence::createWithTwoActions(
			TintTo::createRecursive(0.125f, Color3B(192, 160, 96)),
			TintTo::createRecursive(0.125f, Color3B::WHITE)
		);

		RepeatForever* loop = RepeatForever::create(flicker);
		loop->setTag(to_int(cocos_action_tag::combo_mode_flicker));

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

ActionGeneratorType bombAnimationAction(float expand_ratio)
{
	return [expand_ratio]() -> FiniteTimeAction* {

		FadeTo* fade = FadeTo::create(0.0f, 64);
		ScaleTo* expand = ScaleTo::create(0.125f, expand_ratio);

		Sequence* flicker = Sequence::createWithTwoActions(FadeTo::create(0.125f, 0), FadeTo::create(0.125f, 64));
		Repeat* loop = Repeat::create(flicker, 4);

		return Sequence::create(
			fade,
			expand,
			loop,
			nullptr
		);
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
	Vec2 end_pos = start_pos + Vec2(0.0f, App::pixelsPerTile);
	return [end_pos]() -> FiniteTimeAction* {
		MoveTo* moveTo = MoveTo::create(1.0f, end_pos);
		FadeOut* fadeOut = FadeOut::create(0.5f);
		RemoveSelf* removeSelf = RemoveSelf::create();

		return Sequence::create(moveTo, fadeOut, removeSelf, nullptr);
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
