#version 400

struct Material
{
  vec3 AmbientColour;
  vec3 DiffuseColour;
  vec3 SpecularColour;
  float SpecularExponent;
  sampler2D DiffuseMap;
};

layout(std140) uniform Light
{
  vec3 u_lightPosition;
  vec3 u_lightDir;
  vec3 u_lightColour;  
  float u_lightRadius;    
};

layout(std140) uniform Transforms
{
  mat4 u_proj;
  mat4 u_view;
  vec3 u_viewPosition;
};


uniform bool u_diffuseMappingEnabled;
uniform mat4 u_model;
uniform mat4 u_lightSpaceTransform;
uniform Material u_material;
uniform vec3 u_ambientColour;
uniform sampler2D u_shadowMap;
uniform vec2 u_shadowTexelSize;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoords;

out VSOut
{
  vec4 Position;
  vec4 Normal;
  vec2 UV;
  vec4 LightSpacePos;
} vsOut;

void main()
{
  vsOut.Position = u_model * vec4(a_position, 1.0f);
  vsOut.Normal = u_model * vec4(a_normal, 0.0f);
  vsOut.UV = a_texCoords;
  vsOut.LightSpacePos = u_lightSpaceTransform * vsOut.Position;
  gl_Position = u_proj * u_view * vsOut.Position;
}
#endif

#ifdef FRAGMENT_SHADER
in VSOut
{
  vec4 Position;
  vec4 Normal;
  vec2 UV;
  vec4 LightSpacePos;
} fsIn;

out vec4 o_Colour;

float DiffuseContribution(in vec4 lightDir)
{
  return clamp(dot(fsIn.Normal, -lightDir), 0.0f, 1.0f);
}

float SpecularContribution(in vec4 lightDir, in vec4 viewDir, in float specularExponent)
{
  vec4 reflectDir = reflect(lightDir, fsIn.Normal);
  return pow(clamp(dot(reflectDir, viewDir), 0.0f, 1.0f), specularExponent);
}

float SampleShadowMap(in sampler2D shadowMap, in vec2 coords, in float compare)
{
  return step(compare, texture(shadowMap, coords).r);
}

float SampleShadowMapLinear(in sampler2D shadowMap, in vec2 coords, in float compare, in vec2 texelSize)
{
  vec2 pixelPos = coords/texelSize + vec2(0.5);
  vec2 fracPart = fract(pixelPos);
  vec2 startTexel = (pixelPos - fracPart) * texelSize;
  
  float blTexel = SampleShadowMap(shadowMap, startTexel, compare);
  float brTexel = SampleShadowMap(shadowMap, startTexel + vec2(texelSize.x, 0.0), compare);
  float tlTexel = SampleShadowMap(shadowMap, startTexel + vec2(0.0, texelSize.y), compare);
  float trTexel = SampleShadowMap(shadowMap, startTexel + texelSize, compare);
  
  float mixA = mix(blTexel, tlTexel, fracPart.y);
  float mixB = mix(brTexel, trTexel, fracPart.y);
  
  return mix(mixA, mixB, fracPart.x);
}

float SampleShadowMapPCF(sampler2D shadowMap, vec2 coords, float compare, vec2 texelSize)
{
  const float NUM_SAMPLES = 3.0f;
  const float SAMPLES_START = (NUM_SAMPLES-1.0f)/2.0f;
  const float NUM_SAMPLES_SQUARED = NUM_SAMPLES*NUM_SAMPLES;
  
  float result = 0.0f;
  for(float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0f)
  {
    for(float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0f)
    {
      vec2 coordsOffset = vec2(x,y)*texelSize;
      result += SampleShadowMapLinear(shadowMap, coords + coordsOffset, compare, texelSize);
    }
  }
  return result/NUM_SAMPLES_SQUARED;
}

float ShadowContribution(in vec4 fragmentPos)
{
  // move from NDC [-1,1] to ScreenSpace [0,1]
  vec3 shadowCoords = (fragmentPos.xyz / fragmentPos.w) * 0.5f + 0.5f;
  
  float bias = 0.002f;
  return SampleShadowMapPCF(u_shadowMap, shadowCoords.xy, shadowCoords.z - bias, u_shadowTexelSize);
}

void main()
{
  vec3 diffuseSample = vec3(1.0f, 1.0f, 1.0f);
  if (u_diffuseMappingEnabled)
  {
    diffuseSample = texture(u_material.DiffuseMap, fsIn.UV).xyz;
  }

  vec4 lightDir = vec4(u_lightDir, 0.0f);
  vec3 diffuseColour = DiffuseContribution(lightDir) *
                       (u_lightColour * u_material.DiffuseColour * diffuseSample);

  vec4 viewDir = normalize(vec4(u_viewPosition, 1.0f) - fsIn.Position);
  vec3 specularColour = SpecularContribution(lightDir, viewDir, u_material.SpecularExponent) *
                        (u_lightColour * u_material.SpecularColour);
  vec3 ambientColour = u_ambientColour * u_material.AmbientColour;

  float shadowFactor = ShadowContribution(fsIn.LightSpacePos);

  float gamma = 2.2;
  o_Colour = vec4((diffuseColour + specularColour) * shadowFactor + ambientColour, 1.0f);
  o_Colour.rgb = pow(o_Colour.rgb, vec3(1.0 / gamma));
}
#endif
