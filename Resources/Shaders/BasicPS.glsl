#version 410 core

in vec2 TexCoord;

layout (std140) uniform Constants
{
  mat4 Projection;
  mat4 View;
};

uniform sampler2D DiffuseMap;

out vec4 FinalColour;

void main()
{
  FinalColour = vec4(TexCoord.x, TexCoord.y, 0.0f, 1.0f);
}
