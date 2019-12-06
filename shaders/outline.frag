uniform sampler2D texture;
uniform vec2 resolution;

void main()
{
    vec4 color = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;

    vec2 center = (0.5, 0.5);
    float distance = sqrt((gl_TexCoord[0].x - center.x) * (gl_TexCoord[0].x - center.x) + (gl_TexCoord[0].y - center.y) * (gl_TexCoord[0].y - center.y));

    gl_FragColor = color * (1 - distance);
}
