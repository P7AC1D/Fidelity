#version 410

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(std140) uniform PerObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
  vec4 DiffuseColour;
  bool DiffuseEnabled;
  bool NormalEnabled;
  bool MetalnessEnabled;
  bool RoughnessEnabled;
  bool OcclusionEnabled;
  bool OpacityEnabled;
  float Metalness;
  float Roughness;
} Object;

struct Output
{
  vec3 WorldPos;
  vec2 TexCoord;
  vec3 Normal;
  vec3 Tangent;
  vec3 Binormal;
};

layout(location = 0) out Output vsOut;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  mat3 normalMatrix = transpose(inverse(mat3(Object.Model)));

  vsOut.TexCoord = aTexCoord;
  vsOut.Normal = normalize(normalMatrix * normalize(aNormal));
  vsOut.Tangent = normalize(normalMatrix * normalize(aTangent));
  vsOut.Binormal = normalize(normalMatrix * normalize(aBitangent));
  vsOut.WorldPos = (Object.Model * vec4(aPosition, 1.0f)).xyz;

  gl_Position = Object.ModelViewProjection * vec4(aPosition, 1.0f);
}