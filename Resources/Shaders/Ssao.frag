#version 410
const uint MaxKernelSize = 512;

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

layout(std140) uniform SsaoConstantsBuffer
{
  vec4 NoiseSamples[MaxKernelSize];
  uint KernelSize;
  float Radius;
  float Bias;
  float Intensity;
} SsaoConstants;

layout(location = 0) in vec2 TexCoord;

out float Occulsion;

uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D NoiseMap;
uniform sampler2D PositionMap;

vec3 calculatePositionVS(vec2 screenCoords)
{
  vec2 windowDimensions = textureSize(NormalMap, 0);
  
  // Calculate position in screen space.
  vec3 fragPos = vec3((gl_FragCoord.x / windowDimensions.x), (gl_FragCoord.y / windowDimensions.y), 0.0f);
  fragPos.z = texture(DepthMap, fragPos.xy).r;

  // Transform position to NDC space.
  fragPos = fragPos * 2.0f - 1.0f;

  // Transform position to view space and perform perspective divide.
  vec4 fragPosProjected = Constants.ProjInv * vec4(fragPos, 1.0f);
  fragPosProjected.xyz / fragPosProjected.w;
  return fragPosProjected.xyz;
}

void main()
{
  // Calculate fragment position in view-space from depth and fragment sceen-space position.
  vec3 positionVS = calculatePositionVS(gl_FragCoord.xy);

  // transform normal vector to range [-1,1]
  vec3 normalWS = normalize(texture(NormalMap, TexCoord).xyz * 2.0f - 1.0f);
  vec3 normalVS = (Constants.View * vec4(normalWS, 0.0f)).xyz;

  // tile noise texture over screen
  ivec2 screenSize = textureSize(DepthMap, 0);
  vec2 noiseScale = vec2(screenSize.x / 4.0, screenSize.y / 4.0); 
  vec3 randomVec = normalize(texture(NoiseMap, TexCoord * noiseScale).xyz);

  // Calc tangent -> view space transform
  vec3 tangent = normalize(randomVec - normalVS * dot(randomVec, normalVS));
  vec3 bitangent = cross(normalVS, tangent);
  mat3 transform = mat3(tangent, bitangent, normalVS);

  float occlusion = 0.0;
  for (uint i = 0; i < SsaoConstants.KernelSize; ++i)
  {
    // Create sample position in view space.
    vec3 samplePositionVs = transform * SsaoConstants.NoiseSamples[i].xyz;
    samplePositionVs = positionVS + samplePositionVs * SsaoConstants.Radius;

    // Transform sample position to NDC space.
    vec4 offset = Constants.Proj * vec4(samplePositionVs, 1.0);
    offset.xyz /= offset.w;
    offset.xyz  = offset.xyz * 0.5f + 0.5f;

    // Transform sample position to view space.
    float sampleDepth = calculatePositionVS(offset.xy).z;

    // Check if the depth for that XY position in the scene is larger than the sample's and add it to the occlusion.
    if (sampleDepth >= samplePositionVs.z + SsaoConstants.Bias)
    {
      occlusion += smoothstep(0.0, 1.0, SsaoConstants.Radius / abs(positionVS.z - sampleDepth));
    }
  }

  // Output in the range [0, 1].
  Occulsion = pow(1.0 - (occlusion / SsaoConstants.KernelSize), SsaoConstants.Intensity);
}
