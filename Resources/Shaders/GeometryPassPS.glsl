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

struct TextureMapFlags
{
  bool Diffuse;
  bool Normal;
  bool Specular;
  bool Depth;
};

layout(std140) uniform MaterialBuffer
{  
  TextureMapFlags Enabled;
  vec4 AmbientColour;
  vec4 DiffuseColour;
  vec4 SpecularColour;  
  float SpecularExponent;  
} Material;

layout(location = 0) in ShaderInterface fsIn;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;
uniform sampler2D DepthMap;

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
  vec4 diffuse = texture(DiffuseMap, fsIn.TexCoord) * vec4(Material.Enabled.Diffuse);
  vec4 normal = texture(NormalMap, fsIn.TexCoord) * vec4(Material.Enabled.Normal)
              + fsIn.Normal * (1.0f - vec4(Material.Enabled.Normal));

  Position = fsIn.Position;
  Normal = normal;
  Albedo = CorrectGamma(diffuse);
}
