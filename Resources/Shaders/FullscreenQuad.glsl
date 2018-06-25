#version 410 core

in VsOut
{
  mat3 TbnMtx;
  vec4 Position;
  vec4 Normal;
  vec2 TexCoord;
  vec3 PositionTS;
  vec3 ViewDirTS;
} fsIn;

uniform sampler2D QuadTexture;

out vec4 FinalColour;

void main()
{
  vec3 diffuse = texture(QuadTexture, fsIn.TexCoord).rgb;
  FinalColour.rgb = diffuse;
  FinalColour.a = 1.0f;
}