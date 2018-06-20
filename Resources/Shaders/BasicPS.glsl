#version 410

in vec2 TexCoord;

uniform sampler2D DiffuseMap;

out vec4 FinalColour;

void main()
{
  FinalColour = vec4(texture(DiffuseMap, TexCoord));
}
