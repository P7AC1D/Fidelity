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

layout(location = 0) in vec2 TexCoord;

out float FragColour;

uniform sampler2D PositionMap;
uniform sampler2D NormalMap;
uniform sampler2D NoiseMap;

void main()
{
  vec2 noiseScale = vec2(Frame.SsaoDetails.QuadWidth / 4.0f, Frame.SsaoDetails.QuadHeight / 4.0f);
  
  vec3 position = texture(PositionMap, TexCoord).rgb;
  vec3 normal = normalize(texture(NormalMap, TexCoord).rgb);
  vec3 randomVec = normalize(texture(NoiseMap, TexCoord * noiseScale).rgb);
  
  vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 tbn = mat3(tangent, bitangent, normal);
  
  float occlusion = 0.0f;
  for (int i = 0; i < Frame.SsaoDetails.KernelSize; i++)
  {
    vec3 noiseSample = tbn * Frame.SsaoDetails.Samples[i].xyz;
    noiseSample = position + noiseSample * Frame.SsaoDetails.Radius;
    
    vec4 offset = vec4(noiseSample, 1.0f);
    offset = Frame.Projection * offset;
    offset.xyz /= offset.w;
    offset.xyz = offset.xyz * 0.5f + 0.5f;
    
    float sampleDepth = texture(PositionMap, offset.xy).z;
    
    float rangeCheck = smoothstep(0.0f, 1.0f, Frame.SsaoDetails.Radius / abs(position.z - sampleDepth));
    occlusion += (sampleDepth >= noiseSample.z + Frame.SsaoDetails.Bias ? 1.0f : 0.0f) * rangeCheck;
  }
  occlusion = 1.0f - (occlusion / Frame.SsaoDetails.KernelSize);
  
  FragColour = occlusion;
}
