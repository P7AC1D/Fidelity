#version 410

layout (location = 0) in vec3 aPosition;

layout (std140) uniform Constants
{
  mat4 Projection;
  mat4 View;
};

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  gl_Position = Projection * View * vec4(aPosition, 1.0f);
}
