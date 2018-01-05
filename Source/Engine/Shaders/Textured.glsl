#version 400

struct Material
{
  sampler2D diffuseMap;
  sampler2D bumpMap;
};

layout(std140) uniform Light
{
  vec3 lightPosition;
  vec3 lightDiffuseColour;
  vec3 lightSpecularColour;
  float lightConstContribution;
  float lightLinearContribution;
  float lightQuadraticContribution;
};

layout(std140) uniform Transforms
{
  mat4 proj;
  mat4 view;
  vec3 viewPosition;
};

uniform mat4 model;
uniform Material material;
uniform vec3 ambientLight;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out VsOut
{
  vec4 position;
  vec4 normal;
  vec2 uv;
} vsOut;

void main()
{
  vsOut.position = model * vec4(position, 1.0f);
  vsOut.normal = model * vec4(normal, 0.0f);
  vsOut.uv = uv;

  gl_Position = proj * view * vsOut.position;
}
#endif

#ifdef FRAGMENT_SHADER
in VsOut
{
  vec4 position;
  vec4 normal;
  vec2 uv;
} fsIn;

out vec4 colour;

float CalcLightAttenuation()
{
  float distance = length(vec4(lightPosition, 1.0f) - fsIn.position);
  return 1.0f / (lightConstContribution + lightLinearContribution * distance + lightQuadraticContribution * distance * distance);
}

float CalcDiffuseContribution()
{
  vec4 lightDir = normalize(fsIn.position - vec4(lightPosition, 1.0f));
  return clamp(dot(fsIn.normal, -lightDir), 0.0f, 1.0f);
}

float CalcSpecularContribution()
{
  vec4 lightDir = normalize(fsIn.position - vec4(lightPosition, 1.0f));
  vec4 reflectDir = reflect(lightDir, fsIn.normal);
  vec4 viewDir = normalize(vec4(viewPosition, 1.0f) - fsIn.position);
  return pow(clamp(dot(reflectDir, viewDir), 0.0f, 1.0f), 32.0f);
}

void main()
{
  vec3 diffuseSample = texture(material.diffuseMap, fsIn.uv).xyz;
  vec3 specularSample = texture(material.bumpMap, fsIn.uv).xyz;
  float attenuation = CalcLightAttenuation();
  float diffuse = CalcDiffuseContribution();
  float specular = CalcSpecularContribution();

  vec3 diffuseColour = diffuseSample * CalcDiffuseContribution() * attenuation * lightDiffuseColour;
  vec3 specularColour = specularSample * CalcSpecularContribution() * attenuation * lightSpecularColour;
  vec3 ambientColour = diffuseSample * ambientLight;

  float gamma = 2.2;
  colour = vec4(diffuseColour + specularColour + ambientColour, 1.0f);
  colour.rgb = pow(colour.rgb, vec3(1.0 / gamma));
}
#endif