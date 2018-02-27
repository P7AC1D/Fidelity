#version 400 core

uniform vec3 u_textColour;
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

const float SMOOTHING = 1.0f / 16.0f;

void main()
{
  float distance = texture(u_FontAtlas, TexCoord).a;
  float alpha = smoothstep(0.5f - SMOOTHING, 0.5 + SMOOTHING, distance);
  o_Colour = vec4(u_textColour, alpha);
}
#endif
