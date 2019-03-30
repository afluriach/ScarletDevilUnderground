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
#include "util.h"

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

void ShaderNode::setUniform1f(GLint id, float val)
{
	auto p = getGLProgram();
	p->setUniformLocationWith1f(id, val);
}

void ShaderNode::setUniform2f(GLint id, Vec2 val)
{
	auto p = getGLProgram();
	p->setUniformLocationWith2f(id, val.x, val.y);
}

void ShaderNode::setUniform4f(GLint id, const Color4F& val)
{
	auto p = getGLProgram();
	p->setUniformLocationWith4f(id, val.r, val.g, val.b, val.a);
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

ConeShader::ConeShader(const Color4F& color, float radius, const Vec2& center, SpaceFloat coneWidth, SpaceFloat initialAngleRad) :
	_color(color),
	_radius(radius),
	coneWidth(coneWidth)
{
	_center = center;

	_startAngle = canonicalAngle(initialAngleRad - 0.5*coneWidth);
	_endAngle = canonicalAngle(initialAngleRad + 0.5*coneWidth);
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

void ConeShader::setRotation(float rotation)
{
	float a = fromCocosAngle(rotation);
	_startAngle = canonicalAngle(a - 0.5f*coneWidth);
	_endAngle = canonicalAngle(a + 0.5f*coneWidth);
}

void ConeShader::setLightColor(Color4F color)
{
	_color = color;
}

AgentBodyShader::AgentBodyShader(
	const Color4F& bodyColor,
	const Color4F& coneColor,
	float bodyRadius,
	float coneRadius,
	float thickness,
	const Vec2& center,
	float startAngle, float endAngle
) :
	bodyColor(bodyColor),
	coneColor(coneColor),
	bodyRadius(bodyRadius),
	coneRadius(coneRadius),
	thickness(thickness),
	center(center),
	startAngle(startAngle),
	endAngle(endAngle)
{}

void AgentBodyShader::initUniforms()
{
	auto program = getGLProgram();
	_uniformLocationBodyColor = program->getUniformLocation("u_bodyColor");
	_uniformLocationConeColor = program->getUniformLocation("u_coneColor");
	_uniformLocationBodyRadius = program->getUniformLocation("u_bodyRadius");
	_uniformLocationConeRadius = program->getUniformLocation("u_coneRadius");
	_uniformLocationStartAngle = program->getUniformLocation("u_startAngle");
	_uniformLocationEndAngle = program->getUniformLocation("u_endAngle");
	_uniformLocationCenter = program->getUniformLocation("u_center");
	_uniformLocationThickness = program->getUniformLocation("u_thickness");
}

void AgentBodyShader::updateUniforms()
{
	setUniform4f(_uniformLocationBodyColor, bodyColor);
	setUniform4f(_uniformLocationConeColor, coneColor);
	setUniform2f(_uniformLocationCenter, center);
	setUniform1f(_uniformLocationBodyRadius, bodyRadius);
	setUniform1f(_uniformLocationConeRadius, coneRadius);
	setUniform1f(_uniformLocationThickness, thickness);
	setUniform1f(_uniformLocationStartAngle, startAngle);
	setUniform1f(_uniformLocationEndAngle, endAngle);
}

void AgentBodyShader::setAngles(float startAngle, float endAngle)
{
	this->startAngle = startAngle;
	this->endAngle = endAngle;
}

AmbientLightNode::AmbientLightNode(const AmbientLightArea& light) :
	light(light)
{}

bool AmbientLightNode::init()
{
	DrawNode::init();

	Vec2 halfDim = toCocos(light.dimensions) * 0.5f * App::pixelsPerTile;

	drawSolidRect(-halfDim, halfDim, light.color);
	setContentSize(toCCSize(light.dimensions) * App::pixelsPerTile);

	return true;
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

Color3B toColor3B(const string& s)
{
	vector<string> tokens = splitString(s, ",");

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
