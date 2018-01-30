#version 330

uniform float yScale;
uniform float xScale;
uniform float xOffset;
uniform float yOffset;
uniform sampler2D textureMap;

#ifdef VERTEX_SHADER
layout(location = 0) in vec2 position;
layout(location = 2) in vec2 uv;

out vec2 fragUv;

void main()
{
  fragUv = uv;
  gl_Position = vec4(xOffset + position.x * xScale, yOffset + position.y * yScale, 0.0f, 1.0f);
}
#endif

#ifdef FRAGMENT_SHADER
in vec2 fragUv;

out vec4 fragColour;

void main()
{
  fragColour = texture(textureMap, fragUv);
}
#endif