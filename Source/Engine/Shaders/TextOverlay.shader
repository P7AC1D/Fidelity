#version 400 core

uniform sampler2D u_FontAtlas;

#ifdef VERTEX_SHADER
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;

out vec2 TexCoord;

void main()
{
  TexCoord = a_texCoord;
  gl_Position = vec4(a_position, 0.0f, 1.0f);
}
#endif

#ifdef FRAGMENT_SHADER
in vec2 TexCoord;
out vec4 o_Colour;

void main()
{
  o_Colour = texture(u_FontAtlas, TexCoord);
}
#endif