#version 330
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_uniform_location : enable

in vec3 UV;
out vec3 color;

layout(binding=0) uniform sampler2DArray textureSampler0;

void main(){
    vec3 texColor = texture( textureSampler0, UV ).rgb;
    color = texColor;
}