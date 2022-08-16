#version 410
struct TextureMapFlags
{
  bool Diffuse;
  bool Normal;
  bool Specular;
  bool Depth;
};

layout(std140) uniform ObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
} Object;

layout(std140) uniform MaterialBuffer
{
  TextureMapFlags Enabled;
  vec4 AmbientColour;
  vec4 DiffuseColour;
  vec4 SpecularColour;
  float SpecularExponent;  
} Material;

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
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;
uniform sampler2D DepthMap;

layout(location = 0) out vec4 Position;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec4 Albedo;

vec4 CalculateSpecular(vec4 specularSample, vec4 materialColour, bool isSpecularMapEnabled)
{
  if (isSpecularMapEnabled)
  {
    return specularSample * materialColour;
  }
  return materialColour;
}

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
    vec4 correctedNormalSample = normalize(normalSample * 2.0f - 1.0f);
    return normalize(vec4(fsIn.TbnMtx * correctedNormalSample.xyz, 0.0f));
  }
  return normalize(normal);
}

void main()
{
  vec4 diffuseSample = texture(DiffuseMap, fsIn.TexCoord);  
  vec4 normalSample = texture(NormalMap, fsIn.TexCoord);
  vec4 specularSample = texture(SpecularMap, fsIn.TexCoord);

  Position = fsIn.Position;
  Normal = CalculateNormal(normalSample, fsIn.Normal, Material.Enabled.Normal);
  Albedo.rgb = CalculateDiffuse(diffuseSample, Material.DiffuseColour, Material.Enabled.Diffuse).rgb;
  Albedo.a = CalculateSpecular(specularSample, Material.SpecularColour, Material.Enabled.Specular).r;
}
