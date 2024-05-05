//
//  GraphicsNodes.hpp
//  Koumachika
//
//  Created by Toni on 6/8/19.
//
//

#ifndef GraphicsNodes_hpp
#define GraphicsNodes_hpp

class ShaderNode : public Node
{
public:
	inline ShaderNode() {}
	inline virtual ~ShaderNode() {}

	virtual bool init();
	virtual void setContentSize(const CCSize& size) override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;

	inline void setBlendFunc(BlendFunc b) { _blendFunc = b; }

	virtual string getShaderName() const = 0;
	virtual void initUniforms() = 0;
	virtual void updateUniforms() = 0;

	void setUniform1f(GLint id, float val);
	void setUniform2f(GLint id, Vec2 val);
	void setUniform4f(GLint id, const Color4F& val);
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

	inline void setColor4F(const Color4F& color) { _startColor = color; }
	inline Color4F getColor4F() const { return _startColor; }

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

class RadialMeterShader : public ShaderNode
{
public:
	RadialMeterShader(const Color4F& fillColor, const Color4F& emptyColor, float radius, const Vec2& center, float angle);

	void setAngle(float angle);
	void setColors(Color4F fill, Color4F empty);

	inline virtual string getShaderName() const { return "radial_meter"; }
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

class ConeShader : public ShaderNode
{
public:
	ConeShader(const Color4F& color, float radius, const Vec2& center, SpaceFloat coneWidth, SpaceFloat initialAngleRad);

	virtual void setRotation(float rotation);
	void setLightColor(Color4F color);
	inline Color4F getLightColor() { return _color; }

	inline virtual string getShaderName() const { return "cone"; }
	virtual void initUniforms();
	virtual void updateUniforms();

protected:
	Color4F _color;
	float _radius;
	float coneWidth;

	SpaceFloat _startAngle, _endAngle;

	GLint _uniformLocationColor;
	GLint _uniformLocationCenter;
	GLint _uniformLocationRadius;
	GLint _uniformLocationStartAngle;
	GLint _uniformLocationEndAngle;
};

class AgentBodyShader : public ShaderNode
{
public:
	AgentBodyShader(
		const Color4F& bodyColor, 
		const Color4F& coneColor,
		float bodyRadius,
		float coneRadius,
		float thickness,
		const Vec2& center
	);

	inline virtual string getShaderName() const { return "agent_overlay"; }
	virtual void initUniforms();
	virtual void updateUniforms();

	void setShieldLevel(float level);
protected:
	Vec2 center;
	Color4F bodyColor, coneColor;
	float bodyRadius, coneRadius;
	float thickness;
	float shieldLevel = 1.0f;

	GLint _uniformLocationBodyColor;
	GLint _uniformLocationConeColor;
	GLint _uniformLocationCenter;
	GLint _uniformLocationBodyRadius;
	GLint _uniformLocationConeRadius;
	GLint _uniformLocationThickness;
	GLint _uniformLocationShieldLevel;
};

class AmbientLightNode : public DrawNode
{
public:
	AmbientLightNode(const AmbientLightArea& light);

	virtual bool init();

	inline Color4F getLightColor() const { return light.color; }
protected:
	AmbientLightArea light;
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

struct LinearMeterSettings
{
	Color4F fillColor;
	Color4F emptyColor;
};

class LinearMeter : public Node
{
public:
	static const Vec2 boundingSize;
	static const float outlineWidth;

	static const LinearMeterSettings hpSettings;
	static const LinearMeterSettings mpSettings;
	static const LinearMeterSettings staminaSettings;

	LinearMeter(LinearMeterSettings settings);

	virtual bool init();

	void setValue(float newValue);
	void setMax(float maxValue);

	inline float getValue() const { return crntValue; }
	inline float getMaxValue() const { return maxValue; }
protected:
	void redraw();

	LinearMeterSettings settings;
	DrawNode* draw;

	float crntValue = 0.0f;
	float maxValue = 0.0f;
};

#endif /* Graphics_h */
