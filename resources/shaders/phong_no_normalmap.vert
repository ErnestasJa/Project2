#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec2 vertexUV;
layout(location = 1) in vec3 vertexPosition_modelspace;
out vec2 UV;

uniform mat4 MVP;

void main(){
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
    UV = vertexUV;
}