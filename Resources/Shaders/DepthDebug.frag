#version 410

uniform sampler2D QuadTexture;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

layout(std140) uniform CameraBuffer
{
  float nearClip;
	float farClip;
} Camera;

float LinearizeDepth(in vec2 uv)
{
    float zNear = Camera.nearClip;
    float zFar  = Camera.farClip;
    float depth = texture(QuadTexture, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{             
	float depth = LinearizeDepth(TexCoord);
	FinalColour = vec4(depth, depth, depth, 1.0f);
}
