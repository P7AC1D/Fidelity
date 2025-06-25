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
  float BloomThreshold;
} Constants;

uniform sampler2D LightingMap;
uniform sampler2D BloomMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

vec3 ACESFilm(vec3 x)
{
    // ACES approximation constants
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Gamma correction function (sRGB approx)
vec3 GammaCorrect(vec3 color, float gamma)
{
    return pow(color, vec3(1.0 / gamma));
}

void main()
{
    vec4 colourSample = texture(LightingMap, TexCoord);
    vec3 hdrSample = colourSample.rgb;
    vec3 bloomSample = texture(BloomMap, TexCoord).rgb;

    // Linear interpolation between exposed HDR and bloom
    hdrSample = mix(hdrSample, bloomSample, Constants.BloomStrength);

    vec3 mapped;
    if (Constants.ToneMappingEnabled)
    {
        // Apply ACES tone mapping
        mapped = ACESFilm(hdrSample);
    }
    else
    {
        // Just clamp if tone mapping disabled
        mapped = min(hdrSample, vec3(1.0));
    }

    // Apply gamma correction
    vec3 finalColor = GammaCorrect(mapped, 2.2);

    FinalColour = vec4(finalColor, colourSample.a);
}