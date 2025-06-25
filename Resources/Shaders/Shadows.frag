#version 410

#define DEBUG_SHADOW_ARTIFACTS 0

const int MAX_LIGHTS = 1024;
const int MAX_CASCADE_LAYERS = 8;
const float Pi2 = 6.283185307f;

// TODO: Make this a constant buffer
const vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

// TODO: Make this a constant buffer
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

/**
 * Fidelity Engine - Optimized Shadow Mapping Implementation
 * 
 * Improvements implemented:
 * 1. Optimized PCF with early termination for better performance
 * 2. Contact hardening shadows with variable penumbra size
 * 3. Improved bias calculation using slope and distance-based methods
 * 4. Reduced aggressive shadow sample offset for better quality
 * 5. Better cascade layer validation and error handling
 */

struct Light
{
  vec3 Colour;
  float Intensity;
  vec3 Position;
  float Radius;
};

layout(std140) uniform PerFrameBuffer
{
  mat4 CascadeLightTransforms[MAX_CASCADE_LAYERS];
  mat4 View;
  mat4 Proj;
  mat4 ProjInv;
  mat4 ProjViewInv;
  vec3 ViewPosition;
  float FarPlane;
  vec3 LightDirection;   // ---- Directional Light ----
  bool SsaoEnabled;
  vec3 LightColour;      // ---- Directional Light ----
  float LightIntensity;  // ---- Directional Light ----
  // ---------------------------
  vec3 AmbientColour;
  float AmbientIntensity;
  uint CascadeLayerCount;  
  bool DrawCascadeLayers;
  uint ShadowSampleCount;
  float ShadowSampleSpread;
  Light Lights[MAX_LIGHTS];
  float CascadePlaneDistances[MAX_CASCADE_LAYERS];
  uint LightCount;
} Constants;

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2DArray ShadowMap;
uniform sampler2D RandomRotationsMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out float Shadows;

float sampleShadowMapPcf(vec2 shadowCoords, float shadowDepth, float texelSize, int cascadeLayer, float bias)
{
  // Optimized PCF with early termination for performance
  float shadow = 0.0;
  int litCount = 0;
  int shadowedCount = 0;
  
  for (int i = 0; i < 9 && litCount < 5 && shadowedCount < 5; i++)
  {
    float pcfDepth = texture(ShadowMap, vec3(shadowCoords + sampleOffsetDirections[i].xy * texelSize, cascadeLayer)).r;
    bool inShadow = (shadowDepth - bias) >= pcfDepth;
    
    if (inShadow) {
      shadow += 0.111111;
      shadowedCount++;
    } else {
      litCount++;
    }
  }
  
  // Early termination optimization - if all remaining samples would be the same, skip them
  if (litCount >= 5) {
    // Mostly lit, remaining samples won't change result significantly
    return shadow;
  } else if (shadowedCount >= 5) {
    // Mostly shadowed, add remaining shadow contribution
    return shadow + (9 - litCount - shadowedCount) * 0.111111;
  }
      
  return shadow;
}

float findBlockerDistance(vec2 shadowCoords, float shadowDepth, int cascadeLayer, float texelSize, float searchRadius)
{
  float blockerSum = 0.0;
  int blockerCount = 0;
  
  // Search for blockers in a small area around the fragment
  for (int i = 0; i < 16; i++) // Use subset of Poisson samples for blocker search
  {
    vec2 sampleCoord = shadowCoords + PoissonSamples[i] * searchRadius * texelSize;
    float blockerDepth = texture(ShadowMap, vec3(sampleCoord, cascadeLayer)).r;
    
    if (blockerDepth < shadowDepth - 0.001) // Small epsilon to avoid self-shadowing
    {
      blockerSum += blockerDepth;
      blockerCount++;
    }
  }
  
  return blockerCount > 0 ? blockerSum / float(blockerCount) : shadowDepth;
}

float sampleShadowMapPoissonDisc(vec2 shadowCoords, float shadowDepth, float texelSize, int cascadeLayer, float bias, ivec2 screenPos)
{
  // Calculate random rotations for use when sampling the poisson disc
  ivec2 randomRotationsSize = textureSize(RandomRotationsMap, 0);
  vec2 randomSamplePos = screenPos % randomRotationsSize;
  float theta = texture(RandomRotationsMap, randomSamplePos).r * Pi2;
  mat2 randomRotations = mat2(vec2(cos(theta), -sin(theta)),
                              vec2(sin(theta), cos(theta)));

  // Contact hardening: variable penumbra size based on blocker distance
  float blockerDistance = findBlockerDistance(shadowCoords, shadowDepth, cascadeLayer, texelSize, 3.0);
  float penumbraSize = max((shadowDepth - blockerDistance) / blockerDistance, 0.01);
  float adaptiveSpread = Constants.ShadowSampleSpread * (1.0 + penumbraSize * 2.0);

  float result = 0.0;
  for (int i = 0; i < Constants.ShadowSampleCount; i++)
  {
    result += sampleShadowMapPcf(shadowCoords + (randomRotations * PoissonSamples[i]) / adaptiveSpread, shadowDepth, texelSize, cascadeLayer, bias);
  }
  result /= Constants.ShadowSampleCount;
  return result;
}

int calculateCascadeLayer(vec3 fragPosWorldSpace, float depthValue)
{
  int layerToUse = -1;
  for (int i = 0; i < Constants.CascadeLayerCount; i++)
  {
    vec4 fragPosLightSpace = Constants.CascadeLightTransforms[i] * vec4(fragPosWorldSpace, 1.0);
    vec3 shadowCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    shadowCoord = shadowCoord * 0.5 + 0.5;

    if (clamp(shadowCoord.x, 0.0f, 1.0f) == shadowCoord.x && 
        clamp(shadowCoord.y, 0.0f, 1.0f) == shadowCoord.y && 
        clamp(shadowCoord.z, 0.0f, 1.0f) == shadowCoord.z && 
        depthValue < Constants.CascadePlaneDistances[i])
    {
      layerToUse = i;
      break;
    }
  }
  return layerToUse;
}

vec3 calculateShadowSampleOffset(vec3 normalWorldSpace, float shadowTexelSize, float depthValue)
{
  float nDotL = clamp(dot(normalWorldSpace, Constants.LightDirection), 0.0f, 1.0f);
  
  // More adaptive offset based on slope and distance
  float slopeScale = clamp(1.0f - nDotL, 0.0f, 1.0f);
  float depthScale = 1.0 + depthValue * 0.001; // Slight increase with distance
  
  // Reduce aggressive multiplier from 10.0 to 2.0 for better quality
  vec3 shadowPosOffset = shadowTexelSize * slopeScale * normalWorldSpace * 2.0f * depthScale;
  return shadowPosOffset;
}

float calculateOptimizedBias(vec3 normalWorldSpace, float depthValue, int cascadeLayer)
{
  float nDotL = clamp(dot(normalize(normalWorldSpace), Constants.LightDirection), 0.0f, 1.0f);
  
  // Slope-based bias using more accurate slope calculation
  float slopeBias = clamp(tan(acos(nDotL)), 0.0, 0.01);
  
  // Distance-based bias that scales with depth
  float distanceBias = 0.001 * (1.0 + depthValue * 0.1);
  
  // Cascade-based bias scaling for different resolution levels
  float cascadeScale = 1.0 / (Constants.CascadePlaneDistances[cascadeLayer] * 0.3);
  
  return (slopeBias + distanceBias) * cascadeScale;
}

float calculateShadowFactor(vec3 fragPosWorldSpace, vec3 normalWorldSpace, ivec2 screenPos)
{
    // Transform position to view space and sample depth.
    vec4 fragPosViewSpace = Constants.View * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    vec2 shadowMapSize = textureSize(ShadowMap, 0).xy;
    float texelSize = 1.0f / shadowMapSize.x;

    int layerToUse = calculateCascadeLayer(fragPosWorldSpace, depthValue);
    
    // Early exit if no valid cascade found
    if (layerToUse == -1) {
        return 0.0f; // No shadow
    }

    // Calculates the offset to use for sampling the shadow map, based on the surface normal
#ifdef DEBUG_SHADOW_ARTIFACTS
    // Bypass normal-based offset for debugging artifacts
    vec3 shadowSampleOffset = vec3(0.0);
#else
    vec3 shadowSampleOffset = calculateShadowSampleOffset(normalWorldSpace, texelSize, depthValue);
#endif
    fragPosWorldSpace += shadowSampleOffset;

    vec4 fragPosLightSpace = Constants.CascadeLightTransforms[layerToUse] * vec4(fragPosWorldSpace, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    // In debug modes >=3, skip far-plane cutoff to visualize full depth range
    #if DEBUG_SHADOW_ARTIFACTS < 3
        if (currentDepth > 1.0)
        {
            return 0.0f;
        }
    #endif

    // Use optimized bias calculation
    float bias = calculateOptimizedBias(normalWorldSpace, depthValue, layerToUse);

    // Debugging artifact modes:
    // 1: simple PCF without offset
    // 2: visualize bias value
    // 3: visualize light-space depth (currentDepth)
    // 4: visualize selected cascade layer index
    #if DEBUG_SHADOW_ARTIFACTS == 2
        // Visualize raw slope+distance bias without cascade scale, plus dithering to reduce banding
        {
            float nDotL = clamp(dot(normalWorldSpace, Constants.LightDirection), 0.0, 1.0);
            float slopeBias = clamp(tan(acos(nDotL)), 0.0, 0.01);
            float distanceBias = 0.001 * (1.0 + depthValue * 0.1);
            float rawBias = slopeBias + distanceBias;
            float maxRaw = 0.02; // adjust to fit your range
            // dither noise based on screen position
            float rand = fract(sin(dot(vec2(screenPos), vec2(12.9898,78.233))) * 43758.5453);
            float dither = (rand - 0.5) / 256.0;
            return clamp(rawBias / maxRaw + dither, 0.0, 1.0);
        }
    #elif DEBUG_SHADOW_ARTIFACTS == 3
        // Global normalized view-space depth for continuous gradient
        return clamp(depthValue / Constants.FarPlane, 0.0, 1.0);
    #elif DEBUG_SHADOW_ARTIFACTS == 4
        return float(layerToUse) / float(Constants.CascadeLayerCount);
    #elif DEBUG_SHADOW_ARTIFACTS == 1
        // Use simple PCF sampling for debugging artifacts
        return sampleShadowMapPcf(projCoords.xy, currentDepth, texelSize, layerToUse, bias);
    #else
        return sampleShadowMapPoissonDisc(projCoords.xy, currentDepth, texelSize, layerToUse, bias, screenPos);
    #endif
}

void main()
{
  // Rebuild position of fragment in screen space from frag-coord and depth texture
  vec2 windowDimensions = textureSize(NormalMap, 0);
  vec3 position = vec3((gl_FragCoord.x / windowDimensions.x), (gl_FragCoord.y / windowDimensions.y), 0.0f);
  position.z = texture(DepthMap, position.xy).r;

  // Transform normal vector to range [-1,1]
  vec3 normal = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);

  // Transform position to NDC space.
  position = position * 2.0f - 1.0f;

  // Transform position to world space and perform perspective divide.
  vec4 clip = Constants.ProjViewInv * vec4(position, 1.0f);
  position = clip.xyz / clip.w;

  // Use proper screen coordinates for Poisson disc sampling
  ivec2 screenPos = ivec2(gl_FragCoord.xy);
  float shadowFactor = calculateShadowFactor(position, normal, screenPos);
  Shadows = (1.0f - shadowFactor);
}