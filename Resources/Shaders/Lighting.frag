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

layout (std140) uniform CascadeShadowMapBuffer
{
  mat4 LightTransforms[4];
  vec3 LightDirection;
  float CascadePlaneDistances[4];
  int CascadeCount;
} CascadeShadowMap;

uniform sampler2D AlbedoMap;
uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;
uniform sampler2DArray ShadowMaps;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

float ShadowCalculation(vec3 fragPosWorldSpace, vec3 normalWorldSpace)
{
    // select cascade layer
    vec4 fragPosViewSpace = Constants.View * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < CascadeShadowMap.CascadeCount; ++i)
    {
        if (depthValue < CascadeShadowMap.CascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = CascadeShadowMap.CascadeCount;
    }

    vec4 fragPosLightSpace = CascadeShadowMap.LightTransforms[layer] * vec4(fragPosWorldSpace, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
 
    vec3 normal = normalize(normalWorldSpace);
    float bias = max(0.05 * (1.0 - dot(normal, CascadeShadowMap.LightDirection)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == CascadeShadowMap.CascadeCount)
    {
        bias *= 1 / (Constants.FarPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (CascadeShadowMap.CascadePlaneDistances[layer] * biasModifier);
    }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(ShadowMaps, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
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
  vec4 albedo = texture(AlbedoMap, TexCoord);
  vec4 specular = texture(SpecularMap, TexCoord);

  vec4 clip = Constants.ProjViewInv * vec4(position * 2.0f - 1.0f, 1.0f);
  position = clip.xyz / clip.w;

  float shadowFactor = ShadowCalculation(position, normal);

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
      vec3 lightDir = Lighting.Lights[i].Direction;
      vec3 halfDir = normalize(lightDir + viewDir);
      float diffuseFactor = clamp(dot(lightDir, normal), 0.0f, 1.0f);
      float specularFactor = pow(max(dot(normal, halfDir), 0.0), specular.a * 255.0f);
      finalColour += (1.0f - shadowFactor) * (albedo.rgb * diffuseFactor + specular.rgb * specularFactor) * Lighting.Lights[i].Colour * Lighting.Lights[i].Intensity; 
    }
  }
  FinalColour = vec4(finalColour.rgb, 1.0f);
}