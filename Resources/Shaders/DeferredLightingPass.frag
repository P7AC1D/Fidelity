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
  float _paddingA;
  float _paddingB;
  float _paddingC;
};

struct HdrData
{
  bool Enabled;
  float Exposure;
};

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

layout(std140) uniform FrameBuffer
{
  mat4 Projection;
  mat4 View;
  mat4 ViewInvs; 
  DirectionalLightData DirectionalLight;
  vec4 ViewPos;
  AmbientLightData AmbientLight;  
  SsaoDetailsData SsaoDetails;  
  HdrData Hdr;
} Frame;

layout(std140) uniform MaterialBuffer
{
  TextureMapFlags Enabled;
  vec4 AmbientColour;
  vec4 DiffuseColour;
  vec4 SpecularColour;
  float SpecularExponent;  
} Material;

layout(std140) uniform ShadowBuffer
{
  mat4 Proj;
  mat4 View;
  vec2 TexelDims;
} Shadow;

uniform sampler2D PositionMap;
uniform sampler2D NormalMap;
uniform sampler2D AlbedoSpecMap;
uniform sampler2D SsaoMap;
uniform sampler2D ShadowMap;

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

float SampleShadowMap(vec2 coords)
{
  return texture(ShadowMap, coords).r;
}

float SampleShadowMap(vec2 coords, float compare)
{
  return step(compare, texture(ShadowMap, coords).r);
}

float SampleShadowMapLinear(vec2 coords, float compare, vec2 texelSize)
{
  vec2 pixelPos = coords / texelSize + vec2(0.5);
  vec2 fracPart = fract(pixelPos);
  vec2 startTexel = (pixelPos - fracPart) * texelSize;
  
  float blTexel = SampleShadowMap(startTexel, compare);
  float brTexel = SampleShadowMap(startTexel + vec2(texelSize.x, 0.0), compare);
  float tlTexel = SampleShadowMap(startTexel + vec2(0.0, texelSize.y), compare);
  float trTexel = SampleShadowMap(startTexel + texelSize, compare);
  
  float mixA = mix(blTexel, tlTexel, fracPart.y);
  float mixB = mix(brTexel, trTexel, fracPart.y);
  
  return mix(mixA, mixB, fracPart.x);
}

float SampleShadowMapPCF(vec2 coords, float compare, vec2 texelSize)
{
  const float NUM_SAMPLES = 5.0f;
  const float SAMPLES_START = (NUM_SAMPLES - 1.0f) / 2.0f;
  const float NUM_SAMPLES_SQUARED = NUM_SAMPLES * NUM_SAMPLES;
  
  float result = 0.0f;
  for (float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0f)
  {
    for (float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0f)
    {
      vec2 coordsOffset = vec2(x, y)*texelSize;
      result += SampleShadowMapLinear(coords + coordsOffset, compare, texelSize);
    }
  }
  return result / NUM_SAMPLES_SQUARED;
}

float CalcShadowContrib(vec4 fragmentPos)
{
  vec3 shadowCoords = (fragmentPos.xyz / fragmentPos.w) * 0.5f + 0.5f;
  
  float bias = 0.002f;
  return SampleShadowMapPCF(shadowCoords.xy, shadowCoords.z - bias, Shadow.TexelDims);
}

void main()
{
  vec3 position = texture(PositionMap, TexCoord).rgb;
  vec3 normal = texture(NormalMap, TexCoord).rgb;
  vec3 albedo = texture(AlbedoSpecMap, TexCoord).rgb;
  float specularSample = texture(AlbedoSpecMap, TexCoord).a;
  float ambientOcclusion = texture(SsaoMap, TexCoord).r;
  
  vec3 viewDir = normalize(-position);
  vec3 lightDir = (Frame.View * vec4(Frame.DirectionalLight.Direction, 0.0f)).xyz;
  
  float diffuseFactor = CalcDiffuseContribution(lightDir, normal);
  float specularFactor = CalcSpecularContribution(lightDir, viewDir, normal, Frame.AmbientLight.SpecularExponent);
  
  vec3 ambient = Frame.AmbientLight.Colour.rgb * Frame.AmbientLight.Intensity * (Frame.AmbientLight.SsaoEnabled ? ambientOcclusion : 1.0f);
  vec3 diffuse = Frame.DirectionalLight.Colour.rgb * Frame.DirectionalLight.Intensity * diffuseFactor;
  vec3 specular = Frame.DirectionalLight.Colour.rgb * Frame.DirectionalLight.Intensity * specularFactor * specularSample;
  
  vec4 fragPosLightSpace = Shadow.Proj * Shadow.View * Frame.ViewInvs * vec4(position, 1.0f);
  float shadowFactor = CalcShadowContrib(fragPosLightSpace);
  
  FinalColour.rgb = Frame.Hdr.Enabled ? ReinhardToneMapping(albedo * (ambient + shadowFactor * (diffuse + specular)))
                                      : CorrectGamma(albedo) * (ambient + shadowFactor * (diffuse + specular));
  FinalColour.a = 1.0f;
}
