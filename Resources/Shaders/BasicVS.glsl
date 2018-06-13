#version 410

layout (location = 0) in vec3 aPosition;

layout (std140) uniform Constants
{
  mat4 Projection;
  mat4 View;
};

void main()
{
  gl_Position = Projection * View * vec4(aPosition, 1.0f);
}
