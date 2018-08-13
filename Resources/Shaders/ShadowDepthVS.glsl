#version 410
#include "Common.glsli"
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(std140) uniform DepthBuffer
{
  mat4 Proj;
  mat4 View;
} Depth;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = Depth.Proj * Depth.View * Object.Model * vec4(aPosition, 1.0f);
}
