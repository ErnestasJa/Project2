#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec2 uv_in;
layout(location = 1) in vec3 pos_in;
layout(location = 3) in vec4 blend_index;
layout(location = 4) in vec4 blend_weight;
//gpu skin shader example: https://github.com/lsalzman/iqm/blob/master/demo/gpu-demo.cpp

out vec2 UV;
out vec3 vertexColor;

uniform mat4 Bones[80];
uniform mat4 MVP;

void main(){
    mat4 m = Bones[int(blend_index.x)] * blend_weight.x;
    m += Bones[int(blend_index.y)] * blend_weight.y;
    m += Bones[int(blend_index.z)] * blend_weight.z;
    m += Bones[int(blend_index.w)] * blend_weight.w;

    vec4 mpos = vec4(pos_in,1) * m;

    gl_Position =  MVP * mpos;
    UV = uv_in;
    vertexColor = vec3(1,0,0);
}