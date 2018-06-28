#version 410

struct ShaderInterface
{
  vec4 Position;
  vec2 TexCoord;
  vec4 Normal;
  mat3 TbnMtx;
  vec3 PositionTS;
  vec3 ViewDirTS;
};

layout(location = 0) in ShaderInterface fsIn;

uniform sampler2D DiffuseMap;

layout(location = 0) out vec4 Position;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec4 Albedo;

vec4 CorrectGamma(vec4 inputSample)
{
  const float GAMMA = 2.2f;
  return pow(inputSample, vec4(GAMMA));
}

void main()
{
  Position = fsIn.Position;
  Normal = fsIn.Normal;
  Albedo = CorrectGamma(texture(DiffuseMap, fsIn.TexCoord));
}
