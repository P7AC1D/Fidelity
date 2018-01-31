#version 400

uniform vec2 u_shadowTexelSize;
uniform vec3 u_viewDir;
uniform vec3 u_lightDir;
uniform vec3 u_lightColour;
uniform mat4 u_lightTransform;
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedoSpec;
uniform sampler2D u_dirLightDepth;

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

float SampleShadowMap(in sampler2D shadowMap, in vec2 coords, in float compare)
{
  return step(compare, texture(shadowMap, coords).r);
}

float SampleShadowMapLinear(in sampler2D shadowMap, in vec2 coords, in float compare, in vec2 texelSize)
{
  vec2 pixelPos = coords / texelSize + vec2(0.5);
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
  const float NUM_SAMPLES = 5.0f;
  const float SAMPLES_START = (NUM_SAMPLES - 1.0f) / 2.0f;
  const float NUM_SAMPLES_SQUARED = NUM_SAMPLES*NUM_SAMPLES;

  float result = 0.0f;
  for (float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0f)
  {
    for (float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0f)
    {
      vec2 coordsOffset = vec2(x, y)*texelSize;
      result += SampleShadowMapLinear(shadowMap, coords + coordsOffset, compare, texelSize);
    }
  }
  return result / NUM_SAMPLES_SQUARED;
}

float ShadowContribution(in vec4 fragmentPos)
{
  // move from NDC [-1,1] to ScreenSpace [0,1]
  vec3 shadowCoords = (fragmentPos.xyz / fragmentPos.w) * 0.5f + 0.5f;

  float bias = 0.002f;
  return SampleShadowMapPCF(u_dirLightDepth, shadowCoords.xy, shadowCoords.z - bias, u_shadowTexelSize);
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

  vec4 lightSpacePosition = u_lightTransform * vec4(fragPos, 1.0f);
  float shadowFactor = ShadowContribution(lightSpacePosition);

  float gamma = 2.2;
  o_Colour = vec4((diffuse * u_lightColour) * diffuseFactor * shadowFactor, 1.0f);
  o_Colour.rgb = pow(o_Colour.rgb, vec3(1.0 / gamma));
}
#endif
