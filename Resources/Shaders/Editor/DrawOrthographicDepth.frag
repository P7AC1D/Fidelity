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

float LinearizeDepth(in vec2 uv, in uint layer)
{
    float zNear = ShadowMapDebugConstants.nearClip;
    float zFar  = ShadowMapDebugConstants.farClip;
    float depth = texture(ShadowMaps, vec3(uv, layer)).r;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{             
	float depthValue = texture(ShadowMaps, vec3(TexCoord, ShadowMapDebugConstants.layer)).r;
  //float depthValue = LinearizeDepth(TexCoord, ShadowMapDebugConstants.layer);
	FinalColour = vec4(vec3(depthValue), 1.0);
}
