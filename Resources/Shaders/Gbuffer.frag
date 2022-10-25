#version 410

layout(std140) uniform PerObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
  vec4 DiffuseColour;
  bool DiffuseEnabled;
  bool NormalEnabled;
  bool MetalnessEnabled;
  bool RoughnessEnabled;
  bool OcclusionEnabled;
  bool OpacityEnabled;
  float Metalness;
  float Roughness;
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
uniform sampler2D MetallicMap;
uniform sampler2D RoughnessMap;
uniform sampler2D OcclusionMap;
uniform sampler2D OpacityMap;

layout(location = 0) out vec4 Diffuse;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec4 Material;

vec3 CalculateDiffuse(vec4 diffuseSample)
{
  if (Object.DiffuseEnabled)
  {
    return diffuseSample.rgb * Object.DiffuseColour.rgb;
  }
  return Object.DiffuseColour.rgb;
}

vec4 CalculateNormal(vec4 normalSample, vec4 normal)
{
  if (Object.NormalEnabled)
  {
    vec3 T = fsIn.Tangent;
    vec3 N = fsIn.Normal;
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 tbn = mat3(T, B, N);
    return vec4(normalize(tbn * (normalSample.rgb * 2.0f - 1.0f)), 0.0f);
  }
  return normalize(normal);
}

float CalculateOcclusion(vec4 occlusionSample)
{
  if (Object.OcclusionEnabled)
  {
    return occlusionSample.r;
  }
  return 0.0;
}

void main()
{
  vec4 diffuseSample = texture(DiffuseMap, fsIn.TexCoord);  
  vec4 normalSample = texture(NormalMap, fsIn.TexCoord);
  vec4 metallnessSample = texture(MetallicMap, fsIn.TexCoord);
  vec4 roughnessSample = texture(RoughnessMap, fsIn.TexCoord);
  vec4 occlusionSample = texture(OcclusionMap, fsIn.TexCoord);

  Diffuse.rgb = CalculateDiffuse(diffuseSample);
  Diffuse.a = 1.0f;

  Material.r = Object.MetalnessEnabled ? metallnessSample.r : Object.Metalness;
  Material.g = Object.RoughnessEnabled ? roughnessSample.r : Object.Roughness;
  Material.b = CalculateOcclusion(occlusionSample);
  Material.a = 1.0f;

  // Transforms normals from [-1,1] to [0,1].
  Normal = vec4(CalculateNormal(normalSample, vec4(fsIn.Normal, 0.0f)).xyz * 0.5f + 0.5f, 1.0f);
}