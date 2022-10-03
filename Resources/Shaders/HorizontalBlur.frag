#version 410

const float PixelKernel[9] = float[9]
(
    -4.0,
    -3.0,
    -2.0,
    -1.0,
     0.0,
     1.0,
     2.0,
     3.0,
     4.0
);
 
const float BlurWeights[9] = float[9]
(
    0.02242,
    0.04036,
    0.12332,
    0.20179,
    0.22422,
    0.20179,
    0.12332,
    0.04036,
    0.02242
);

layout(std140) uniform BlurBuffer
{
  float ScreenWidth;
  float ScreenHeight;
  float BlurAmount;
} BlurData;

layout(location = 0) in vec2 TexCoord;

uniform sampler2D TextureMap;

out vec4 FinalColour;

void main()
{
  vec4 colour = vec4(0.0, 0.0, 0.0, 0.0);
  float texelSize = 1.0 / BlurData.ScreenWidth;

  for(int i = 0; i < 9; i++)
  {
    colour += texture(TextureMap, TexCoord + vec2(texelSize * PixelKernel[i] * BlurData.BlurAmount, 0.0f)) * BlurWeights[i];
  }
	
  colour.a = 1.0f;
  FinalColour = colour;
}