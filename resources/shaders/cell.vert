#version 330
layout(location = 2) in vec3 normal;
layout(location = 1) in vec3 pos;

uniform mat4 MVP;
uniform mat3 MV3x3;

smooth out vec3 vpos;
smooth out vec3 vnormal;

void main()
{
    vnormal = MV3x3 * normal;
    gl_Position = MVP * vec4(pos,1);
    vpos = gl_Position.xyz;
}