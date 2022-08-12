#version 410

layout(std140) uniform ObjectBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
} Object;

layout(std140) uniform ShadowBuffer
{
  mat4 Proj;
  mat4 View;
  vec2 TexelDims;
} Shadow;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = Shadow.Proj * Shadow.View * Object.Model * vec4(aPosition, 1.0f);
}
