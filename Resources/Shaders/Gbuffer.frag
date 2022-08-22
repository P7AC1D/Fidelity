#version 410

struct TextureMapFlags
{
  bool Diffuse;
  bool Normal;
  bool Specular;
  bool Opacity;
};

layout(std140) uniform MaterialBuffer
{
  TextureMapFlags Enabled;
  vec4 AmbientColour;
  vec4 DiffuseColour;
  vec4 SpecularColour;
  float SpecularExponent;
} Material;

struct Input
{
  vec3 WorldPos;
  vec2 TexCoord;
  vec3 Normal;
  vec3 Tangent;
  vec3 Binormal;
};

layout(location = 0) in Input fsIn;

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;
uniform sampler2D OpacityMap;

layout(location = 0) out vec4 Diffuse;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec4 Specular;

vec4 CalculateDiffuse(vec4 diffuseSample, vec4 materialColour, bool isDiffuseMapEnabled)
{
  if (isDiffuseMapEnabled)
  {
    return diffuseSample * materialColour;
  }
  return materialColour;
}

vec4 CalculateNormal(vec4 normalSample, vec4 normal, bool isNormalMapEnabled)
{
  if (isNormalMapEnabled)
  {
    mat3 tbn = mat3(fsIn.Tangent, fsIn.Binormal, fsIn.Normal);
    return vec4(normalize(tbn * (normalSample.rgb) * 2.0f - 1.0f), 0.0f);
  }
  return normalize(normal);
}

vec4 CalculateSpecular(vec4 specularSample, bool isSpecularEnabled)
{
  if (isSpecularEnabled)
  {
    return vec4(specularSample.r) * Material.SpecularColour;
  }
  return Material.SpecularColour;
}

float CalculateOpacity(float opacitySample, bool isOpacityMapEnabled)
{
  if (isOpacityMapEnabled)
  {
    return opacitySample;
  }
  return 1.0f;
}

void main()
{
  vec4 diffuseSample = texture(DiffuseMap, fsIn.TexCoord);  
  vec4 normalSample = texture(NormalMap, fsIn.TexCoord);
  vec4 opacitySample = texture(OpacityMap, fsIn.TexCoord);
  vec4 specularSample = texture(SpecularMap, fsIn.TexCoord);

  float opacityFactor = CalculateOpacity(opacitySample.r, Material.Enabled.Opacity);
  Diffuse.rgb = Material.DiffuseColour.rgb * diffuseSample.rgb;
  Diffuse.a = opacityFactor;
  Specular = CalculateSpecular(specularSample, Material.Enabled.Specular);
  Specular.a = Material.SpecularExponent / 250.0f;
  Normal = vec4(CalculateNormal(normalSample, vec4(fsIn.Normal, 0.0f), Material.Enabled.Normal).xyz * 0.5f + 0.5f, 1.0f);
}