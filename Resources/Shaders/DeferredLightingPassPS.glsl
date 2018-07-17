#version 410
#include "Common.glsli"

uniform sampler2D PositionMap;
uniform sampler2D NormalMap;
uniform sampler2D AlbedoSpecMap;
uniform sampler2D SsaoMap;

layout(location = 0) in vec2 TexCoord;

out vec4 FinalColour;

float CalcDiffuseContribution(vec3 lightDir, vec3 normal)
{
  return max(dot(normal, -lightDir), 0.0f);
}

float CalcSpecularContribution(vec3 lightDir, vec3 viewDir, vec3 normal, float specularExponent)
{
  vec3 halfwayDir = normalize(lightDir + viewDir);
  return pow(max(dot(normal, halfwayDir), 0.0f), specularExponent);
}

vec3 CorrectGamma(vec3 inputSample)
{
  const float GAMMA_INV = 1.0f / 2.2f;
  return pow(inputSample, vec3(GAMMA_INV));
}

vec3 ReinhardToneMapping(vec3 inputSample)
{
  vec3 mapped = inputSample / (inputSample + vec3(1.0f));
  return CorrectGamma(mapped);
}

void main()
{
  vec3 position = texture(PositionMap, TexCoord).rgb;
  vec3 normal = texture(NormalMap, TexCoord).rgb;
  vec3 albedo = texture(AlbedoSpecMap, TexCoord).rgb;
  float specularSample = texture(AlbedoSpecMap, TexCoord).a;
  float ambientOcclusion = texture(SsaoMap, TexCoord).r;
  
  vec3 viewDir = normalize(-position);
  
  float diffuseFactor = CalcDiffuseContribution(Frame.DirectionalLight.Direction, normal);
  float specularFactor = CalcSpecularContribution(Frame.DirectionalLight.Direction, viewDir, normal, Frame.AmbientLight.SpecularExponent);
  
  vec3 ambient = Frame.AmbientLight.Colour.rgb * Frame.AmbientLight.Intensity * (Frame.AmbientLight.SsaoEnabled ? ambientOcclusion : 1.0f);
  vec3 diffuse = Frame.DirectionalLight.Colour.rgb * Frame.DirectionalLight.Intensity * diffuseFactor;
  vec3 specular = Frame.DirectionalLight.Colour.rgb * Frame.DirectionalLight.Intensity * specularFactor * specularSample;
  
  FinalColour.rgb = Frame.Hdr.Enabled ? ReinhardToneMapping(albedo * (ambient + diffuse + specular)) 
                                      : CorrectGamma(albedo) * (ambient + diffuse + specular);
  FinalColour.a = 1.0f;
}
