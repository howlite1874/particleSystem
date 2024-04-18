#version 330 core
uniform vec4 u_color;
uniform sampler2D s_texture;

out vec4 fragColor;

void main()
{
    vec4 texColor;
    texColor = texture(s_texture,gl_PointCoord);
    fragColor = vec4(texColor.xyz,texColor.y);
}