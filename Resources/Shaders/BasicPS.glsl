#version 410

layout (std140) uniform Constants
{
  mat4 Projection;
  mat4 View;
};

out vec4 FinalColour;

void main()
{
  FinalColour = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
