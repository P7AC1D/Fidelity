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

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

struct ShaderInterface
{
  vec4 Position;
  vec2 TexCoord;
  vec4 Normal;
  mat3 TbnMtx;
  vec3 PositionTS;
  vec3 ViewDirTS;
};

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

layout(location=0) out ShaderInterface vsOut;

mat3 CalcTbnMatrix(vec3 normal, vec3 tangent, vec3 bitangent, mat4 modelView)
{
  vec3 t = normalize(vec3(modelView * vec4(tangent, 0.0f)));
  vec3 b = normalize(vec3(modelView * vec4(bitangent, 0.0f)));
  vec3 n = normalize(vec3(modelView * vec4(normal, 0.0f)));
  return mat3(t, b, n);
}

void main()
{
  mat4 modelView = Object.ModelView;

  vsOut.Position = Object.ModelView * vec4(aPosition, 1.0f);
  vsOut.TexCoord = aTexCoord;
  vsOut.Normal = Object.ModelView * vec4(aNormal, 0.0f);
  vsOut.TbnMtx = CalcTbnMatrix(aNormal, aTangent, aBitangent, Object.ModelView);
  vsOut.PositionTS = vsOut.TbnMtx * vsOut.Position.xyz;
  vsOut.ViewDirTS = vsOut.TbnMtx * normalize(vsOut.Position.xyz - Frame.ViewPos.xyz);
  
  gl_Position = Object.ModelViewProjection * vec4(aPosition, 1.0f);
}
