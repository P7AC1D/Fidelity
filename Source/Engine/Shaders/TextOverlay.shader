#version 400 core

uniform float u_fontScale;
uniform vec3 u_fontColour;
uniform sampler2D u_fontAtlas;

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

const float SPREAD = 4.0f;

void main()
{
  float smoothing = 0.25f / (SPREAD * u_fontScale);
  float distance = texture(u_fontAtlas, TexCoord).a;
  float alpha = smoothstep(0.5f - smoothing, 0.5f + smoothing, distance);
  o_Colour = vec4(u_fontColour, alpha);
}
#endif
