#version 410 core

in VsOut
{
  mat3 TbnMtx;
  vec4 Position;
  vec4 Normal;
  vec2 TexCoord;
  vec3 PositionTS;
  vec3 ViewDirTS;
} fsIn;

uniform sampler2D DiffuseMap;

//layout(location = 0) out vec4 Position;
//layout(location = 1) out vec4 Normal;
//layout(location = 2) out vec4 Albedo;

out vec4 PixelColour;

vec4 CorrectGamma(vec4 inputSample)
{
  const float GAMMA = 2.2f;
  return pow(inputSample, vec4(GAMMA));
}

void main()
{
  PixelColour = CorrectGamma(vec4(texture(DiffuseMap, fsIn.TexCoord)));
}
