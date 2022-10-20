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
  vec3 positionTo = positionFrom;
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

  // Difference in screen coords between start and end positions.
  float deltaX = endSS.x - startSS.x;
  float deltaY = endSS.y - startSS.y;

  // Determine which is the larger path along the verical, horizontal or diagonal direction.
  // This will help us determine how much to travel in the X and Y direction each iteration and how many interations in the first pass.
  float useX = abs(deltaX) >= abs(deltaY) ? 1.0f : 0.0f;
  float delta = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0.0f, 1.0f);

  // How much to increment the X and Y position.
  vec2 increment = vec2(deltaX, deltaY) / max(delta, 0.001f);

  float search0 = 0.0f;
  float search1 = 0.0f;

  // Indicates there was an intersection during the first and second pass.
  int hit0 = 0;
  int hit1 = 0;

  float viewDistance = positionFrom.z;
  // The view distance difference between the current ray point and scene position.
  float depth = thickness;

  vec2 frag  = startSS;

  // First pass. When i reaches delta, the algorithm has traveled the entire length of the line.
  for (int i = 0; i < int(delta); i++)
  {
    frag += increment;
    uv = frag / texSize;
    positionTo = calculatePositionVS(uv);

    // Calculate the percentage or portion of the line the current fragment represents.
    search1 = mix((frag.y - startSS.y) / deltaY, (frag.x - startSS.x) / deltaX, useX);
    search1 = clamp(search1, 0.0f, 1.0f);

    // Interpolate (using perspective-correct interpolation) the view distance for the current position we're at on the reflection ray.
    viewDistance = (startVS.z * endVS.z) / mix(endVS.z, startVS.z, search1);
    depth = viewDistance - positionTo.z;

    if (depth > 0.0f && depth < thickness)
    {
      // Hit!
      hit0 = 1;
      break;
    }
    else
    {
      // Miss!
      search0 = search1;
    }
  }

  search1 = search0 + ((search1 - search0) / 2.0);
  steps *= hit0;

  // Second pass. If there was a miss during first pass, skip second pass.
  for (int i = 0; i < steps; i++)
  {
    frag = mix(startSS.xy, endSS.xy, search1);
    uv.xy = frag / texSize;
    positionTo = calculatePositionVS(uv);

    viewDistance = (startVS.y * endVS.y) / mix(endVS.y, startVS.y, search1);
    depth = viewDistance - positionTo.z;

    if (depth > 0.0f && depth < thickness)
    {
      // Hit!
      hit1 = 1;
      search1 = search0 + ((search1 - search0) / 2);
    }
    else
    {
      // Miss!
      float temp = search1;
      search1 = search1 + ((search1 - search0) / 2);
      search0 = temp;
    }
  }

  float visibility =
      hit1
    * positionTo.w
    * ( 1
      - max
         ( dot(-unitPositionFrom, pivot)
         , 0
         )
      )
    * ( 1
      - clamp
          ( depth / thickness
          , 0
          , 1
          )
      )
    * ( 1
      - clamp
          (   length(positionTo - positionFrom)
            / maxDistance
          , 0
          , 1
          )
      )
    * (uv.x < 0 || uv.x > 1 ? 0 : 1)
    * (uv.y < 0 || uv.y > 1 ? 0 : 1);
}