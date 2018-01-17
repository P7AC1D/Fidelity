#version 400

struct Material
{
  vec3 AmbientColour;
  vec3 DiffuseColour;
  vec3 SpecularColour;
  float SpecularExponent;
  sampler2D DiffuseMap;
};

layout(std140) uniform PointLight
{
  vec3 u_pLightPosition;
  vec3 u_pLightColour;  
  float u_pLightRadius;    
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
uniform vec3 u_ambientColour;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoords;

out VSOut
{
  vec4 Position;
  vec4 Normal;
  vec2 UV;
} vsOut;

void main()
{
  vsOut.Position = u_model * vec4(a_position, 1.0f);
  vsOut.Normal = u_model * vec4(a_normal, 0.0f);
  vsOut.UV = a_texCoords;

  gl_Position = u_proj * u_view * vsOut.Position;
}
#endif

#ifdef FRAGMENT_SHADER
in VSOut
{
  vec4 Position;
  vec4 Normal;
  vec2 UV;
} fsIn;

out vec4 o_Colour;

float LightAttenuation()
{
  float dist = length(vec4(u_pLightPosition, 1.0f) - fsIn.Position);
  float f = 1.0f - dist / (u_pLightRadius);
  float atten = clamp(f, 0.0f, 1.0f);
  atten *= atten;
  return atten;
}

float DiffuseContribution()
{
  vec4 lightDir = normalize(fsIn.Position - vec4(u_pLightPosition, 1.0f));
  return clamp(dot(fsIn.Normal, -lightDir), 0.0f, 1.0f);
}

float SpecularContribution(in float specularExponent)
{
  vec4 lightDir = normalize(fsIn.Position - vec4(u_pLightPosition, 1.0f));
  vec4 reflectDir = reflect(lightDir, fsIn.Normal);
  vec4 viewDir = normalize(vec4(u_viewPosition, 1.0f) - fsIn.Position);
  return pow(clamp(dot(reflectDir, viewDir), 0.0f, 1.0f), specularExponent);
}

void main()
{
  vec3 diffuseSample = vec3(1.0f, 1.0f, 1.0f);
  if (u_diffuseMappingEnabled)
  {
    diffuseSample = texture(u_material.DiffuseMap, fsIn.UV).xyz;
  }

  float attenuation = LightAttenuation();
  vec3 diffuseColour = (DiffuseContribution() * attenuation) * (u_pLightColour * u_material.DiffuseColour * diffuseSample);
  vec3 specularColour = (SpecularContribution(u_material.SpecularExponent) * attenuation) * (u_material.SpecularColour);
  vec3 ambientColour = u_ambientColour * u_material.AmbientColour;

  float gamma = 2.2;
  o_Colour = vec4(diffuseColour + specularColour + ambientColour, 1.0f);
  o_Colour.rgb = pow(o_Colour.rgb, vec3(1.0 / gamma));
}
#endif
