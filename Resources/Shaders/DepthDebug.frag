#version 410

uniform sampler2D QuadTexture;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

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
  float _paddingA;
  float _paddingB;
};

layout(std140) uniform FrameBuffer
{
  mat4 Projection;
  mat4 ProjViewInv;
  mat4 View;
  mat4 ViewInvs; 
  DirectionalLightData DirectionalLight;
  vec4 ViewPos;
  AmbientLightData AmbientLight;  
  SsaoDetailsData SsaoDetails;  
  HdrData Hdr;
  float nearClip;
	float farClip;
} Frame;

float LinearizeDepth(in vec2 uv)
{
    float zNear = Frame.nearClip;
    float zFar  = Frame.farClip;
    float depth = texture2D(QuadTexture, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{             
	float depth = LinearizeDepth(TexCoord);
	FinalColour = vec4(depth, depth, depth, 1.0f);
}
