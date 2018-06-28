#version 410

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

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

out VsOut
{
  mat3 TbnMtx;
  vec4 Position;
  vec4 Normal;
  vec2 TexCoord;
  vec3 PositionTS;
  vec3 ViewDirTS;
} vsOut;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

mat3 CalcTbnMatrix(vec3 normal, vec3 tangent, vec3 bitangent, mat4 model)
{
  vec3 t = normalize(vec3(model * vec4(tangent, 0.0f)));
  vec3 b = normalize(vec3(model * vec4(bitangent, 0.0f)));
  vec3 n = normalize(vec3(model * vec4(normal, 0.0f)));
  return mat3(t, b, n);
}

void main()
{
  vsOut.TbnMtx = CalcTbnMatrix(aNormal, aTangent, aBitangent, Model);
  vsOut.Position = Model * vec4(aPosition, 1.0f);
  vsOut.Normal = Model * vec4(aNormal, 0.0f);
  vsOut.TexCoord = aTexCoord;
  vsOut.PositionTS = vsOut.TbnMtx * vsOut.Position.xyz;
  vsOut.ViewDirTS = vsOut.TbnMtx * normalize(vsOut.Position.xyz - ViewPos);
  
  gl_Position = Projection * View * vec4(aPosition, 1.0f);
}
