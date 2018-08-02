#version 410
#include "Common.glsli"
layout(location = 0) in vec3 aPosition;

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
  gl_Position = Object.Model * vec4(aPosition, 1.0f);
}
