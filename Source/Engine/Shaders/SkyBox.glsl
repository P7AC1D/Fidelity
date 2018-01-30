#version 330

uniform mat4 projection;
uniform mat4 view;
uniform samplerCube skybox;

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

void main()
{
  TexCoords = aPos;
  gl_Position = projection * view * vec4(aPos, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 FragColor;

in vec3 TexCoords;

void main()
{
  FragColor = texture(skybox, TexCoords);
}
#endif