#version 410

layout(std140) uniform PerObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
  bool DiffuseEnabled;
  bool NormalEnabled;
  bool SpecularEnabled;
  bool OpacityEnabled;
  vec4 AmbientColour;
  vec4 DiffuseColour;
  vec4 SpecularColour;
  float SpecularExponent;
} Object;

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

vec3 CalculateDiffuse(vec4 diffuseSample, vec4 materialColour, bool isDiffuseMapEnabled)
{
  if (isDiffuseMapEnabled)
  {
    return diffuseSample.rgb * materialColour.rgb;
  }
  return materialColour.rgb;
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

vec3 CalculateSpecular(vec4 specularSample, vec4 materialColour, bool isSpecularEnabled)
{
  if (isSpecularEnabled)
  {
    return specularSample.rgb;
  }
  return vec3(1.0f);
}

void main()
{
  vec4 opacitySample = texture(OpacityMap, fsIn.TexCoord);
  if (opacitySample.r < 1.0f)
  {
    discard;
  }

  vec4 diffuseSample = texture(DiffuseMap, fsIn.TexCoord);  
  vec4 normalSample = texture(NormalMap, fsIn.TexCoord);
  vec4 specularSample = texture(SpecularMap, fsIn.TexCoord);

  Diffuse.rgb = CalculateDiffuse(diffuseSample, Object.DiffuseColour, Object.DiffuseEnabled);
  Diffuse.a = 1.0f;
  Specular.rgb = CalculateSpecular(specularSample, Object.SpecularColour, Object.SpecularEnabled);
  Specular.a = 1.0f;

  // Transforms normals from [-1,1] to [0,1].
  Normal = vec4(CalculateNormal(normalSample, vec4(fsIn.Normal, 0.0f), Object.NormalEnabled).xyz * 0.5f + 0.5f, 1.0f);
}