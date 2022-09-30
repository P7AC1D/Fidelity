#version 410

layout(std140) uniform FullscreenQuadBuffer
{
  bool SingleChannel;
} FullscreenQuadData;

uniform sampler2D QuadTexture;

layout(location = 0) in vec2 TexCoord;

out vec4 FinalColour;

void main()
{
  vec4 textureSample = texture(QuadTexture, TexCoord);
  if (FullscreenQuadData.SingleChannel)
  {
    FinalColour = vec4(textureSample.r, textureSample.r, textureSample.r, 1.0f);
  }
  else
  {
    FinalColour = vec4(textureSample.rgb, 1.0f);
  }
}
