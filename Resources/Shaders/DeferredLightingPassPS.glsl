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

uniform sampler2D PositionMap;
uniform sampler2D NormalMap;
uniform sampler2D AlbedoSpecMap;

out vec4 FinalColour;

void main()
{
  vec3 diffuse = texture(AlbedoSpecMap, fsIn.TexCoord).rgb;
  FinalColour.rgb = diffuse;
  FinalColour.a = 1.0f;
}