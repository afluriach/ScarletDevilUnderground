#ifdef GL_ES
precision highp float;
#endif

uniform vec4 u_bodyColor;
uniform vec4 u_coneColor;
uniform vec2 u_center;
uniform float u_bodyRadius;
uniform float u_coneRadius;
uniform float u_thickness;
uniform float u_shieldLevel;

#ifdef GL_ES
varying lowp vec4 v_position;
#else
varying vec4 v_position;
#endif

void main()
{
    float d = distance(v_position.xy, u_center);
	
	//Is pixel coord within body circle outline?
	float body = float(d >= u_bodyRadius - u_thickness) * float(d <= u_bodyRadius);
	
	//Is pixel coord within cone radius, outside of body circle outline?
	float cone = float(d >= u_bodyRadius && d < u_coneRadius);

	gl_FragColor = body*u_bodyColor + cone*u_coneColor;	
}
