#version 410

const float Pi2 = 6.283185307f;

const vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

const vec2 PoissonSamples[64] = vec2[]
(
    vec2(-0.5119625f, -0.4827938f),
    vec2(-0.2171264f, -0.4768726f),
    vec2(-0.7552931f, -0.2426507f),
    vec2(-0.7136765f, -0.4496614f),
    vec2(-0.5938849f, -0.6895654f),
    vec2(-0.3148003f, -0.7047654f),
    vec2(-0.42215f, -0.2024607f),
    vec2(-0.9466816f, -0.2014508f),
    vec2(-0.8409063f, -0.03465778f),
    vec2(-0.6517572f, -0.07476326f),
    vec2(-0.1041822f, -0.02521214f),
    vec2(-0.3042712f, -0.02195431f),
    vec2(-0.5082307f, 0.1079806f),
    vec2(-0.08429877f, -0.2316298f),
    vec2(-0.9879128f, 0.1113683f),
    vec2(-0.3859636f, 0.3363545f),
    vec2(-0.1925334f, 0.1787288f),
    vec2(0.003256182f, 0.138135f),
    vec2(-0.8706837f, 0.3010679f),
    vec2(-0.6982038f, 0.1904326f),
    vec2(0.1975043f, 0.2221317f),
    vec2(0.1507788f, 0.4204168f),
    vec2(0.3514056f, 0.09865579f),
    vec2(0.1558783f, -0.08460935f),
    vec2(-0.0684978f, 0.4461993f),
    vec2(0.3780522f, 0.3478679f),
    vec2(0.3956799f, -0.1469177f),
    vec2(0.5838975f, 0.1054943f),
    vec2(0.6155105f, 0.3245716f),
    vec2(0.3928624f, -0.4417621f),
    vec2(0.1749884f, -0.4202175f),
    vec2(0.6813727f, -0.2424808f),
    vec2(-0.6707711f, 0.4912741f),
    vec2(0.0005130528f, -0.8058334f),
    vec2(0.02703013f, -0.6010728f),
    vec2(-0.1658188f, -0.9695674f),
    vec2(0.4060591f, -0.7100726f),
    vec2(0.7713396f, -0.4713659f),
    vec2(0.573212f, -0.51544f),
    vec2(-0.3448896f, -0.9046497f),
    vec2(0.1268544f, -0.9874692f),
    vec2(0.7418533f, -0.6667366f),
    vec2(0.3492522f, 0.5924662f),
    vec2(0.5679897f, 0.5343465f),
    vec2(0.5663417f, 0.7708698f),
    vec2(0.7375497f, 0.6691415f),
    vec2(0.2271994f, -0.6163502f),
    vec2(0.2312844f, 0.8725659f),
    vec2(0.4216993f, 0.9002838f),
    vec2(0.4262091f, -0.9013284f),
    vec2(0.2001408f, -0.808381f),
    vec2(0.149394f, 0.6650763f),
    vec2(-0.09640376f, 0.9843736f),
    vec2(0.7682328f, -0.07273844f),
    vec2(0.04146584f, 0.8313184f),
    vec2(0.9705266f, -0.1143304f),
    vec2(0.9670017f, 0.1293385f),
    vec2(0.9015037f, -0.3306949f),
    vec2(-0.5085648f, 0.7534177f),
    vec2(0.9055501f, 0.3758393f),
    vec2(0.7599946f, 0.1809109f),
    vec2(-0.2483695f, 0.7942952f),
    vec2(-0.4241052f, 0.5581087f),
    vec2(-0.1020106f, 0.6724468f)
);

layout(std140) uniform LightingConstantsBuffer
{
  mat4 View;
  mat4 ProjViewInv;
  vec3 ViewPosition;
  float FarPlane;
  bool SsaoEnabled;
} Constants;

layout(std140) uniform CascadeShadowMapBuffer
{
  mat4 LightTransforms[4];
  float CascadePlaneDistances[4];
  vec3 LightDirection;
  int CascadeCount;
  bool DrawLayers;
  int SampleCount;
  float SampleSpread;
} CascadeShadowMapData;

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2DArray ShadowMap;
uniform sampler2D RandomRotationsMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out float Shadows;

float sampleShadowMapPcf(vec2 shadowCoords, float shadowDepth, float texelSize, int cascadeLayer, float bias);

float sampleShadowMapPoissonDisc(vec2 shadowCoords, float shadowDepth, float texelSize, int cascadeLayer, float bias, ivec2 screenPos)
{
  // Calculate random rotations for use when sampling the poisson disc
  ivec2 randomRotationsSize = textureSize(RandomRotationsMap, 0);
  vec2 randomSamplePos = screenPos % randomRotationsSize;
  float theta = texture(RandomRotationsMap, randomSamplePos).r * Pi2;
  mat2 randomRotations = mat2(vec2(cos(theta), -sin(theta)),
                              vec2(sin(theta), cos(theta)));

  float result = 0.0;
  for (int i = 0; i < CascadeShadowMapData.SampleCount; i++)
  {
    result += sampleShadowMapPcf(shadowCoords + (randomRotations * PoissonSamples[i]) / CascadeShadowMapData.SampleSpread, shadowDepth, texelSize, cascadeLayer, bias);
  }
  result /= CascadeShadowMapData.SampleCount;
  return result;
}

float sampleShadowMapPcf(vec2 shadowCoords, float shadowDepth, float texelSize, int cascadeLayer, float bias)
{
  // Sample shadow map using percentage closer filtering with 8 samples.
  float shadow = 0.0;
  for (int i = 0; i < 9; i++)
  {
    float pcfDepth = texture(ShadowMap, vec3(shadowCoords + sampleOffsetDirections[i].xy * texelSize, cascadeLayer)).r;
    shadow += (shadowDepth - bias) > pcfDepth ? 0.111111 : 0.0;  
  }
      
  return shadow;
}

float calculateShadowFactor(vec3 fragPosWorldSpace, vec3 normalWorldSpace, ivec2 screenPos)
{
    // Select which shadow map cascade to use
    vec4 fragPosViewSpace = Constants.View * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layerToUse = -1;
    for (int i = 0; i < CascadeShadowMapData.CascadeCount; i++)
    {
      vec4 fragPosLightSpace = CascadeShadowMapData.LightTransforms[i] * vec4(fragPosWorldSpace, 1.0);
      vec3 shadowCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
      shadowCoord = shadowCoord * 0.5 + 0.5;

      if (clamp(shadowCoord.x, 0.0f, 1.0f) == shadowCoord.x && 
          clamp(shadowCoord.y, 0.0f, 1.0f) == shadowCoord.y && 
          clamp(shadowCoord.z, 0.0f, 1.0f) == shadowCoord.z && 
          depthValue < CascadeShadowMapData.CascadePlaneDistances[i])
      {
        layerToUse = i;
        break;
      }
    }

    // Calculates the offset to use for sampling the shadow map, based on the surface normal
    float nDotL = clamp(dot(normalWorldSpace, CascadeShadowMapData.LightDirection), 0.0f, 1.0f);
    vec2 shadowMapSize = textureSize(ShadowMap, 0).xy;
    float texelSize = 1.0f / shadowMapSize.x;
    float nmlOffsetScale = clamp(1.0f - nDotL, 0.0f, 1.0f);
    vec3 shadowPosOffset = texelSize * nmlOffsetScale * normalWorldSpace * 10.f;
    fragPosWorldSpace += shadowPosOffset;

    vec4 fragPosLightSpace = CascadeShadowMapData.LightTransforms[layerToUse] * vec4(fragPosWorldSpace, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
    {
        return 0.0f;
    }

    // Apply bias based on distance from far plane
    vec3 normal = normalize(normalWorldSpace);
    float bias = max(0.05 * (1.0 - dot(normal, CascadeShadowMapData.LightDirection)), 0.005);
    const float biasModifier = 0.5f;
    bias *= 1 / (CascadeShadowMapData.CascadePlaneDistances[layerToUse] * biasModifier);

    return sampleShadowMapPoissonDisc(projCoords.xy, currentDepth, texelSize, layerToUse, bias, screenPos);
}

void main()
{
  // Rebuild position of fragment from frag-coord and depth texture
  vec2 windowDimensions = textureSize(NormalMap, 0);
  vec3 position = vec3((gl_FragCoord.x / windowDimensions.x), (gl_FragCoord.y / windowDimensions.y), 0.0f);
  position.z = texture(DepthMap, position.xy).r;

  // transform normal vector to range [-1,1]
  vec3 normal = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);

  vec4 clip = Constants.ProjViewInv * vec4(position * 2.0f - 1.0f, 1.0f);
  position = clip.xyz / clip.w;

  float shadowFactor = calculateShadowFactor(position, normal, ivec2(TexCoord));
  Shadows = (1.0f - shadowFactor);
}