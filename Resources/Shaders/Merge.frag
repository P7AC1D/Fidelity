#version 410

uniform sampler2D DiffuseMap;
uniform sampler2D EmissiveMap;
uniform sampler2D SpecularMap;
uniform sampler2D NormalMap;
uniform sampler2D DepthMap;

layout(std140) uniform PointLightPassConstants
{
  mat4 ProjViewInv;
  vec3 CameraPosition;
  float _paddingA;
  vec2 PixelSize;
} Constants;

layout(std140) uniform PointLightBuffer
{
  mat4 Model;
  mat4 ModelView;
  mat4 ModelViewProjection;
  vec4 Colour;
  vec3 Position;
  float Radius;
  vec3 Direction;
  int LightType; // 0 - Point; 1 - Directional
} DirectionalLight;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

void main()
{
  // Rebuild world position of fragment from frag-coord and depth texture
  vec3 position = vec3((gl_FragCoord.x * Constants.PixelSize.x), (gl_FragCoord.y * Constants.PixelSize.y), 0.0f);
  position.z = texture(DepthMap, position.xy).r;

  vec3 normal = normalize(texture(NormalMap, position.xy).xyz * 2.0f - 1.0f);
  vec3 dirLightDiffuse = DirectionalLight.Colour.xyz * max(dot(normal, -DirectionalLight.Direction), 0.0f);

  vec3 diffuse = texture(DiffuseMap, TexCoord).xyz;
  vec3 light = texture(EmissiveMap, TexCoord).xyz;
  vec3 specular = texture(SpecularMap, TexCoord).xyz;

  FinalColour.xyz = diffuse * 0.1f; // ambient
  FinalColour.xyz += diffuse * (light + dirLightDiffuse * 0.2f); // lambert
  FinalColour.xyz += specular; // specular
  FinalColour.a = 1.0f;
}