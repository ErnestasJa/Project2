#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 pos_in;
layout(location = 1) in vec3 color_in;
out vec2 UV;
out vec3 vertexColor;

uniform mat4 MVP;

void main(){
    gl_Position =  MVP * vec4(pos_in,1);
    vertexColor = color_in;
}