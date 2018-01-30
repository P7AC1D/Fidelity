#version 330

struct Material
{
  vec3 AmbientColour;
  vec3 DiffuseColour;
  vec3 SpecularColour;
  float SpecularExponent;
  sampler2D DiffuseMap;
};

layout(std140) uniform Transforms
{
  mat4 u_proj;
  mat4 u_view;
  vec3 u_viewPosition;
};

uniform bool u_diffuseMappingEnabled;
uniform mat4 u_model;
uniform Material u_material;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoords;

out VSOut
{
  vec4 Position;
  vec4 Normal;
  vec2 TexCoords;
} vsOut;

void main()
{
  vsOut.Position = u_model * vec4(a_position, 1.0f);
  vsOut.Normal = u_model * vec4(a_normal, 0.0f);
  vsOut.TexCoords = a_texCoords;
  gl_Position = u_proj * u_view * vsOut.Position;
}
#endif

#ifdef FRAGMENT_SHADER
in VSOut
{
  vec4 Position;
  vec4 Normal;
  vec2 TexCoords;
} fsIn;

out vec4 o_gPosition;
out vec4 o_gNormal;
out vec4 o_gAlbedoSpec;

void main()
{
  vec3 diffuseSample = vec3(1.0f, 1.0f, 1.0f);
  if (u_diffuseMappingEnabled)
  {
    diffuseSample = texture(u_material.DiffuseMap, fsIn.TexCoords).xyz;
  }

  o_gPosition = fsIn.Position;
  o_gNormal = normalize(fsIn.Normal);
  o_gAlbedoSpec.rgb = u_material.DiffuseColour * diffuseSample;
  o_gAlbedoSpec.a = 1.0f;
}
#endif
