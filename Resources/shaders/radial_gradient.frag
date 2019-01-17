#ifdef GL_ES
precision highp float;
#endif

uniform vec4 u_startColor;
uniform vec4 u_endColor;
uniform vec2 u_center;
uniform float u_radius;
uniform float u_expand;

#ifdef GL_ES
varying lowp vec4 v_position;
#else
varying vec4 v_position;
#endif

void main()
{
    float d = distance(v_position.xy, u_center) / u_radius;
    if (d <= 1.0)
    {
        if (d <= u_expand)
        {
            gl_FragColor = u_startColor;
        }
        else
        {
            gl_FragColor = mix(u_startColor, u_endColor, (d - u_expand) / (1.0 - u_expand));
        }
    }
    else
    {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
