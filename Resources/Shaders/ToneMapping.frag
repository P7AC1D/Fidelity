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
  float Exposure;
  bool ToneMappingEnabled;
  float BloomStrength;
} Constants;

uniform sampler2D LightingMap;
uniform sampler2D BloomMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

void main()
{
  vec4 colourSample = texture(LightingMap, TexCoord);
  vec3 hdrSample = colourSample.rgb;
  vec3 bloomSample = texture(BloomMap, TexCoord).rgb;

  // Linear interpolation.
  hdrSample = mix(hdrSample, bloomSample, Constants.BloomStrength);

  // Perform Reinhard tone mapping.
  vec3 ldrSample = vec3(1.0) - exp(-hdrSample * Constants.Exposure);

  if (Constants.ToneMappingEnabled == true)
  {
    FinalColour = vec4(ldrSample, colourSample.a);
  }
  else
  {
    FinalColour = colourSample;
  }
}