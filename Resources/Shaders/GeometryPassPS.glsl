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

vec3 CorrectGamma(vec3 inputSample)
{
  const float GAMMA = 2.2f;
  return pow(inputSample, vec3(GAMMA));
}

vec4 CalculateDiffuse(vec4 diffuseSample, bool isDiffuseMapEnabled)
{
  return vec4(CorrectGamma(diffuseSample.rgb), 1.0f) * vec3(Material.Enabled.Diffuse);
}

vec4 CalculateNormal(vec4 normalSample, vec4 normal, bool isNormalMapEnabled)
{
  vec4 correctedNormalSample = normalize(normalSample * 2.0f - 1.0f);
  vec4 correctedNormalSampleTbn = vec4(fsIn.TbnMtx * correctedNormalSample.xyz, 0.0f);
  
  return correctedNormalSampleTbn * vec4(isNormalMapEnabled) + normal * (1.0f - vec4(isNormalMapEnabled));
}

void main()
{
  vec4 diffuseSample = texture(DiffuseMap, fsIn.TexCoord);  
  vec4 normalSample = texture(NormalMap, fsIn.TexCoord);

  Position = fsIn.Position;
  Normal = CalculateNormal(normalSample, fsIn.Normal, Material.Enabled.Normal);
  Albedo = CalculateDiffuse(diffuseSample, Material.Enabled.Diffuse);
}
