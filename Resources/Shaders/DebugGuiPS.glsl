#version 410

layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec4 FragColour;

uniform sampler2D DiffuseMap;

out vec4 FinalColour;

void main()
{
  FinalColour = FragColour * vec4(texture(DiffuseMap, TexCoord));
}
