#version 410

const int MaxKernelSize = 64;

struct DirectionalLightData
{
  vec4 Colour;
  vec3 Direction;
  float Intensity;
};

struct AmbientLightData
{
  vec4 Colour;
  float Intensity;
  float SpecularExponent;
  bool SsaoEnabled;
};

struct SsaoDetailsData
{
  vec4 Samples[MaxKernelSize];
  int KernelSize;
  int QuadWidth;
  int QuadHeight;
  float Radius;
  float Bias;
};

layout(std140) uniform FrameBuffer
{
  mat4 Projection;
  mat4 View;
  DirectionalLightData DirectionalLight;
  vec4 ViewPos;
  AmbientLightData AmbientLight;
  SsaoDetailsData SsaoDetails;
};

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

void main()
{
  vec3 position = texture(PositionMap, TexCoord).rgb;
  vec3 normal = texture(NormalMap, TexCoord).rgb;
  vec3 albedo = texture(AlbedoSpecMap, TexCoord).rgb;
  float specularSample = texture(AlbedoSpecMap, TexCoord).a;
  float ambientOcclusion = texture(SsaoMap, TexCoord).r;
  
  vec3 viewDir = normalize(-position);
  
  float diffuseFactor = CalcDiffuseContribution(DirectionalLight.Direction, normal);
  float specularFactor = CalcSpecularContribution(DirectionalLight.Direction, viewDir, normal, AmbientLight.SpecularExponent);
  
  vec3 ambient = AmbientLight.Colour.rgb * AmbientLight.Intensity * (AmbientLight.SsaoEnabled ? ambientOcclusion : 1.0f);
  vec3 diffuse = DirectionalLight.Colour.rgb * DirectionalLight.Intensity * diffuseFactor;
  vec3 specular = DirectionalLight.Colour.rgb * DirectionalLight.Intensity * specularFactor * specularSample;
  
  FinalColour.rgb = CorrectGamma(albedo) * (ambient + diffuse + specular); 
  FinalColour.a = 1.0f;
}
