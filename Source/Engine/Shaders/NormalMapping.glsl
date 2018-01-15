#version 400

struct Material
{
  vec3 ambientColour;
  vec3 diffuseColour;
  vec3 specularColour;
  float specularExponent;
  sampler2D diffuseMap;
  sampler2D specularMap;
  sampler2D normalMap;
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
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

out VsOut
{
  vec4 position;
  vec2 uv;
  vec4 TangentLightPos;
  vec4 TangentViewPos;
  vec4 TangentFragPos;
} vsOut;

void main()
{
  vsOut.position = model * vec4(position, 1.0f);
  vsOut.uv = uv;

  mat3 norm = transpose(inverse(mat3(model)));
  vec3 t = norm * tangent;
  vec3 n = norm * normal;
  vec3 b = norm * bitangent;

  //t = normalize(t - dot(t, n) * n);
  //vec3 b = cross(n, t);

  mat3 tbn = transpose(mat3(t, b, n));

  vsOut.TangentLightPos = vec4(tbn * lightPosition.xyz, 1.0f);
  vsOut.TangentViewPos = vec4(tbn * viewPosition.xyz, 1.0f);
  vsOut.TangentFragPos = vec4(tbn * vsOut.position.xyz, 1.0f);

  gl_Position = proj * view * vsOut.position;
}
#endif

#ifdef FRAGMENT_SHADER
in VsOut
{
  vec4 position;
  vec2 uv;
  vec4 TangentLightPos;
  vec4 TangentViewPos;
  vec4 TangentFragPos;
} fsIn;

out vec4 colour;

float CalcLightAttenuation()
{
  float distance = length(fsIn.TangentLightPos - fsIn.TangentFragPos);
  return 1.0f / (lightConstContribution + lightLinearContribution * distance + lightQuadraticContribution * distance * distance);
}

float CalcDiffuseContribution(in vec4 normal)
{
  vec4 lightDir = normalize(fsIn.TangentFragPos - fsIn.TangentLightPos);
  return clamp(dot(normal, -lightDir), 0.0f, 1.0f);
}

float CalcSpecularContribution(in vec4 normal, in float exponent)
{
  vec4 lightDir = normalize(fsIn.TangentFragPos - fsIn.TangentLightPos);
  vec4 reflectDir = reflect(lightDir, normal);
  vec4 viewDir = normalize(fsIn.TangentViewPos - fsIn.TangentFragPos);
  return pow(clamp(dot(reflectDir, viewDir), 0.0f, 1.0f), exponent);
}

void main()
{
  vec3 diffuseSample = texture(material.diffuseMap, fsIn.uv).xyz;
  vec3 specularSample = texture(material.specularMap, fsIn.uv).xyz;
  vec3 normalSample = texture(material.normalMap, fsIn.uv).xyz;

  float attenuation = CalcLightAttenuation();
  vec4 normal = vec4(normalize(normalSample * 2.0f - 1.0f), 0.0f);

  vec3 diffuseColour = diffuseSample * CalcDiffuseContribution(normal) * attenuation * lightDiffuseColour * material.diffuseColour;
  vec3 specularColour = specularSample * CalcSpecularContribution(normal, material.specularExponent) * attenuation * lightSpecularColour * material.specularColour;
  vec3 ambientColour = diffuseSample * ambientLight * material.ambientColour;

  float gamma = 2.2;
  colour = vec4(diffuseColour + specularColour + ambientColour, 1.0f);
  colour.rgb = pow(colour.rgb, vec3(1.0 / gamma));
}
#endif