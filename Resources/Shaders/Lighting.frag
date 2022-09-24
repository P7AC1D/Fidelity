#version 410

const int MAX_LIGHTS = 32;

struct Light
{
  vec3 Colour;
  float Intensity;
  vec3 Position;
  float Radius;
  vec3 Direction;
  int LightType; // 0 - Point; 1 - Directional
};

layout(std140) uniform LightingConstantsBuffer
{
  mat4 View;
  mat4 ProjViewInv;
  vec3 ViewPosition;
  float FarPlane;
  vec2 PixelSize;
} Constants;

layout(std140) uniform LightingBuffer
{
  vec3 AmbientColour;
  float AmbientIntensity;
  Light Lights[MAX_LIGHTS];
} Lighting;

layout(std140) uniform CascadeShadowMapBuffer
{
  mat4 LightTransforms[4];
  float CascadePlaneDistances[4];
  vec3 LightDirection;
  int CascadeCount;
  bool DrawLayers;
} CascadeShadowMapData;

uniform sampler2D AlbedoMap;
uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;
uniform sampler2DArray ShadowMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

float calculateShadowFactor(vec3 fragPosWorldSpace, vec3 normalWorldSpace)
{
    // Select which shadow map cascade to use
    vec4 fragPosViewSpace = Constants.View * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layerToUse = -1;
    for (int i = 0; i < CascadeShadowMapData.CascadeCount; i++)
    {
      if (depthValue < CascadeShadowMapData.CascadePlaneDistances[i])
      {
        layerToUse = i;
        break;
      }
    }

    if (layerToUse == -1)
    {
      return 0.0f;
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

vec3 drawCascadeLayers(vec3 position)
{
  vec3 cascadeDebugColour = vec3(0.0,0.0,0.0);
  vec4 fragPosViewSpace = Constants.View * vec4(position, 1.0);
  float depthValue = abs(fragPosViewSpace.z);

  int layerToUse = -1;
  for (int i = 0; i < 4; i++)
  {
    if (depthValue < CascadeShadowMapData.CascadePlaneDistances[i])
    {
      layerToUse = i;
      break;
    }
  }

  if (layerToUse == 0)
  {
    cascadeDebugColour = vec3(1.0,0.0,0.0);
  }
  else if (layerToUse == 1)
  {
    cascadeDebugColour = vec3(0.0,1.0,0.0);
  }
  else if (layerToUse == 2)
  {
    cascadeDebugColour = vec3(0.0,0.0,1.0);
  }
  else if (layerToUse == 2)
  {
    cascadeDebugColour = vec3(1.0,0.0,1.0);
  }
  return cascadeDebugColour;
}

void main()
{
  // Rebuild world position of fragment from frag-coord and depth texture
  vec3 position = vec3((gl_FragCoord.x * Constants.PixelSize.x), (gl_FragCoord.y * Constants.PixelSize.y), 0.0f);
  position.z = texture(DepthMap, position.xy).r;

  vec3 normal = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);
  vec4 albedo = texture(AlbedoMap, TexCoord);
  vec4 specular = texture(SpecularMap, TexCoord);

  vec4 clip = Constants.ProjViewInv * vec4(position * 2.0f - 1.0f, 1.0f);
  position = clip.xyz / clip.w;

  float shadowFactor = calculateShadowFactor(position, normal);
  vec3 ambient = albedo.rgb * Lighting.AmbientColour * Lighting.AmbientIntensity;
  vec3 finalColour = ambient;
  vec3 viewDir = normalize(Constants.ViewPosition - position);
  for (int i = 0; i < 4; i++)
  {
    if (Lighting.Lights[i].LightType == 0)
    {
      float distance = length(Lighting.Lights[i].Position - position);
      float attenuation = 1.0f - clamp(distance / Lighting.Lights[i].Radius, 0.0f, 1.0f);
      if (attenuation > 0.0f)
      {
        vec3 lightDir = normalize(Lighting.Lights[i].Position - position);
        vec3 halfDir = normalize(lightDir + viewDir);
        float diffuseFactor = clamp(dot(lightDir, normal), 0.0f, 1.0f) * attenuation;
        float specularFactor = pow(max(dot(normal, halfDir), 0.0), specular.a * 255.0f) * attenuation;
        finalColour += (albedo.rgb * diffuseFactor + specular.rgb * specularFactor) * Lighting.Lights[i].Colour * Lighting.Lights[i].Intensity; 
      }
    }
    else if (Lighting.Lights[i].LightType == 1)
    {
      vec3 lightDir = normalize(-Lighting.Lights[i].Direction);
      vec3 halfDir = normalize(lightDir + viewDir);
      float diffuseFactor = clamp(dot(lightDir, normal), 0.0f, 1.0f);
      float specularFactor = pow(max(dot(normal, halfDir), 0.0), specular.a * 255.0f);
      finalColour += (1.0f - shadowFactor) * (albedo.rgb * diffuseFactor + specular.rgb * specularFactor) * Lighting.Lights[i].Colour * Lighting.Lights[i].Intensity; 
    }
  }
  if (CascadeShadowMapData.DrawLayers)
  {
    FinalColour = vec4(finalColour.rgb * drawCascadeLayers(position), 1.0f);
  }
  else
  {
    FinalColour = vec4(finalColour.rgb, 1.0f);
  }
}