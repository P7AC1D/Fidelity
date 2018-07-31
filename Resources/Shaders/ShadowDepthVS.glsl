#version 410
#include "Common.glsli"
layout(location = 0) in vec3 aPosition;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = Frame.View * Object.Model * vec4(aPosition, 1.0f);
}
