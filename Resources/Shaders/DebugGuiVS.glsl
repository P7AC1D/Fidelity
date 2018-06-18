#version 410 core

layout (location = 0) in vec3 aPosition;
layout (location = 4) in vec2 aTexCoord;
layout (location = 5) in vec4 aColour;

layout (std140) uniform Constants
{
  mat4 Projection;
};

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec4 FragColour;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
  TexCoord = aTexCoord;
  FragColour = aColour;
  gl_Position = Projection * vec4(aPosition.xy, 0.0f, 1.0f);
}
