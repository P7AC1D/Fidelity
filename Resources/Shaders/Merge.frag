#version 410

uniform sampler2D DiffuseMap;
uniform sampler2D EmissiveMap;
uniform sampler2D SpecularMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

void main()
{
  vec3 diffuse = texture(DiffuseMap, TexCoord).xyz;
  vec3 light = texture(EmissiveMap, TexCoord).xyz;
  vec3 specular = texture(SpecularMap, TexCoord).xyz;

  FinalColour.xyz = diffuse * 0.1f; // ambient
  FinalColour.xyz += diffuse * light; // lambert
  FinalColour.xyz += specular; // specular
  FinalColour.a = 1.0f;
}