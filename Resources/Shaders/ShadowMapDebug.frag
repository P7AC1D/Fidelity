#version 410

uniform sampler2DArray ShadowMaps;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

layout(std140) uniform ShadowMapDebugBuffer
{
  float nearClip;
	float farClip;
  uint layer;
} ShadowMapDebugConstants;

void main()
{             
	float depthValue = texture(ShadowMaps, vec3(TexCoord, ShadowMapDebugConstants.layer)).r;
	FinalColour = vec4(vec3(depthValue), 1.0);
}
