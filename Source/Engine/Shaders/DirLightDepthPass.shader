#version 400

#ifdef VERTEX_SHADER
layout (location = 0) in vec3 a_position;

uniform mat4 u_lightSpaceTransform;
uniform mat4 u_modelTransform;

void main()
{
  gl_Position = u_lightSpaceTransform * u_modelTransform * vec4(a_position, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
void main()
{
}
#endif
