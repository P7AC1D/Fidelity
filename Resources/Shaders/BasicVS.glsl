#version 410 core

layout (location = 0) in vec3 aPosition;
layout (location = 4) in vec2 aTexCoord;

layout (std140) uniform Constants
{
  mat4 Projection;
  mat4 View;
};

out vec2 TexCoord;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  TexCoord = aTexCoord;
  gl_Position = Projection * View * vec4(aPosition, 1.0f);
}
