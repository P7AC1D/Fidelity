#version 400

#ifdef VERTEX_SHADER
layout (location = 0) in vec3 a_position;

uniform mat4 u_lightProj;
uniform mat4 u_lightView;
uniform mat4 u_modelTransform;

void main()
{
  gl_Position = u_lightProj *  u_lightView * u_modelTransform * vec4(a_position, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
void main()
{
  gl_FragDepth = gl_FragCoord.z;
}
#endif
