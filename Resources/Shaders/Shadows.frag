#version 410

layout(std140) uniform LightingConstantsBuffer
{
  mat4 View;
  mat4 ProjViewInv;
  vec3 ViewPosition;
  float FarPlane;
  vec2 PixelSize;
} Constants;

layout(std140) uniform CascadeShadowMapBuffer
{
  mat4 LightTransforms[4];
  float CascadePlaneDistances[4];
  vec3 LightDirection;
  int CascadeCount;
  bool DrawLayers;
} CascadeShadowMapData;

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2DArray ShadowMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out float Shadows;

float calculateShadowFactor(vec3 fragPosWorldSpace, vec3 normalWorldSpace)
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

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(ShadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layerToUse)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
        
    return shadow;
}

void main()
{
  // Rebuild world position of fragment from frag-coord and depth texture
  vec3 position = vec3((gl_FragCoord.x * Constants.PixelSize.x), (gl_FragCoord.y * Constants.PixelSize.y), 0.0f);
  position.z = texture(DepthMap, position.xy).r;

  vec3 normal = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);

  vec4 clip = Constants.ProjViewInv * vec4(position * 2.0f - 1.0f, 1.0f);
  position = clip.xyz / clip.w;

  float shadowFactor = calculateShadowFactor(position, normal);
  Shadows = (1.0f - shadowFactor);
}