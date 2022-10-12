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
  bool SsaoEnabled;
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
uniform sampler2D ShadowMap;
uniform sampler2D OcculusionMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

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

vec3 calculatePositionWS(vec2 screenCoords)
{
  vec2 windowDimensions = textureSize(NormalMap, 0);
  vec3 fragPos = vec3((gl_FragCoord.x / windowDimensions.x), (gl_FragCoord.y / windowDimensions.y), 0.0f);
  fragPos.z = texture(DepthMap, fragPos.xy).r;
  vec4 fragPosProjected = Constants.ProjViewInv * vec4(fragPos * 2.0f - 1.0f, 1.0f);
  fragPos = fragPosProjected.xyz / fragPosProjected.w;
  return fragPos.xyz;
}

void main()
{
  // Rebuild world position of fragment from frag-coord and depth texture
  vec3 position = calculatePositionWS(gl_FragCoord.xy);
  vec2 windowDimensions = textureSize(NormalMap, 0);

  // transform normal vector to range [-1,1]
  vec3 normal = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);
  vec4 albedo = texture(AlbedoMap, TexCoord);
  vec4 specular = texture(SpecularMap, TexCoord);
  float occlusion = texture(OcculusionMap, TexCoord).r;
  float shadowFactor = texture(ShadowMap, TexCoord).r;

  if (Constants.SsaoEnabled == false)
  {
    occlusion = 1.0f;
  }

  vec3 ambient = albedo.rgb * Lighting.AmbientColour * Lighting.AmbientIntensity * occlusion;
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
      finalColour += shadowFactor * (albedo.rgb * diffuseFactor + specular.rgb * specularFactor) * Lighting.Lights[i].Colour * Lighting.Lights[i].Intensity; 
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