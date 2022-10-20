#version 410

// How far a fragment can reflect. In other words, it controls the maximum length or magnitude of the reflection ray.
float maxDistance = 15;
// How many fragments are skipped while traveling or marching the reflection ray during the first pass. 0 -> 1.
float resolution  = 0.3;
// How many iterations occur during the second pass.
int   steps       = 10;
// Cutoff between what counts as a possible reflection hit and what does not.
float thickness   = 0.5;

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

/// @brief Transforms the fragment position from screen space into view space.
/// @param texCoords The screen space position mapped to the range [0:1].
/// @return The transformed position in view space.
vec3 calculatePositionVS(vec2 texCoords)
{
  // Calculate position in screen space from depth.
  vec3 fragPos = vec3(texCoords, 0.0f);
  fragPos.z = texture(DepthMap, fragPos.xy).r;

  // Transform position to NDC space.
  fragPos = fragPos * 2.0f - 1.0f;

  // Transform position to view space and perform perspective divide.
  vec4 fragPosProjected = Constants.ProjInv * vec4(fragPos, 1.0f);
  fragPosProjected.xyz / fragPosProjected.w;
  return fragPosProjected.xyz;
}

vec2 transformFromViewToScreenSpace(vec3 pointVS, vec2 texSize)
{
  // Transform to screen space and perform perspective divide.
  vec4 point = Constants.Proj * vec4(pointVS, 1.0f);
  point.xyz /= point.w;

  // Convert from x-y coords to u-v texture coords.
  point.xy = point.xy * 0.5f + 0.5f;

  // Converet to pixel coords.
  point.xy *= texSize;
  return point.xy;
}

void main()
{
  vec2 texSize = textureSize(NormalMap, 0);
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec3 positionFrom = calculatePositionVS(texCoord);
  vec3 unitPositionFrom = normalize(positionFrom);
  vec3 normal = normalize(texture(NormalMap, texCoord).xyz);
  // Reflected ray from the unitPositionFrom vector against the normal.
  vec3 pivot = normalize(reflect(unitPositionFrom, normal));

  // Start and end locations of reflection ray in view space.
  vec3 startVS = vec3(positionFrom.xyz + pivot);
  vec3 endVS = vec3(positionFrom.xyz + (pivot * maxDistance));

  vec2 startSS = transformFromViewToScreenSpace(startVS, texSize);
  vec2 endSS = transformFromViewToScreenSpace(endVS, texSize);

  vec2 uv = startSS / texSize;

  float deltaX = endSS.x - startSS.x;
  float deltaY = endSS.y - startSS.y;
}