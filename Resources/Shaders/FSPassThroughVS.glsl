#version 410

layout(location = 0) in vec2 aPosition;
layout(location = 2) in vec2 aTexCoord;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

layout(location = 0) out vec2 TexCoord;

void main()
{
  TexCoord = aTexCoord;
  gl_Position = vec4(aPosition, 0.0f, 1.0f);
}
