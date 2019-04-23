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

vec4 cone_colors[4] = vec4[4](
	u_coneColor * 2.0,
	u_coneColor * 1.5 * float(u_shieldLevel > 1),
	u_coneColor * 1.0 * float(u_shieldLevel > 2),
	u_coneColor * 0.5 * float(u_shieldLevel > 3)
);

void main()
{
    float d = distance(v_position.xy, u_center);
	float a = atan(v_position.y - u_center.y, v_position.x - u_center.x);
	//convert angle to [0,2pi)
	a += float(a < 0.0)*radians(360);
	
	//Is pixel coord within body circle outline?
	float body = float(d >= u_bodyRadius - u_thickness) * float(d <= u_bodyRadius);

	vec4 cone_color =
		float(a >= radians(45) && a < radians(135)) * cone_colors[0] +
		float(a >= radians(0) && a < radians(45) || a >= radians(135) && a < radians(180)) * cone_colors[1] +
		float(a >= radians(315) && a < radians(360) || a >= radians(180) && a < radians(225)) * cone_colors[2] +
		float(a >= radians(225) && a < radians(315)) * cone_colors[3]
	;
	
	//Is pixel coord within cone radius, outside of body circle outline?
	float cone = float(d >= u_bodyRadius && d < u_coneRadius);

	gl_FragColor = body*u_bodyColor + cone*cone_color;	
}
