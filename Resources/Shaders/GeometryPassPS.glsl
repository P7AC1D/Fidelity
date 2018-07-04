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

vec2 CalcParallaxTexCoords(vec2 texCoords, vec3 viewDir, bool isDepthMapEnabled) 
{ 
  const float heightScale = 0.1f; 
  const float minLayers = 8; 
  const float maxLayers = 64; 
  float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir))); 
  float layerDepth = 1.0 / numLayers; 
  float currentLayerDepth = 0.0; 
  vec2 P = viewDir.xy / viewDir.z * heightScale; 
  vec2 deltaTexCoords = P / numLayers; 
 
  vec2  currentTexCoords = texCoords; 
  float currentDepthMapValue = texture(DepthMap, currentTexCoords).r; 
 
  while (currentLayerDepth < currentDepthMapValue) 
  { 
    currentTexCoords -= deltaTexCoords; 
    currentDepthMapValue = texture(DepthMap, currentTexCoords).r; 
    currentLayerDepth += layerDepth; 
  } 
 
  vec2 prevTexCoords = currentTexCoords + deltaTexCoords; 
 
  float afterDepth = currentDepthMapValue - currentLayerDepth; 
  float beforeDepth = texture(DepthMap, prevTexCoords).r - currentLayerDepth + layerDepth; 
 
  float weight = afterDepth / (afterDepth - beforeDepth); 
  vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight); 
  return finalTexCoords * vec2(isDepthMapEnabled) + texCoords * (1.0f - vec2(isDepthMapEnabled)); 
} 

vec3 CorrectGamma(vec3 inputSample)
{
  const float GAMMA = 2.2f;
  return pow(inputSample, vec3(GAMMA));
}

vec4 CalculateSpecular(vec4 specularSample, bool isSpecularMapEnabled)
{
  return specularSample * vec4(isSpecularMapEnabled);
}

vec4 CalculateDiffuse(vec4 diffuseSample, bool isDiffuseMapEnabled)
{
  return vec4(CorrectGamma(diffuseSample.rgb), 1.0f) * vec4(Material.Enabled.Diffuse);
}

vec4 CalculateNormal(vec4 normalSample, vec4 normal, bool isNormalMapEnabled)
{
  vec4 correctedNormalSample = normalize(normalSample * 2.0f - 1.0f);
  vec4 correctedNormalSampleTbn = vec4(fsIn.TbnMtx * correctedNormalSample.xyz, 0.0f);
  
  return correctedNormalSampleTbn * vec4(isNormalMapEnabled) + normal * (1.0f - vec4(isNormalMapEnabled));
}

void main()
{
  vec3 viewDir = normalize(fsIn.ViewDirTS - fsIn.PositionTS); 
  vec2 parallaxTexCoord = CalcParallaxTexCoords(fsIn.TexCoord, viewDir, Material.Enabled.Depth); 

  vec4 diffuseSample = texture(DiffuseMap, fsIn.TexCoord);  
  vec4 normalSample = texture(NormalMap, fsIn.TexCoord);
  vec4 specularSample = texture(SpecularMap, fsIn.TexCoord);

  Position = fsIn.Position;
  Normal = CalculateNormal(normalSample, fsIn.Normal, Material.Enabled.Normal);
  Albedo.rgb = CalculateDiffuse(diffuseSample, Material.Enabled.Diffuse).rgb;
  Albedo.a = CalculateSpecular(specularSample, Material.Enabled.Specular).r;
}
