#version 410

in VsOut
{
  mat4 TbnMtx;
  vec4 Position;
  vec4 Normal;
  vec2 TexCoord;
  vec3 PositionTS;
  vec3 ViewTS;
} fsIn;

uniform sampler2D DiffuseMap;

out vec4 FinalColour;

vec4 CorrectGamma(vec4 inputSample)
{
  const float GAMMA = 2.2f;
  return pow(inputSample, vec4(GAMMA));
}

void main()
{
  FinalColour = CorrectGamma(vec4(texture(DiffuseMap, fsIn.TexCoord)));
}
