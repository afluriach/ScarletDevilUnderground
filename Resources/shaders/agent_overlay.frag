#ifdef GL_ES
precision highp float;
#endif

uniform vec4 u_bodyColor;
uniform vec4 u_coneColor;
uniform vec2 u_center;
uniform float u_bodyRadius;
uniform float u_coneRadius;
uniform float u_thickness;

#ifdef GL_ES
varying lowp vec4 v_position;
#else
varying vec4 v_position;
#endif

void main()
{
    float d = distance(v_position.xy, u_center);
	float a = atan(v_position.y - u_center.y, v_position.x - u_center.x);
	//convert angle to [0,2pi)
	a += float(a < 0.0)*radians(360);
	
	//Is pixel coord within body circle outline?
	float body = float(d >= u_bodyRadius - u_thickness) * float(d <= u_bodyRadius);
	//is pixel coord within cone angle?
	float cone_angle = float(a >= radians(45) && a < radians(135));
	//Is pixel coord within cone radius, outside of body circle outline?
	float cone = float(d >= u_bodyRadius && d < u_coneRadius)*cone_angle;

	gl_FragColor = body*u_bodyColor + cone*u_coneColor;	
}
