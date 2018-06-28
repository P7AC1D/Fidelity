#version 410

struct DirectionalLightData
{
  vec4 Colour;
  vec3 Direction;
  float Intensity;
};

layout(std140) uniform ObjectBuffer
{
  mat4 Model;
};

layout(std140) uniform FrameBuffer
{
  mat4 Projection;
  mat4 View;  
  DirectionalLightData DirectionalLight;
  vec3 ViewPos;
};

uniform sampler2D PositionMap;
uniform sampler2D NormalMap;
uniform sampler2D AlbedoSpecMap;

layout(location = 0) in vec2 TexCoord;

out vec4 FinalColour;

float CalcDiffuseContribution(vec3 lightDir, vec3 normal)
{
  return max(dot(normal, -lightDir), 0.0f);
}

float CalcSpecularContribution(vec3 lightDir, vec3 viewDir, vec3 normal, float specularExponent)
{
  vec3 reflectDir = reflect(lightDir, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  return pow(max(dot(normal, halfwayDir), 0.0f), specularExponent);
}


float CalcDirectionLightFactor(vec3 viewDir, vec3 lightDir, vec3 normal)
{
  float diffuseFactor = CalcDiffuseContribution(lightDir, normal);
  float specularFactor = CalcSpecularContribution(normal, viewDir, normal, 1.0f);
  return diffuseFactor * specularFactor;
}


void main()
{
  vec3 position = texture(PositionMap, TexCoord).rgb;
  vec3 normal = texture(NormalMap, TexCoord).rgb;
  vec3 albedo = texture(AlbedoSpecMap, TexCoord).rgb;
  
  vec3 viewDir = normalize(ViewPos - position);
  float directionLightFactor = CalcDirectionLightFactor(viewDir, DirectionalLight.Direction, normal) * DirectionalLight.Intensity;

  FinalColour.rgb = albedo * directionLightFactor * DirectionalLight.Colour.rgb;
  FinalColour.a = 1.0f * DirectionalLight.Colour.a;
}
