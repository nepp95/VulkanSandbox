#version 450

layout(location = 0) in vec3 oColor;
layout(location = 1) in vec2 oTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    outColor = texture(texSampler, oTexCoord);
}