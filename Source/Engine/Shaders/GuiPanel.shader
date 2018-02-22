#version 400 core

uniform vec4 u_PanelColour;

#ifdef VERTEX_SHADER
layout(location = 0) in vec2 a_position;

out vec2 TexCoord;

void main()
{
  gl_Position = vec4(a_position, 0.0f, 1.0f);
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 o_Colour;

void main()
{
  o_Colour = u_PanelColour;
}
#endif

