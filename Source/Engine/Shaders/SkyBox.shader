#version 400

layout(std140) uniform Transforms
{
  mat4 u_proj;
  mat4 u_view;
  vec3 u_viewPosition;
};

uniform samplerCube u_skyBox;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 a_position;

out vec3 TexCoords;

void main()
{
  TexCoords = a_position;
  mat4 viewAdjusted = mat4(mat3(u_view));
  gl_Position = u_proj * viewAdjusted * vec4(a_position, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
in vec3 TexCoords;
out vec4 o_Colour;

void main()
{
  o_Colour = texture(u_skyBox, TexCoords);
}
#endif