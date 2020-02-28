#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 uv_in;
layout(location = 1) in vec3 pos_in;
layout(location = 2) in vec3 normal_in;

out vec3 UV;
uniform mat4 MVP;

void main(){
    vec4 mpos = vec4(pos_in,1);

    gl_Position = MVP * mpos;
    UV = uv_in;
}