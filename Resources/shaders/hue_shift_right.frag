#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
    //v_fragmentColor seems to be the color mask.
    //vec4 pixColor = v_fragmentColor * texture2D(CC_Texture0, v_texCoord);
    vec4 pixColor = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor = vec4(
        pixColor.b,
        pixColor.r,
        pixColor.g,
        pixColor.a
    );
}
