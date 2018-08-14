#version 410

uniform sampler2D QuadTexture;

layout(location = 0) in vec2 TexCoord;

out vec4 FinalColour;

void main()
{
  float depth = texture(QuadTexture, TexCoord).r;
  FinalColour = vec4(depth, depth, depth, 1.0f);
}
