#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

in vec3 vertexColor;
out vec3 color;

layout(binding=0) uniform sampler2D textureSampler0;

void main(){
    color = vertexColor;
}