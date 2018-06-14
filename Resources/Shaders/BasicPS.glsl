#version 410

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
  FinalColour = vec4(texture(DiffuseMap, TexCoord));
}
