#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

in vec3 vertexColor;
out vec3 color;

void main(){
    color = vertexColor;
}