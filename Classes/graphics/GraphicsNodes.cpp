//
//  GraphicsNodes.cpp
//  Koumachika
//
//  Created by Toni on 6/8/19.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "graphics_types.h"
#include "GraphicsNodes.hpp"
#include "util.h"

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
	const Vec2& center
) :
	bodyColor(bodyColor),
	coneColor(coneColor),
	bodyRadius(bodyRadius),
	coneRadius(coneRadius),
	thickness(thickness),
	center(center)
{}

void AgentBodyShader::initUniforms()
{
	auto program = getGLProgram();
	_uniformLocationBodyColor = program->getUniformLocation("u_bodyColor");
	_uniformLocationConeColor = program->getUniformLocation("u_coneColor");
	_uniformLocationBodyRadius = program->getUniformLocation("u_bodyRadius");
	_uniformLocationConeRadius = program->getUniformLocation("u_coneRadius");
	_uniformLocationCenter = program->getUniformLocation("u_center");
	_uniformLocationThickness = program->getUniformLocation("u_thickness");
	_uniformLocationShieldLevel = program->getUniformLocation("u_shieldLevel");
}

void AgentBodyShader::updateUniforms()
{
	setUniform4f(_uniformLocationBodyColor, bodyColor);
	setUniform4f(_uniformLocationConeColor, coneColor);
	setUniform2f(_uniformLocationCenter, center);
	setUniform1f(_uniformLocationBodyRadius, bodyRadius);
	setUniform1f(_uniformLocationConeRadius, coneRadius);
	setUniform1f(_uniformLocationThickness, thickness);
	setUniform1f(_uniformLocationShieldLevel, shieldLevel);
}

void AgentBodyShader::setShieldLevel(float level)
{
	shieldLevel = level;
}

AmbientLightNode::AmbientLightNode(const AmbientLightArea& light) :
	light(light)
{}

bool AmbientLightNode::init()
{
	DrawNode::init();

	Vec2 halfDim = toCocos(light.dimensions) * 0.5f * app::pixelsPerTile;

	drawSolidRect(-halfDim, halfDim, light.color);
	setContentSize(toCCSize(light.dimensions) * app::pixelsPerTile);

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
	return halfSize * app::params.getScale();
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

const LinearMeterSettings LinearMeter::hpSettings = LinearMeterSettings{
	Color4F(.86f,.16f,.19f,1.0f),
	Color4F(.42f,.29f,.29f,1.0f),
};

const LinearMeterSettings LinearMeter::mpSettings = LinearMeterSettings{
	Color4F(.37f,.56f,.57f,1.0f),
	Color4F(.4f,.4f,.4f,1.0f),
};

const LinearMeterSettings LinearMeter::staminaSettings = LinearMeterSettings{
	Color4F(.47f,.75f,.18f,1.0f),
	Color4F(.44f,.51f,.36f,1.0f),
};

const Vec2 LinearMeter::boundingSize = Vec2(288, 18);
const float LinearMeter::outlineWidth = 3;

LinearMeter::LinearMeter(LinearMeterSettings settings) :
	settings(settings)
{
}

bool LinearMeter::init()
{
	Node::init();

	draw = DrawNode::create();
	addChild(draw);

	return true;
}

void LinearMeter::setValue(float newValue)
{
	crntValue = newValue;
	redraw();
}

void LinearMeter::setMax(float maxValue)
{
	this->maxValue = maxValue;
	redraw();
}

void LinearMeter::redraw()
{
	if (maxValue <= 0.0f) return;

	float ratio = crntValue / maxValue;

	draw->clear();

	draw->drawSolidRect(
		Vec2(-0.5f * boundingSize.x - outlineWidth, -boundingSize.y*0.5f - outlineWidth),
		Vec2(boundingSize.x*0.5f + outlineWidth, boundingSize.y*0.5f + outlineWidth),
		Color4F::BLACK
	);

	draw->drawSolidRect(
		Vec2(-0.5f * boundingSize.x, -boundingSize.y*0.5f),
		Vec2(boundingSize.x*(ratio - 0.5f), boundingSize.y*0.5f),
		settings.fillColor
	);
	draw->drawSolidRect(
		Vec2(boundingSize.x*(ratio - 0.5f), -0.5f * boundingSize.y),
		Vec2(boundingSize.x * 0.5f, 0.5f*boundingSize.y),
		settings.emptyColor
	);
}
