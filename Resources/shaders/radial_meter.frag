#ifdef GL_ES
precision highp float;
#endif

uniform vec4 u_fillColor;
uniform vec4 u_emptyColor;
uniform vec2 u_center;
uniform float u_radius;
uniform float u_angle;

#ifdef GL_ES
varying lowp vec4 v_position;
#else
varying vec4 v_position;
#endif

void main()
{
    float d = distance(v_position.xy, u_center);
	float a = radians(180.0) - atan(v_position.y - u_center.y, v_position.x - u_center.x);
	//is pixel coord within circle
	float co1 = 1.0 - step(u_radius, d);
	//is pixel within angle
	float co2 = 1.0 - step(u_angle, a);
	
    gl_FragColor = mix(
		vec4(0.0, 0.0, 0.0, 0.0),
		mix(
			u_emptyColor,
			u_fillColor,
			co2
		),
		co1
	);
}
