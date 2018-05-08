#version 400 core

uniform bool u_IsTextured;
uniform vec4 u_PanelColour;
uniform sampler2D u_PanelTexture;

#ifdef VERTEX_SHADER
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoords;

out vec2 TexCoords;

void main()
{
  TexCoords = a_texCoords;
  gl_Position = vec4(a_position, 0.0f, 1.0f);
}
#endif

#ifdef FRAGMENT_SHADER
in vec2 TexCoords;
out vec4 o_Colour;

void main()
{  
  if (u_IsTextured)
  {
    vec4 texSample = texture(u_PanelTexture, TexCoords);
    o_Colour = u_PanelColour * texSample;
  }
  else
  {
    o_Colour = u_PanelColour;
  }
}
#endif

