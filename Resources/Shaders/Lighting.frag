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

layout(std140) uniform ConstantsBuffer
{
  mat4 ProjViewInv;
  vec3 ViewPosition;
  float _padding;
  vec2 PixelSize;
} Constants;

layout(std140) uniform LightingBuffer
{
  Light Lights[MAX_LIGHTS];
  vec3 AmbientColour;
  float AmbientIntensity;
} Lighting;

uniform sampler2D AlbedoMap;
uniform sampler2D DepthMap;
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

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

  vec3 finalColour = Lighting.AmbientColour * Lighting.AmbientIntensity;

  vec3 ambient = albedo.rgb * Lighting.AmbientColour * Lighting.AmbientIntensity;
  vec3 viewDir = normalize(Constants.ViewPosition - position);
  for (int i = 0; i < MAX_LIGHTS; i++)
  {
    vec3 lightDir;
    if (Lighting.Lights[i].LightType == 0)
    {
      lightDir = normalize(Lighting.Lights[i].Position - position);

      float distance = length(Lighting.Lights[i].Position - position);
      float attenuation = 1.0f - clamp(distance / Lighting.Lights[i].Radius, 0.0f, 1.0f);

      if (attenuation == 0.0f)
      {
        continue;
      }
    }
    else if (Lighting.Lights[i].LightType == 1)
    {
      lightDir = Lighting.Lights[i].Direction;
    }

    vec3 halfDir = normalize(lightDir + viewDir);

    float diffuseFactor = clamp(dot(lightDir, normal), 0.0f, 1.0f);
    float specularFactor = pow(max(dot(normal, halfDir), 0.0), specular.a * 255.0f);

    finalColour += (albedo.rgb * diffuseFactor + specular.rgb * specularFactor) * Lighting.Lights[i].Colour * Lighting.Lights[i].Intensity;
  }
  FinalColour = vec4(finalColour, 1.0f);
}