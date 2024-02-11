#ifdef GL_ES
precision highp float;
#endif

uniform vec4 u_color;
uniform vec2 u_center;
uniform float u_radius;
uniform float u_startAngle;
uniform float u_endAngle;

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
	a += float(a < 0.0)*radians(360.0);
	
	//Is pixel coord within circle?
	float co1 = 1.0 - step(u_radius, d);
	//Is pixel coord within angle? If cone includes angle 0, start angle will be larger than end angle,
	//and angles from start to 2pi, and from 0 to end should be included.
	float co2 = float(u_startAngle < u_endAngle)*float(a >= u_startAngle && a <= u_endAngle) + 
	            float(u_startAngle >= u_endAngle)*float(a >= u_startAngle || a <= u_endAngle);
    gl_FragColor = mix(
		vec4(0.0, 0.0, 0.0, 0.0),
		mix(
			vec4(0.0,0.0,0.0,0.0),
			u_color,
			co2
		),
		co1
	);
}
