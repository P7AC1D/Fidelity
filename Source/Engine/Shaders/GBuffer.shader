#version 400

struct Material
{
  vec3 AmbientColour;
  vec3 DiffuseColour;
  vec3 SpecularColour;
  float SpecularExponent;
  sampler2D DiffuseMap;
  sampler2D NormalMap;
  sampler2D SpecularMap;
  sampler2D DepthMap;
};

layout(std140) uniform Transforms
{
  mat4 u_proj;
  mat4 u_view;
  vec3 u_viewPosition;
};

uniform bool u_diffuseMappingEnabled;
uniform bool u_normalMappingEnabled;
uniform bool u_specularMappingEnabled;
uniform bool u_depthMappingEnabled;
uniform mat4 u_model;
uniform Material u_material;
uniform vec3 u_viewDir;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoords;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec3 a_bitangent;

out VSOut
{
  mat3 Tbn;
  vec4 Position;
  vec4 Normal;
  vec2 TexCoords;
  vec3 PositionTangent;
  vec3 ViewDirTangent;
} vsOut;

void main()
{
  vec3 t = normalize(vec3(u_model * vec4(a_tangent, 0.0f)));
  vec3 b = normalize(vec3(u_model * vec4(a_bitangent, 0.0f)));
  vec3 n = normalize(vec3(u_model * vec4(a_normal, 0.0f)));

  vsOut.Tbn = mat3(t, b, n);
  vsOut.Position = u_model * vec4(a_position, 1.0f);
  vsOut.Normal = u_model * vec4(a_normal, 0.0f);
  vsOut.TexCoords = a_texCoords;
  gl_Position = u_proj * u_view * vsOut.Position;

  vsOut.ViewDirTangent = vsOut.Tbn * u_viewDir;
  vsOut.PositionTangent = vsOut.Tbn * vsOut.Position.xyz;
}
#endif

#ifdef FRAGMENT_SHADER
in VSOut
{
  mat3 Tbn;
  vec4 Position;
  vec4 Normal;
  vec2 TexCoords;
  vec3 PositionTangent;
  vec3 ViewDirTangent;
} fsIn;

layout(location = 0) out vec4 o_gPosition;
layout(location = 1) out vec4 o_gNormal;
layout(location = 2) out vec4 o_gAlbedoSpec;

vec4 TextureGammaCorrected(in sampler2D textureToSample, in vec2 texCoords)
{
  float gamma = 2.2;
  return pow(texture(textureToSample, texCoords), vec4(gamma));
}

vec2 GetParallaxTexCoords(vec2 texCoords, vec3 viewDir)
{
  const float HEIGHT_SCALE = 0.0f;
  float height = texture(u_material.DepthMap, texCoords).r;
  return texCoords - viewDir.xy * (height * HEIGHT_SCALE);
}

void main()
{
  o_gPosition = fsIn.Position;

  vec2 texCoords = fsIn.TexCoords;
  if (u_depthMappingEnabled)
  {
    vec3 viewDir = normalize(fsIn.ViewDirTangent - fsIn.PositionTangent);
    texCoords = GetParallaxTexCoords(fsIn.TexCoords, viewDir);
    if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    {
      discard;
    }
  }

  vec3 diffuseSample = vec3(1.0f, 1.0f, 1.0f);
  if (u_diffuseMappingEnabled)
  {
    vec3 diffuseSample = TextureGammaCorrected(u_material.DiffuseMap, texCoords).rgb;
    o_gAlbedoSpec.rgb = u_material.DiffuseColour * diffuseSample;
  }
  else
  {
    o_gAlbedoSpec.rgb = u_material.DiffuseColour;
  }

  if (u_normalMappingEnabled)
  {
    vec3 normalSample = texture(u_material.NormalMap, texCoords).rgb;
    vec3 normal = normalSample * 2.0f - 1.0f;
    vec3 normalWorld = normalize(fsIn.Tbn * normal);
    o_gNormal = vec4(normalWorld, 0.0f);
  }
  else
  {
    o_gNormal = normalize(fsIn.Normal);
  }

  if (u_specularMappingEnabled)
  {
    float specularSample = texture(u_material.SpecularMap, texCoords).r;
    o_gAlbedoSpec.a = specularSample;
  }
  else
  {
    o_gAlbedoSpec.a = 1.0f;
  }
}
#endif
