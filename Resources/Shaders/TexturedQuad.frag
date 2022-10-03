#version 410

layout(location = 0) in vec2 TexCoord;

uniform sampler2D TextureMap;

out vec4 FinalColour;

void main()
{
  FinalColour = vec4(texture(TextureMap, TexCoord));
}
