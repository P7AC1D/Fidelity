#version 410

layout(location = 0) in vec2 TexCoord;

out float FragColour;

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

layout(std140) uniform ObjectBuffer
{
  mat4 Model;
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

uniform sampler2D SsaoMap;

void main()
{
  vec2 texelSize = 1.0f / vec2(textureSize(SsaoMap, 0));
  float result = 0.0f;
  for (int x = -2; x < 2; ++x) 
  {
    for (int y = -2; y < 2; ++y)
    {
      vec2 offset = vec2(float(x), float(y)) * texelSize;
      result += texture(SsaoMap, TexCoord + offset).r;
    }
  }
  FragColour = result / (4.0f * 4.0f);
}
