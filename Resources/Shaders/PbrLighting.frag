#version 410

#define M_PI 3.1415926535897932384626433832795

const int MAX_LIGHTS = 1024;
const int MAX_CASCADE_LAYERS = 4;

struct Light
{
  vec3 Colour;
  float Intensity;
  vec3 Position;
  float Radius;
};

const vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

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
  vec3 AmbientColour;
  float AmbientIntensity;
  uint CascadeLayerCount;  
  bool DrawCascadeLayers;
  uint ShadowSampleCount;
  float ShadowSampleSpread;
  Light Lights[MAX_LIGHTS];
  float CascadePlaneDistances[MAX_CASCADE_LAYERS];
  uint LightCount;
  float Exposure;
  bool ToneMappingEnabled;
  float BloomStrength;
  float BloomThreshold;
} Constants;

uniform sampler2D AlbedoMap;
uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D MaterialMap;
uniform sampler2D ShadowMask; // precomputed directional shadow mask
uniform sampler2D OcclusionMap;
uniform samplerCube PointShadowMap; // point-light shadow cubemap

layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec4 FinalColour;
layout(location = 1) out vec4 BloomColour;

// PBR function declarations.
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float distributionGGX(vec3 N, vec3 H, float rough);
float geometrySchlickGGX(float nDotV, float rough);
float geometrySmith(float nDotV, float nDotL, float rough);

// Light function declarations.
vec3 calcDirLight(vec3 lightDirection,
                  vec3 lightColour,
                  float lightIntensity,
                  vec3 normal, 
                  vec3 viewDir, 
                  vec3 albedo, 
                  float roughness, 
                  float metalness, 
                  float shadowFactor, 
                  vec3 F0);
vec3 calcPointLight(Light light, 
                    vec3 normal, 
                    vec3 fragPos, 
                    vec3 viewDir, 
                    vec3 albedo, 
                    float roughness, 
                    float metalness, 
                    vec3 F0);


vec3 drawCascadeLayers(vec3 position)
{
  // Reconstruct view-space depth
  float depthValue = abs((Constants.View * vec4(position, 1.0)).z);
  // Predefined debug colors for up to 8 cascades
  vec3 colors[8] = vec3[]( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(1,0,1),
                           vec3(1,1,0), vec3(0,1,1), vec3(1,1,1), vec3(0.5,0.5,0.5) );
  int maxLayers = int(Constants.CascadeLayerCount);
  for (int i = 0; i < maxLayers; ++i)
  {
    if (depthValue < Constants.CascadePlaneDistances[i])
    {
      return colors[i];
    }
  }
  // Default color if beyond last cascade
  return vec3(0.0);
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

float getPointShadowPcf(Light light, vec3 fragPos, float bias) {
    vec3 fragToLight = fragPos - light.Position;
    float currentDepth = length(fragToLight);

    float shadow = 0.0f;
    float diskRadius = 0.05f; // Adjust radius for point light shadow softening

    for (int i = 0; i < Constants.ShadowSampleCount; i++) {
        // Sample offset based on sample index and disk radius
        vec3 sampleOffset = sampleOffsetDirections[i] * diskRadius;
        float closestDepth = texture(PointShadowMap, fragToLight + sampleOffset).r;
        closestDepth *= light.Radius; // undo [0, 1] mapping

        if (currentDepth - bias >= closestDepth) {
            shadow += 1.0f;
        }
    }

    shadow /= float(Constants.ShadowSampleCount);
    return shadow;
}

void main()
{
  // Rebuild world position of fragment from frag-coord and depth texture.
  vec3 position = calculatePositionWS(gl_FragCoord.xy);
  vec2 windowDimensions = textureSize(NormalMap, 0);

  // Sample combined directional shadow mask
  float shadowFactor = texture(ShadowMask, TexCoord).r;
  
  float occlusionFactor = texture(OcclusionMap, TexCoord).r;
  if (Constants.SsaoEnabled == false)
  {
    occlusionFactor = 1.0f;
  }

  // transform normal vector to range [-1,1]
  vec3 normal = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);
  vec3 albedo = texture(AlbedoMap, TexCoord).rgb;
  vec3 material = texture(MaterialMap, TexCoord).rgb;

  float metalness = material.r;
  float roughness = material.g;

  // Components common to all light types.
  vec3 viewDir = normalize(Constants.ViewPosition - position);
  vec3 R = reflect(-viewDir, normal);

  // Correcting zero incidence reflection.
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metalness);

  vec3 totalRadiance = vec3(0.0);  

  // Directional light contribution.
  totalRadiance += calcDirLight(Constants.LightDirection,
                                 Constants.LightColour, 
                                 Constants.LightIntensity, 
                                 normal, 
                                 viewDir,
                                 albedo, 
                                 roughness, 
                                 metalness, 
                                 shadowFactor, 
                                 F0);

  // Point light contributions.
  for (int i = 0; i < Constants.LightCount; i++)
  {
    Light currentLight = Constants.Lights[i];
    vec3 toLight = position - currentLight.Position;
    vec3 dir = normalize(toLight);
    float nl = max(dot(normal, dir), 0.0);
    float bias = max(0.001, (1.0 - nl) * 0.01);
    float pointShadowFactor = getPointShadowPcf(currentLight, position, bias);

    // accumulate PBR with point-light shadow
    totalRadiance += calcPointLight(Constants.Lights[i], 
                                     normal, 
                                     position, 
                                     viewDir, 
                                     albedo, 
                                     roughness, 
                                     metalness, 
                                     F0) * (1.0 - pointShadowFactor);
    // Breaking after first light as we have not implemented multiple point light shadow depth cube generation / passes yet.
    break;
  }
  // Ambient light contribution.
  totalRadiance += albedo.rgb * Constants.AmbientColour * Constants.AmbientIntensity * occlusionFactor;

  // Apply exposure to the HDR result
  totalRadiance *= Constants.Exposure;

  // Extract bright areas for bloom using luminance threshold (on exposed values)
  float luminance = dot(totalRadiance.rgb, vec3(0.2126, 0.7152, 0.0722));
  BloomColour = vec4(max(vec3(0.0), totalRadiance.rgb - Constants.BloomThreshold), 1.0);

  if (Constants.DrawCascadeLayers)
  {
    FinalColour = vec4(totalRadiance.rgb * drawCascadeLayers(position), 1.0f);
  }
  else
  {
    FinalColour = vec4(totalRadiance.rgb, 1.0f);
  }
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
  float val = clamp(1.0 - cosTheta, 0.0f, 1.0f);
  return F0 + (1.0 - F0) * (val*val*val*val*val);
}

float distributionGGX(vec3 N, vec3 H, float rough)
{
  float a  = rough * rough;
  float a2 = a * a;

  float nDotH  = max(dot(N, H), 0.0);
  float nDotH2 = nDotH * nDotH;

  float num = a2; 
  float denom = (nDotH2 * (a2 - 1.0) + 1.0);
  denom = 1 / (M_PI * denom * denom);

  return num * denom;
}

float geometrySchlickGGX(float nDotV, float rough)
{
  float r = (rough + 1.0);
  float k = r*r / 8.0;

  float num = nDotV;
  float denom = 1 / (nDotV * (1.0 - k) + k);

  return num * denom;
}

float geometrySmith(float nDotV, float nDotL, float rough)
{
  float ggx2  = geometrySchlickGGX(nDotV, rough);
  float ggx1  = geometrySchlickGGX(nDotL, rough);

  return ggx1 * ggx2;
}

vec3 calcDirLight(vec3 lightDirection,
                  vec3 lightColour,
                  float lightIntensity,
                  vec3 normal, 
                  vec3 viewDir, 
                  vec3 albedo, 
                  float roughness, 
                  float metalness, 
                  float shadowFactor, 
                  vec3 F0)
{
  vec3 lightDir = normalize(-lightDirection);
  vec3 halfway  = normalize(lightDir + viewDir);
  float nDotV = max(dot(normal, viewDir), 0.0f);
  float nDotL = max(dot(normal, lightDir), 0.0f);
  vec3 radianceIn = lightColour * lightIntensity;

  // Calculating Cook-Torrance BRDF terms.
  float NDF = distributionGGX(normal, halfway, roughness);
  float G = geometrySmith(nDotV, nDotL, roughness);
  vec3 F = fresnelSchlick(max(dot(halfway,viewDir), 0.0f), F0);

  vec3 kS = F;
  vec3 kD = vec3(1.0f) - kS;
  kD *= 1.0f - metalness;

  vec3 numerator = NDF * G * F;
  float denominator = 4.0f * nDotV * nDotL;
  vec3 specular = numerator / max (denominator, 0.0001f);

  vec3 radiance = (kD * (albedo / M_PI) + specular ) * radianceIn * nDotL;
  radiance *= (shadowFactor);

  return radiance;
}

vec3 calcPointLight(Light light, 
                    vec3 normal, 
                    vec3 fragPos, 
                    vec3 viewDir, 
                    vec3 albedo, 
                    float roughness, 
                    float metalness, 
                    vec3 F0)
{
  vec3 lightPosition = light.Position;
  vec3 colour = light.Colour;
  float radius = light.Radius;
  float intensity = light.Intensity;

  vec3 lightDir = normalize(lightPosition - fragPos);
  vec3 halfway  = normalize(lightDir + viewDir);
  float nDotV = max(dot(normal, viewDir), 0.0f);
  float nDotL = max(dot(normal, lightDir), 0.0f);

  float distance = length(lightPosition - fragPos);
  // Use inverse-square falloff for energy-conserving attenuation
  float attenuation = 1.0 / max(distance * distance, 0.0001);
  // Fold intensity and color into radianceIn
  vec3 radianceIn = colour * intensity * attenuation;

  // Calculating Cook-Torrance BRDF terms.
  float NDF = distributionGGX(normal, halfway, roughness);
  float G = geometrySmith(nDotV, nDotL, roughness);
  vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0f), F0);

  vec3 kS = F;
  vec3 kD = vec3(1.0f) - kS;
  kD *= 1.0 - metalness;

  vec3 numerator = NDF * G * F;
  float denominator = 4.0f * nDotV * nDotL;
  vec3 specular = numerator / max(denominator, 0.0001f);

  vec3 radiance = (kD * (albedo / M_PI) + specular ) * radianceIn * nDotL;

  return radiance;
}