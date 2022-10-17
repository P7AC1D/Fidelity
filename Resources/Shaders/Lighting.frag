#version 410

const int MAX_LIGHTS = 1024;
const int MAX_CASCADE_LAYERS = 8;

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
    if (depthValue < Constants.CascadePlaneDistances[i])
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

  vec3 ambient = albedo.rgb * Constants.AmbientColour * Constants.AmbientIntensity * occlusion;
  vec3 finalColour = ambient;
  vec3 viewDir = normalize(Constants.ViewPosition - position);

  vec3 lightDir = normalize(-Constants.LightDirection);
  vec3 halfDir = normalize(lightDir + viewDir);
  float diffuseFactor = clamp(dot(lightDir, normal), 0.0f, 1.0f);
  float specularFactor = pow(max(dot(normal, halfDir), 0.0), specular.a * 255.0f);
  finalColour += shadowFactor * (albedo.rgb * diffuseFactor + specular.rgb * specularFactor) * Constants.LightColour * Constants.LightIntensity; 

  for (int i = 0; i < Constants.LightCount; i++)
  {
    float distance = length(Constants.Lights[i].Position - position);
    float attenuation = 1.0f - clamp(distance / Constants.Lights[i].Radius, 0.0f, 1.0f);
    if (attenuation > 0.0f)
    {
      vec3 lightDir = normalize(Constants.Lights[i].Position - position);
      vec3 halfDir = normalize(lightDir + viewDir);
      float diffuseFactor = clamp(dot(lightDir, normal), 0.0f, 1.0f) * attenuation;
      float specularFactor = pow(max(dot(normal, halfDir), 0.0), specular.a * 255.0f) * attenuation;
      finalColour += (albedo.rgb * diffuseFactor + specular.rgb * specularFactor) * Constants.Lights[i].Colour * Constants.Lights[i].Intensity; 
    }
  }

  if (Constants.DrawCascadeLayers)
  {
    FinalColour = vec4(finalColour.rgb * drawCascadeLayers(position), 1.0f);
  }
  else
  {
    FinalColour = vec4(finalColour.rgb, 1.0f);
  }
}