#version 410

uniform sampler2D DepthMap;
uniform sampler2D BumpMap;

layout(std140) uniform PointLightPassConstants
{
  mat4 ProjViewInv;
  vec3 CameraPosition;
  float _paddingA;
  vec2 PixelSize;
} Constants;

layout(std140) uniform PointLightBuffer
{
  vec4 Colour;
  vec3 Position;
  float Radius;
} PointLight;

layout(location = 0) out vec4 Emissive;
layout(location = 1) out vec4 Specular;

void main()
{
  // Rebuild world position of fragment from frag-coord and depth texture
  vec3 position = vec3((gl_FragCoord.x * Constants.PixelSize.x), (gl_FragCoord.y * Constants.PixelSize.y), 0.0f);
  position.z = texture(DepthMap, position.xy).r;

  vec3 normal = normalize(texture(BumpMap, position.xy).xyz * 2.0f - 1.0f);

  vec4 clip = Constants.ProjViewInv * vec4(position * 2.0f - 1.0f, 1.0f);
  position = clip.xyz / clip.w;

  // Calculate attenuation and discard any fragments out of light's sphere
  float distance = length(PointLight.Position - position);
  float attenuation = 1.0f - clamp(distance / PointLight.Radius, 0.0f, 1.0f);

  if (attenuation == 0.0f) 
  {
    discard;
  }

  vec3 incident = normalize(PointLight.Position - position);
  vec3 viewDir = normalize(Constants.CameraPosition - position);
  vec3 halfDir = normalize(incident + viewDir);

  float lambert = clamp(dot(incident, normal), 0.0f, 1.0f);
  float rFactor = clamp(dot(halfDir, normal), 0.0f, 1.0f);
  float sFactor = pow(rFactor, 33.0f);

  Emissive = vec4(PointLight.Colour.xyz * lambert * attenuation, 1.0f);
  Specular = vec4(PointLight.Colour.xyz * sFactor * attenuation * 0.33f, 1.0f);
}