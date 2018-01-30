#version 330

uniform vec3 u_viewDir;
uniform vec3 u_lightDir;
uniform vec3 u_lightColour;
uniform vec3 u_ambientColour;
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedoSpec;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 2) in vec2 a_texCoords;

out VSOut
{
  vec2 TexCoords;
} vsOut;

void main()
{
  vsOut.TexCoords = a_texCoords;
  gl_Position = vec4(a_position, 1.0f);
}
#endif

#ifdef FRAGMENT_SHADER
in VSOut
{
  vec2 TexCoords;
} fsIn;

out vec4 o_Colour;

float DiffuseContribution(in vec3 lightDir, in vec3 normal)
{
  return max(dot(normal, -lightDir), 0.0f);
}

float SpecularContribution(in vec3 lightDir, in vec3 viewDir, in vec3 normal, in float specularExponent)
{
  vec3 reflectDir = reflect(lightDir, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  return pow(max(dot(normal, halfwayDir), 0.0f), specularExponent);
}

void main()
{
  vec3 fragPos = texture(u_gPosition, fsIn.TexCoords).rgb;
  vec3 normal = texture(u_gNormal, fsIn.TexCoords).rgb;
  vec3 diffuse = texture(u_gAlbedoSpec, fsIn.TexCoords).rgb;
  float specular = texture(u_gAlbedoSpec, fsIn.TexCoords).a;

  vec3 viewDir = normalize(u_viewDir - fragPos);
  float diffuseFactor = DiffuseContribution(u_lightDir, normal);
  float specularFactor = SpecularContribution(u_lightDir, viewDir, normal, 1.0f);

  float gamma = 2.2;
  o_Colour = vec4((diffuse * u_lightColour) * diffuseFactor, 1.0f);
  o_Colour.rgb = pow(o_Colour.rgb, vec3(1.0 / gamma));
}
#endif
