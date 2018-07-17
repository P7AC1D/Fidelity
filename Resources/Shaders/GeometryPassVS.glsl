#version 410
#include "Common.glsli"

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
  mat4 modelView = Frame.View * Object.Model;

  vsOut.Position = modelView * vec4(aPosition, 1.0f);
  vsOut.TexCoord = aTexCoord;
  vsOut.Normal = modelView * vec4(aNormal, 0.0f);
  vsOut.TbnMtx = CalcTbnMatrix(aNormal, aTangent, aBitangent, modelView);
  vsOut.PositionTS = vsOut.TbnMtx * vsOut.Position.xyz;
  vsOut.ViewDirTS = vsOut.TbnMtx * normalize(vsOut.Position.xyz - Frame.ViewPos.xyz);
  
  gl_Position = Frame.Projection * vsOut.Position;
}
