#version 410

uniform sampler2D QuadTexture;

layout(location = 0) in vec2 TexCoord;

out vec4 FinalColour;

void main()
{
  vec3 diffuse = texture(QuadTexture, TexCoord).rgb;
  FinalColour.rgb = diffuse;
  FinalColour.a = 1.0f;
}
